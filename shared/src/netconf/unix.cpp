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

#include <shared/include/netconf/unix.hpp>
#include <shared/include/logger.hpp>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <sys/select.h>
#include <regex>

namespace netd::shared::netconf {

  UnixTransport::UnixTransport() : listening_(false), server_socket_(-1) {}

  UnixTransport::~UnixTransport() { 
    stop(); 
  }

  bool UnixTransport::start(const std::string &address) {
    auto &logger = netd::shared::Logger::getInstance();
    socketPath_ = address;

    // Check if running as root (required for UNIX socket binding)
    if (geteuid() != 0) {
      logger.error("Unix transport must be run as root to bind to UNIX socket");
      return false;
    }

    // Check socket directory permissions
    if (!checkSocketDirectory()) {
      logger.error("Socket directory permission check failed");
      return false;
    }

    // Prepare socket file (remove existing if present)
    if (!prepareSocketFile()) {
      logger.error("Failed to prepare socket file");
      return false;
    }

    // Create server socket
    if (!createServerSocket()) {
      logger.error("Failed to create server socket");
      return false;
    }

    listening_ = true;
    logger.info("Unix transport started on " + socketPath_);
    return true;
  }

  void UnixTransport::stop() {
    if (!listening_) {
      return;
    }

    auto &logger = netd::shared::Logger::getInstance();
    listening_ = false;

    // Clean up socket file
    if (!socketPath_.empty()) {
      if (unlink(socketPath_.c_str()) == 0) {
        logger.info("Removed socket file: " + socketPath_);
      } else if (errno != ENOENT) {
        logger.warning("Failed to remove socket file " + socketPath_ + ": " + std::string(strerror(errno)));
      }
    }

    // Close server socket
    if (server_socket_ >= 0) {
      close(server_socket_);
      server_socket_ = -1;
    }

    logger.info("Unix transport stopped");
  }

  bool UnixTransport::isListening() const { 
    return listening_; 
  }

  const std::string &UnixTransport::getSocketPath() const { 
    return socketPath_; 
  }

  int UnixTransport::getServerSocket() const {
    return server_socket_;
  }

  int UnixTransport::acceptConnection() {
    if (server_socket_ < 0) {
      return -1;
    }

    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
    return client_socket;
  }

  void UnixTransport::closeConnection(int socket_fd) {
    if (socket_fd >= 0) {
      close(socket_fd);
    }
  }

  bool UnixTransport::sendData(int socket_fd, const std::string& data) {
    if (socket_fd < 0) {
      return false;
    }

    // Send the data
    ssize_t bytes_sent = send(socket_fd, data.c_str(), data.length(), 0);
    if (bytes_sent < 0) {
      auto &logger = netd::shared::Logger::getInstance();
      logger.error("Failed to send data: " + std::string(strerror(errno)));
      return false;
    }

    // Send NETCONF 1.0 separator
    const char* separator = "]]>]]>";
    bytes_sent = send(socket_fd, separator, strlen(separator), 0);
    if (bytes_sent < 0) {
      auto &logger = netd::shared::Logger::getInstance();
      logger.error("Failed to send NETCONF separator: " + std::string(strerror(errno)));
      return false;
    }

    return true;
  }

  std::string UnixTransport::receiveData(int socket_fd) {
    if (socket_fd < 0) {
      return "";
    }

    std::string result;
    char buffer[4096];
    
    while (true) {
      ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
      if (bytes_received <= 0) {
        if (bytes_received == 0) {
          // Connection closed by peer
          return "";
        } else {
          // Error
          auto &logger = netd::shared::Logger::getInstance();
          logger.error("Failed to receive data: " + std::string(strerror(errno)));
          return "";
        }
      }

      buffer[bytes_received] = '\0';
      result += buffer;

      // Check for NETCONF message separators using regex
      std::regex netconf_separator_regex(R"((\]\]>\]\]>|\n##\n))");
      if (std::regex_search(result, netconf_separator_regex)) {
        break;
      }
    }

    // Extract XML message (remove separator) using regex
    std::regex netconf_separator_regex(R"((\]\]>\]\]>|\n##\n))");
    std::smatch match;
    if (std::regex_search(result, match, netconf_separator_regex)) {
      result = result.substr(0, match.position());
    }

    return result;
  }

  const std::string& UnixTransport::getAddress() const {
    return socketPath_;
  }

  bool UnixTransport::createServerSocket() {
    server_socket_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
      auto &logger = netd::shared::Logger::getInstance();
      logger.error("Failed to create socket: " + std::string(strerror(errno)));
      return false;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
      auto &logger = netd::shared::Logger::getInstance();
      logger.error("Failed to set socket options: " + std::string(strerror(errno)));
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }

    // Bind to socket path
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath_.c_str(), sizeof(addr.sun_path) - 1);

    if (bind(server_socket_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
      auto &logger = netd::shared::Logger::getInstance();
      logger.error("Failed to bind socket: " + std::string(strerror(errno)));
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }

    // Set socket permissions
    if (chmod(socketPath_.c_str(), 0666) < 0) {
      auto &logger = netd::shared::Logger::getInstance();
      logger.warning("Failed to set socket permissions: " + std::string(strerror(errno)));
    }

    // Start listening
    if (listen(server_socket_, 10) < 0) {
      auto &logger = netd::shared::Logger::getInstance();
      logger.error("Failed to listen on socket: " + std::string(strerror(errno)));
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }

    return true;
  }

  bool UnixTransport::prepareSocketFile() {
    auto &logger = netd::shared::Logger::getInstance();
    
    // Check if socket file already exists
    struct stat st;
    if (stat(socketPath_.c_str(), &st) == 0) {
      // Check if it's actually a socket file
      if (!S_ISSOCK(st.st_mode)) {
        logger.error("Socket path exists but is not a socket file: " + socketPath_);
        return false;
      }
      
      // Try to remove existing socket file
      if (unlink(socketPath_.c_str()) != 0) {
        logger.error("Failed to remove existing socket file " + socketPath_ + ": " + std::string(strerror(errno)));
        return false;
      }
      
      logger.info("Removed existing socket file: " + socketPath_);
    }
    
    return true;
  }

  bool UnixTransport::checkSocketDirectory() {
    auto &logger = netd::shared::Logger::getInstance();
    
    // Extract directory from socket path
    size_t lastSlash = socketPath_.find_last_of('/');
    if (lastSlash == std::string::npos) {
      logger.error("Invalid socket path (no directory): " + socketPath_);
      return false;
    }
    
    std::string dirPath = socketPath_.substr(0, lastSlash);
    
    // Check if directory exists and is writable
    struct stat st;
    if (stat(dirPath.c_str(), &st) != 0) {
      logger.error("Socket directory does not exist: " + dirPath);
      return false;
    }
    
    if (!S_ISDIR(st.st_mode)) {
      logger.error("Socket path is not a directory: " + dirPath);
      return false;
    }
    
    // Check write permissions
    if (access(dirPath.c_str(), W_OK) != 0) {
      logger.error("No write permission to socket directory: " + dirPath + " (" + std::string(strerror(errno)) + ")");
      return false;
    }
    
    return true;
  }

} // namespace netd::shared::netconf
