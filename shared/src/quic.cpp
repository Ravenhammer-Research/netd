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

#include <shared/include/quic.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <random>
#include <chrono>
#include <algorithm>

namespace netd::shared {

  QuicTransport::QuicTransport() 
    : listen_address_(""), listen_port_(0) {}

  QuicTransport::~QuicTransport() {
    stop();
  }

  // BaseTransport interface implementation
  bool QuicTransport::start(const std::string& address) {
    return start(address, 443); // Default to port 443 for QUIC
  }

  bool QuicTransport::start(const std::string& address, uint16_t port) {
    if (listening_) {
      return false; // Already running
    }
    
    listen_address_ = address;
    listen_port_ = port;
    
    if (!setupServerSocket()) {
      return false;
    }
    
    listening_ = true;
    should_stop_ = false;
    
    // Start worker threads
    for (size_t i = 0; i < MAX_WORKER_THREADS; ++i) {
      worker_threads_.emplace_back([this]() {
        while (!should_stop_) {
          // Accept connections in a loop
          struct sockaddr_in client_addr;
          socklen_t client_len = sizeof(client_addr);
          
          int client_socket = accept(server_socket_, 
                                   reinterpret_cast<struct sockaddr*>(&client_addr), 
                                   &client_len);
          
          if (client_socket >= 0) {
            // Generate connection ID
            QuicConnectionId connection_id = generateConnectionId();
            
            // Create connection
            auto connection = std::make_unique<QuicConnection>(
              connection_id, 
              inet_ntoa(client_addr.sin_addr), 
              ntohs(client_addr.sin_port)
            );
            
            // Store connection
            {
              std::lock_guard<std::mutex> lock(connections_mutex_);
              connections_[connection_id] = std::move(connection);
            }
            
            // Handle connection in separate thread
            std::thread([this, connection_id]() {
              std::lock_guard<std::mutex> lock(connections_mutex_);
              auto it = connections_.find(connection_id);
              if (it != connections_.end()) {
                handleConnection(std::move(it->second));
              }
            }).detach();
          }
        }
      });
    }
    
    return true;
  }

  void QuicTransport::stop() {
    if (!listening_) {
      return;
    }
    
    should_stop_ = true;
    listening_ = false;
    
    // Close server socket
    if (server_socket_ >= 0) {
      close(server_socket_);
      server_socket_ = -1;
    }
    
    // Close all connections
    {
      std::lock_guard<std::mutex> lock(connections_mutex_);
      for (auto& [id, connection] : connections_) {
        if (connection) {
          connection->is_active = false;
        }
      }
      connections_.clear();
    }
    
    // Wait for all worker threads to finish
    for (auto& thread : worker_threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    worker_threads_.clear();
  }

  bool QuicTransport::isListening() const {
    return listening_;
  }

  const std::string& QuicTransport::getAddress() const {
    return listen_address_;
  }

  uint16_t QuicTransport::getPort() const {
    return listen_port_;
  }

  int QuicTransport::acceptConnection() {
    // QUIC doesn't use traditional accept() - it handles connections via UDP packets
    // This method is required by BaseTransport but not used in QUIC context
    return -1; // Not applicable for QUIC
  }

  void QuicTransport::closeConnection(int socket_fd) {
    if (socket_fd >= 0) {
      close(socket_fd);
    }
  }

  bool QuicTransport::sendData(int socket_fd, const std::string& data) {
    if (socket_fd < 0) {
      return false;
    }
    
    // Convert string to vector<uint8_t> for QUIC
    std::vector<uint8_t> data_vec(data.begin(), data.end());
    
    // For QUIC, we need a connection ID - use a default one
    QuicConnectionId default_conn_id;
    default_conn_id.data = {0x00, 0x01, 0x02, 0x03}; // Simple default connection ID
    
    return sendData(default_conn_id, 0, data_vec);
  }

  std::string QuicTransport::receiveData(int socket_fd) {
    if (socket_fd < 0) {
      return "";
    }
    
    // For QUIC, we need a connection ID - use a default one
    QuicConnectionId default_conn_id;
    default_conn_id.data = {0x00, 0x01, 0x02, 0x03}; // Simple default connection ID
    
    auto data_vec = receiveData(default_conn_id, 0);
    return std::string(data_vec.begin(), data_vec.end());
  }

  bool QuicTransport::setupServerSocket() {
    server_socket_ = socket(AF_INET, SOCK_DGRAM, 0); // QUIC uses UDP
    if (server_socket_ < 0) {
      return false;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }
    
    // Set non-blocking
    int flags = fcntl(server_socket_, F_GETFL, 0);
    fcntl(server_socket_, F_SETFL, flags | O_NONBLOCK);
    
    // Bind to address and port
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(listen_port_);
    
    if (listen_address_.empty() || listen_address_ == "0.0.0.0") {
      server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
      if (inet_pton(AF_INET, listen_address_.c_str(), &server_addr.sin_addr) <= 0) {
        close(server_socket_);
        server_socket_ = -1;
        return false;
      }
    }
    
    if (bind(server_socket_, reinterpret_cast<struct sockaddr*>(&server_addr), 
             sizeof(server_addr)) < 0) {
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }
    
    return true;
  }

  void QuicTransport::handleConnection(std::unique_ptr<QuicConnection> connection) {
    if (!connection || !connection->is_active) {
      return;
    }
    
    char buffer[65536]; // Maximum UDP packet size
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    while (connection->is_active && !should_stop_) {
      ssize_t bytes_received = recvfrom(server_socket_, buffer, sizeof(buffer), 0,
                                       reinterpret_cast<struct sockaddr*>(&client_addr),
                                       &client_len);
      
      if (bytes_received > 0) {
        std::vector<uint8_t> packet(buffer, buffer + bytes_received);
        processPacket(packet, connection.get());
      }
    }
    
    // Cleanup connection
    cleanupConnection(connection->connection_id);
  }

  QuicConnectionId QuicTransport::generateConnectionId() {
    QuicConnectionId id;
    id.data.resize(8); // Standard connection ID length
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (auto& byte : id.data) {
      byte = static_cast<uint8_t>(dis(gen));
    }
    
    return id;
  }

  QuicStreamId QuicTransport::generateStreamId(QuicStreamType type) {
    static std::atomic<QuicStreamId> next_stream_id{0};
    
    QuicStreamId base_id = next_stream_id.fetch_add(2);
    
    // Client-initiated bidirectional streams: 0, 4, 8, 12, ...
    // Client-initiated unidirectional streams: 2, 6, 10, 14, ...
    // Server-initiated bidirectional streams: 1, 5, 9, 13, ...
    // Server-initiated unidirectional streams: 3, 7, 11, 15, ...
    
    switch (type) {
      case QuicStreamType::BIDIRECTIONAL:
        return base_id; // Server-initiated bidirectional
      case QuicStreamType::UNIDIRECTIONAL:
        return base_id + 1; // Server-initiated unidirectional
      default:
        return base_id;
    }
  }

  bool QuicTransport::processPacket(const std::vector<uint8_t>& packet, QuicConnection* connection) {
    if (packet.empty()) {
      return false;
    }
    
    // Determine packet type based on first byte
    uint8_t first_byte = packet[0];
    
    if ((first_byte & 0x80) != 0) {
      // Long header packet
      if ((first_byte & 0x30) == 0x00) {
        return processInitialPacket(packet, connection);
      } else if ((first_byte & 0x30) == 0x20) {
        return processHandshakePacket(packet, connection);
      }
    } else {
      // Short header packet
      return processShortHeaderPacket(packet, connection);
    }
    
    return false;
  }

  bool QuicTransport::processInitialPacket(const std::vector<uint8_t>& packet, QuicConnection* connection) {
    // Simplified initial packet processing
    // In a real implementation, this would handle version negotiation, etc.
    
    (void)packet;
    (void)connection;
    
    auto& logger = Logger::getInstance();
    logger.info("Processing QUIC initial packet");
    
    // For now, just acknowledge the connection
    return true;
  }

  bool QuicTransport::processHandshakePacket(const std::vector<uint8_t>& packet, QuicConnection* connection) {
    // Simplified handshake packet processing
    (void)packet;
    (void)connection;
    
    auto& logger = Logger::getInstance();
    logger.info("Processing QUIC handshake packet");
    
    return true;
  }

  bool QuicTransport::processShortHeaderPacket(const std::vector<uint8_t>& packet, QuicConnection* connection) {
    // Simplified short header packet processing
    // Extract frames and process them
    
    (void)connection;
    
    size_t offset = 1; // Skip first byte (header)
    
    while (offset < packet.size()) {
      if (offset >= packet.size()) break;
      
      uint8_t frame_type = packet[offset++];
      
      // Simplified frame processing
      switch (static_cast<QuicFrameType>(frame_type)) {
        case QuicFrameType::STREAM:
          // Process stream frame
          break;
        case QuicFrameType::CRYPTO:
          // Process crypto frame
          break;
        case QuicFrameType::ACK:
          // Process ACK frame
          break;
        default:
          // Unknown frame type
          break;
      }
    }
    
    return true;
  }

  bool QuicTransport::processFrame(const std::vector<uint8_t>& frame_data, QuicConnection* connection) {
    if (frame_data.empty()) {
      return false;
    }
    
    uint8_t frame_type = frame_data[0];
    
    switch (static_cast<QuicFrameType>(frame_type)) {
      case QuicFrameType::STREAM:
        return processStreamFrame(frame_data, connection);
      case QuicFrameType::CRYPTO:
        return processCryptoFrame(frame_data, connection);
      case QuicFrameType::ACK:
        return processAckFrame(frame_data, connection);
      default:
        return false;
    }
  }

  bool QuicTransport::processStreamFrame(const std::vector<uint8_t>& frame_data, QuicConnection* connection) {
    // Simplified stream frame processing
    // In a real implementation, this would parse the stream ID, offset, length, and data
    
    (void)frame_data;
    
    auto& logger = Logger::getInstance();
    logger.debug("Processing QUIC stream frame");
    
    // For HTTP/3 integration, we would extract the stream data and call the callback
    if (on_stream_data) {
      // Extract stream ID and data (simplified)
      QuicStreamId stream_id = 0; // Would be parsed from frame
      std::vector<uint8_t> data; // Would be extracted from frame
      
      on_stream_data(connection->connection_id, stream_id, data);
    }
    
    return true;
  }

  bool QuicTransport::processCryptoFrame(const std::vector<uint8_t>& frame_data, QuicConnection* connection) {
    // Simplified crypto frame processing
    (void)frame_data;
    (void)connection;
    
    auto& logger = Logger::getInstance();
    logger.debug("Processing QUIC crypto frame");
    
    return true;
  }

  bool QuicTransport::processAckFrame(const std::vector<uint8_t>& frame_data, QuicConnection* connection) {
    // Simplified ACK frame processing
    (void)frame_data;
    (void)connection;
    
    auto& logger = Logger::getInstance();
    logger.debug("Processing QUIC ACK frame");
    
    return true;
  }

  QuicStream* QuicTransport::getOrCreateStream(QuicConnection* connection, QuicStreamId stream_id, QuicStreamType type) {
    std::lock_guard<std::mutex> lock(connection->streams_mutex);
    
    auto it = connection->streams.find(stream_id);
    if (it != connection->streams.end()) {
      return it->second.get();
    }
    
    // Create new stream
    auto stream = std::make_unique<QuicStream>(stream_id, type);
    QuicStream* stream_ptr = stream.get();
    connection->streams[stream_id] = std::move(stream);
    
    return stream_ptr;
  }

  bool QuicTransport::sendStreamData(QuicConnection* connection, QuicStreamId stream_id, const std::vector<uint8_t>& data, bool fin) {
    // Simplified stream data sending
    (void)connection;
    (void)data;
    (void)fin;
    
    auto& logger = Logger::getInstance();
    logger.debug("Sending QUIC stream data on stream " + std::to_string(stream_id));
    
    // In a real implementation, this would create a STREAM frame and send it
    return true;
  }

  std::vector<uint8_t> QuicTransport::receiveStreamData(QuicConnection* connection, QuicStreamId stream_id) {
    std::lock_guard<std::mutex> lock(connection->streams_mutex);
    
    auto it = connection->streams.find(stream_id);
    if (it == connection->streams.end()) {
      return {};
    }
    
    QuicStream* stream = it->second.get();
    std::vector<uint8_t> data = stream->recv_buffer;
    stream->recv_buffer.clear();
    
    return data;
  }

  std::vector<uint8_t> QuicTransport::createInitialPacket(const QuicConnectionId& connection_id, const std::vector<uint8_t>& payload) {
    // Simplified initial packet creation
    std::vector<uint8_t> packet;
    
    // Long header (1 byte)
    packet.push_back(0xC0); // Version 1, Initial packet
    
    // Version (4 bytes)
    packet.push_back(0x00);
    packet.push_back(0x00);
    packet.push_back(0x00);
    packet.push_back(0x01);
    
    // Connection ID length and data
    packet.push_back(static_cast<uint8_t>(connection_id.data.size()));
    packet.insert(packet.end(), connection_id.data.begin(), connection_id.data.end());
    
    // Payload
    packet.insert(packet.end(), payload.begin(), payload.end());
    
    return packet;
  }

  std::vector<uint8_t> QuicTransport::createHandshakePacket(const QuicConnectionId& connection_id, const std::vector<uint8_t>& payload) {
    // Simplified handshake packet creation
    std::vector<uint8_t> packet;
    
    // Long header (1 byte)
    packet.push_back(0xE0); // Version 1, Handshake packet
    
    // Version (4 bytes)
    packet.push_back(0x00);
    packet.push_back(0x00);
    packet.push_back(0x00);
    packet.push_back(0x01);
    
    // Connection ID length and data
    packet.push_back(static_cast<uint8_t>(connection_id.data.size()));
    packet.insert(packet.end(), connection_id.data.begin(), connection_id.data.end());
    
    // Payload
    packet.insert(packet.end(), payload.begin(), payload.end());
    
    return packet;
  }

  std::vector<uint8_t> QuicTransport::createShortHeaderPacket(const QuicConnectionId& connection_id, const std::vector<uint8_t>& payload) {
    // Simplified short header packet creation
    std::vector<uint8_t> packet;
    
    // Short header (1 byte)
    packet.push_back(0x40); // Key phase bit, packet number length
    
    // Connection ID
    packet.insert(packet.end(), connection_id.data.begin(), connection_id.data.end());
    
    // Payload
    packet.insert(packet.end(), payload.begin(), payload.end());
    
    return packet;
  }

  std::vector<uint8_t> QuicTransport::createStreamFrame(QuicStreamId stream_id, const std::vector<uint8_t>& data, bool fin) {
    (void)fin;
    
    std::vector<uint8_t> frame;
    
    // Frame type
    frame.push_back(static_cast<uint8_t>(QuicFrameType::STREAM));
    
    // Stream ID (simplified - would use variable-length encoding)
    frame.push_back(static_cast<uint8_t>(stream_id & 0xFF));
    
    // Data length (simplified)
    frame.push_back(static_cast<uint8_t>(data.size() & 0xFF));
    
    // Data
    frame.insert(frame.end(), data.begin(), data.end());
    
    return frame;
  }

  void QuicTransport::cleanupConnection(const QuicConnectionId& connection_id) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.erase(connection_id);
    
    if (on_connection_closed) {
      on_connection_closed(connection_id);
    }
  }

  void QuicTransport::sendConnectionClose(QuicConnection* connection, uint64_t error_code, const std::string& reason) {
    // Simplified connection close sending
    (void)error_code;
    
    auto& logger = Logger::getInstance();
    logger.info("Sending QUIC connection close: " + reason);
    
    connection->is_active = false;
  }

  // Configuration methods
  void QuicTransport::setMaxStreamData(uint32_t max_data) {
    max_stream_data_ = max_data;
  }

  void QuicTransport::setMaxData(uint32_t max_data) {
    max_data_ = max_data;
  }

  void QuicTransport::setMaxStreams(uint32_t max_bidi, uint32_t max_uni) {
    max_streams_bidi_ = max_bidi;
    max_streams_uni_ = max_uni;
  }

  void QuicTransport::setIdleTimeout(uint32_t timeout_ms) {
    idle_timeout_ = timeout_ms;
  }

  size_t QuicTransport::getActiveConnections() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(connections_mutex_));
    return connections_.size();
  }

  std::vector<QuicConnectionId> QuicTransport::getConnectionIds() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(connections_mutex_));
    std::vector<QuicConnectionId> ids;
    ids.reserve(connections_.size());
    
    for (const auto& [id, connection] : connections_) {
      ids.push_back(id);
    }
    
    return ids;
  }

  bool QuicTransport::sendData(QuicConnectionId connection_id, QuicStreamId stream_id, const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    auto it = connections_.find(connection_id);
    if (it == connections_.end()) {
      return false;
    }
    
    return sendStreamData(it->second.get(), stream_id, data);
  }

  std::vector<uint8_t> QuicTransport::receiveData(QuicConnectionId connection_id, QuicStreamId stream_id) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    auto it = connections_.find(connection_id);
    if (it == connections_.end()) {
      return {};
    }
    
    return receiveStreamData(it->second.get(), stream_id);
  }

  bool QuicTransport::closeStream(QuicConnectionId connection_id, QuicStreamId stream_id) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    auto it = connections_.find(connection_id);
    if (it == connections_.end()) {
      return false;
    }
    
    std::lock_guard<std::mutex> stream_lock(it->second->streams_mutex);
    auto stream_it = it->second->streams.find(stream_id);
    if (stream_it != it->second->streams.end()) {
      stream_it->second->state = QuicStreamState::CLOSED;
      
      if (on_stream_closed) {
        on_stream_closed(connection_id, stream_id);
      }
    }
    
    return true;
  }

} // namespace netd::shared
