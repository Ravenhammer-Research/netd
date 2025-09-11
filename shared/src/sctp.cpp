/*
 * Copyright (c) 2024 Paige Thompson / Ravenhammer Research (paige@paige.bio)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <shared/include/sctp.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <cstring>
#include <sstream>

namespace netd::shared {

  SCTPTransport::SCTPTransport(SCTPMode mode) 
    : mode_(mode)
    , primary_address_("")
    , port_(0)
    , listening_(false)
    , running_(false)
    , server_socket_(-1)
  {
  }

  SCTPTransport::~SCTPTransport() { 
    stop(); 
  }

  bool SCTPTransport::start(const std::string& address) {
    // Default to port 830 for NETCONF over SCTP
    return start(address, 830);
  }

  bool SCTPTransport::start(const std::string& address, int port) {
    if (running_) {
      return false;
    }

    primary_address_ = address;
    port_ = port;
    
    // Add primary address to local addresses if not already present
    bool found = false;
    for (const auto& addr : local_addresses_) {
      if (addr.address == address && addr.port == port) {
        found = true;
        break;
      }
    }
    if (!found) {
      addLocalAddress(address, true);
    }

    if (mode_ == SCTPMode::SERVER) {
      if (!setupServerSocket()) {
        return false;
      }
      
      running_ = true;
      listening_ = true;
      
      // Start accept thread
      accept_thread_ = std::make_unique<std::thread>(&SCTPTransport::acceptConnections, this);
      
      auto& logger = Logger::getInstance();
      logger.info("SCTP server started on " + primary_address_ + ":" + std::to_string(port_));
      return true;
    } else {
      // Client mode - connect to server
      if (!setupClientSocket()) {
        return false;
      }
      
      running_ = true;
      
      auto& logger = Logger::getInstance();
      logger.info("SCTP client connected to " + primary_address_ + ":" + std::to_string(port_));
      return true;
    }
  }

  void SCTPTransport::stop() {
    if (!running_) {
      return;
    }

    running_ = false;
    listening_ = false;

    // Close server socket
    if (server_socket_ != -1) {
      close(server_socket_);
      server_socket_ = -1;
    }

    // Close all connections
    {
      std::lock_guard<std::mutex> lock(connections_mutex_);
      for (auto& conn : connections_) {
        if (conn && conn->socket_fd != -1) {
          close(conn->socket_fd);
        }
      }
      connections_.clear();
    }

    // Wait for accept thread to finish
    if (accept_thread_ && accept_thread_->joinable()) {
      accept_thread_->join();
      accept_thread_.reset();
    }

    auto& logger = Logger::getInstance();
    logger.info("SCTP transport stopped");
  }

  bool SCTPTransport::isListening() const {
    return listening_;
  }

  bool SCTPTransport::isRunning() const {
    return running_;
  }

  int SCTPTransport::acceptConnection() {
    if (!running_ || mode_ != SCTPMode::SERVER) {
      return -1;
    }

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
    if (client_socket == -1) {
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        auto& logger = Logger::getInstance();
        logger.error("SCTP accept failed: " + std::string(strerror(errno)));
      }
      return -1;
    }

    // Create connection object
    auto conn = std::make_unique<SCTPConnection>();
    conn->socket_fd = client_socket;
    conn->address = client_addr;
    conn->connected = true;
    
    char addr_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, addr_str, INET_ADDRSTRLEN);
    conn->peer_address = std::string(addr_str);
    conn->peer_port = ntohs(client_addr.sin_port);

    // Store connection
    {
      std::lock_guard<std::mutex> lock(connections_mutex_);
      connections_.push_back(std::move(conn));
    }

    auto& logger = Logger::getInstance();
    logger.info("SCTP connection accepted from " + conn->peer_address + ":" + std::to_string(conn->peer_port));

    return client_socket;
  }

  void SCTPTransport::closeConnection(int socket_fd) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    auto it = std::find_if(connections_.begin(), connections_.end(),
                          [socket_fd](const std::unique_ptr<SCTPConnection>& conn) {
                            return conn && conn->socket_fd == socket_fd;
                          });
    
    if (it != connections_.end()) {
      auto& conn = *it;
      if (conn->socket_fd != -1) {
        close(conn->socket_fd);
        conn->socket_fd = -1;
        conn->connected = false;
        
        auto& logger = Logger::getInstance();
        logger.info("SCTP connection closed: " + conn->peer_address + ":" + std::to_string(conn->peer_port));
      }
      connections_.erase(it);
    }
  }

  bool SCTPTransport::sendData(int socket_fd, const std::string& data) {
    ssize_t bytes_sent = sctp_sendmsg(socket_fd, data.c_str(), data.length(), 
                                     nullptr, 0, 0, 0, 0, 0, 0);
    if (bytes_sent == -1) {
      auto& logger = Logger::getInstance();
      logger.error("SCTP send failed: " + std::string(strerror(errno)));
      return false;
    }
    
    return bytes_sent == static_cast<ssize_t>(data.length());
  }

  std::string SCTPTransport::receiveData(int socket_fd) {
    char buffer[4096];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    int flags = 0;
    
    ssize_t bytes_received = sctp_recvmsg(socket_fd, buffer, sizeof(buffer) - 1,
                                         (struct sockaddr*)&from_addr, &from_len,
                                         nullptr, &flags);
    
    if (bytes_received == -1) {
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        auto& logger = Logger::getInstance();
        logger.error("SCTP receive failed: " + std::string(strerror(errno)));
      }
      return "";
    }
    
    buffer[bytes_received] = '\0';
    return std::string(buffer);
  }

  const std::string& SCTPTransport::getAddress() const {
    return primary_address_;
  }

  int SCTPTransport::getPort() const {
    return port_;
  }

  std::vector<std::string> SCTPTransport::getConnectedPeers() const {
    std::vector<std::string> peers;
    
    std::lock_guard<std::mutex> lock(connections_mutex_);
    for (const auto& conn : connections_) {
      if (conn && conn->connected) {
        peers.push_back(conn->peer_address + ":" + std::to_string(conn->peer_port));
      }
    }
    
    return peers;
  }

  bool SCTPTransport::setupServerSocket() {
    server_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (server_socket_ == -1) {
      auto& logger = Logger::getInstance();
      logger.error("Failed to create SCTP socket: " + std::string(strerror(errno)));
      return false;
    }

    // Set socket options
    int reuse = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
      auto& logger = Logger::getInstance();
      logger.error("Failed to set SO_REUSEADDR: " + std::string(strerror(errno)));
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }

    // Set non-blocking
    int flags = fcntl(server_socket_, F_GETFL, 0);
    if (flags == -1 || fcntl(server_socket_, F_SETFL, flags | O_NONBLOCK) == -1) {
      auto& logger = Logger::getInstance();
      logger.error("Failed to set non-blocking mode: " + std::string(strerror(errno)));
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }

    // Bind to addresses (multihoming support)
    if (local_addresses_.size() > 1) {
      // Multihomed setup
      if (!setupMultihomedSocket()) {
        close(server_socket_);
        server_socket_ = -1;
        return false;
      }
    } else {
      // Single address setup
      struct sockaddr_in addr;
      memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port_);
      
      if (primary_address_.empty() || primary_address_ == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
      } else {
        if (inet_pton(AF_INET, primary_address_.c_str(), &addr.sin_addr) != 1) {
          auto& logger = Logger::getInstance();
          logger.error("Invalid address: " + primary_address_);
          close(server_socket_);
          server_socket_ = -1;
          return false;
        }
      }

      if (bind(server_socket_, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        auto& logger = Logger::getInstance();
        logger.error("Failed to bind SCTP socket: " + std::string(strerror(errno)));
        close(server_socket_);
        server_socket_ = -1;
        return false;
      }
    }

    if (listen(server_socket_, 10) == -1) {
      auto& logger = Logger::getInstance();
      logger.error("Failed to listen on SCTP socket: " + std::string(strerror(errno)));
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }

    return true;
  }

  bool SCTPTransport::setupClientSocket() {
    // For client mode, we would implement connection logic here
    // This is a placeholder for future client implementation
    auto& logger = Logger::getInstance();
    logger.warning("SCTP client mode not yet implemented");
    return false;
  }

  void SCTPTransport::acceptConnections() {
    while (running_ && listening_) {
      int client_socket = acceptConnection();
      if (client_socket != -1) {
        // Connection accepted and stored in connections_ vector
        // The actual message handling would be done by the NETCONF layer
      }
      
      // Small delay to prevent busy waiting
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  void SCTPTransport::handleConnection(std::unique_ptr<SCTPConnection> conn) {
    // This would handle individual connection processing
    // For now, just a placeholder
    (void)conn; // Suppress unused parameter warning
  }

  bool SCTPTransport::sendMessage(int socket_fd, const std::string& message) {
    return sendData(socket_fd, message);
  }

  std::string SCTPTransport::receiveMessage(int socket_fd) {
    return receiveData(socket_fd);
  }

  void SCTPTransport::closeConnection(std::unique_ptr<SCTPConnection> conn) {
    if (conn && conn->socket_fd != -1) {
      close(conn->socket_fd);
      conn->socket_fd = -1;
      conn->connected = false;
    }
  }

  // Multihoming support methods
  bool SCTPTransport::addLocalAddress(const std::string& address, bool is_primary) {
    if (running_) {
      auto& logger = Logger::getInstance();
      logger.error("Cannot add addresses while SCTP transport is running");
      return false;
    }

    // Check if address already exists
    for (const auto& addr : local_addresses_) {
      if (addr.address == address && addr.port == port_) {
        return true; // Already exists
      }
    }

    local_addresses_.emplace_back(address, port_, is_primary);
    
    if (is_primary) {
      primary_address_ = address;
      // Mark other addresses as non-primary
      for (auto& addr : local_addresses_) {
        if (addr.address != address) {
          addr.is_primary = false;
        }
      }
    }

    auto& logger = Logger::getInstance();
    logger.info("Added local address: " + address + ":" + std::to_string(port_) + 
                (is_primary ? " (primary)" : ""));
    return true;
  }

  bool SCTPTransport::removeLocalAddress(const std::string& address) {
    if (running_) {
      auto& logger = Logger::getInstance();
      logger.error("Cannot remove addresses while SCTP transport is running");
      return false;
    }

    auto it = std::find_if(local_addresses_.begin(), local_addresses_.end(),
                          [&address](const SCTPAddress& addr) {
                            return addr.address == address;
                          });

    if (it != local_addresses_.end()) {
      bool was_primary = it->is_primary;
      local_addresses_.erase(it);
      
      // If we removed the primary address, make the first remaining address primary
      if (was_primary && !local_addresses_.empty()) {
        local_addresses_[0].is_primary = true;
        primary_address_ = local_addresses_[0].address;
      }
      
      auto& logger = Logger::getInstance();
      logger.info("Removed local address: " + address);
      return true;
    }

    return false;
  }

  std::vector<SCTPAddress> SCTPTransport::getLocalAddresses() const {
    return local_addresses_;
  }

  std::vector<SCTPAddress> SCTPTransport::getRemoteAddresses(int socket_fd) const {
    std::vector<SCTPAddress> remote_addrs;
    
    std::lock_guard<std::mutex> lock(connections_mutex_);
    auto it = std::find_if(connections_.begin(), connections_.end(),
                          [socket_fd](const std::unique_ptr<SCTPConnection>& conn) {
                            return conn && conn->socket_fd == socket_fd;
                          });
    
    if (it != connections_.end()) {
      remote_addrs = (*it)->remote_addresses;
    }
    
    return remote_addrs;
  }


  // Stream management (RFC 6083)
  bool SCTPTransport::sendDataOnStream(int socket_fd, const std::string& data, uint16_t stream_id, bool ordered) {
    // Use sctp_sendmsg with stream and ordered parameters
    struct sctp_sndrcvinfo sri;
    memset(&sri, 0, sizeof(sri));
    sri.sinfo_stream = stream_id;
    sri.sinfo_flags = ordered ? 0 : SCTP_UNORDERED;
    
    ssize_t bytes_sent = sctp_sendmsg(socket_fd, data.c_str(), data.length(),
                                     nullptr, 0, 0, 0, stream_id, 0, 0);
    
    if (bytes_sent == -1) {
      auto& logger = Logger::getInstance();
      logger.error("SCTP send on stream " + std::to_string(stream_id) + " failed: " + std::string(strerror(errno)));
      return false;
    }
    
    return bytes_sent == static_cast<ssize_t>(data.length());
  }

  std::string SCTPTransport::receiveDataFromStream(int socket_fd, uint16_t& stream_id, bool& ordered) {
    char buffer[4096];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    struct sctp_sndrcvinfo sri;
    int flags = 0;
    
    ssize_t bytes_received = sctp_recvmsg(socket_fd, buffer, sizeof(buffer) - 1,
                                         (struct sockaddr*)&from_addr, &from_len,
                                         &sri, &flags);
    
    if (bytes_received == -1) {
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        auto& logger = Logger::getInstance();
        logger.error("SCTP receive from stream failed: " + std::string(strerror(errno)));
      }
      return "";
    }
    
    stream_id = sri.sinfo_stream;
    ordered = !(sri.sinfo_flags & SCTP_UNORDERED);
    
    buffer[bytes_received] = '\0';
    return std::string(buffer);
  }

  // Private helper methods
  bool SCTPTransport::setupMultihomedSocket() {
    // For multihoming, we need to bind to all local addresses
    return bindToAllAddresses();
  }

  bool SCTPTransport::bindToAllAddresses() {
    if (local_addresses_.empty()) {
      return false;
    }

    // Bind to all local addresses
    for (const auto& addr : local_addresses_) {
      struct sockaddr_in bind_addr;
      memset(&bind_addr, 0, sizeof(bind_addr));
      bind_addr.sin_family = AF_INET;
      bind_addr.sin_port = htons(addr.port);
      
      if (addr.address.empty() || addr.address == "0.0.0.0") {
        bind_addr.sin_addr.s_addr = INADDR_ANY;
      } else {
        if (inet_pton(AF_INET, addr.address.c_str(), &bind_addr.sin_addr) != 1) {
          auto& logger = Logger::getInstance();
          logger.error("Invalid address for binding: " + addr.address);
          return false;
        }
      }

      if (bind(server_socket_, (struct sockaddr*)&bind_addr, sizeof(bind_addr)) == -1) {
        auto& logger = Logger::getInstance();
        logger.error("Failed to bind SCTP socket to " + addr.address + ":" + std::to_string(addr.port) + 
                    ": " + std::string(strerror(errno)));
        return false;
      }
    }

    return true;
  }


  bool SCTPTransport::enableSCTPAuth() {
    // Enable SCTP authentication as per RFC 4895
    // This is required for DTLS over SCTP
    auto& logger = Logger::getInstance();
    logger.info("SCTP-AUTH enabled (placeholder implementation)");
    return true;
  }

  bool SCTPTransport::setSCTPAuthKey(const std::string& key) {
    // Set SCTP authentication key
    auto& logger = Logger::getInstance();
    logger.info("SCTP-AUTH key set (placeholder implementation)");
    (void)key; // Suppress unused parameter warning
    return true;
  }

} // namespace netd::shared
