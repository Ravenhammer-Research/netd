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

#ifndef NETD_SHARED_QUIC_HPP
#define NETD_SHARED_QUIC_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>
#include <thread>
#include <cstdint>
#include <shared/include/transport.hpp>

namespace netd::shared {

  // QUIC Connection ID
  struct QuicConnectionId {
    std::vector<uint8_t> data;
    bool operator==(const QuicConnectionId& other) const {
      return data == other.data;
    }
  };

  // Hash function for QuicConnectionId
  struct QuicConnectionIdHash {
    std::size_t operator()(const QuicConnectionId& id) const {
      std::size_t hash = 0;
      for (uint8_t byte : id.data) {
        hash ^= std::hash<uint8_t>{}(byte) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
      }
      return hash;
    }
  };

  // QUIC Stream ID
  using QuicStreamId = uint64_t;

  // QUIC Stream types
  enum class QuicStreamType {
    BIDIRECTIONAL,
    UNIDIRECTIONAL
  };

  // QUIC Stream state
  enum class QuicStreamState {
    IDLE,
    OPEN,
    HALF_CLOSED_LOCAL,
    HALF_CLOSED_REMOTE,
    CLOSED
  };

  // QUIC Stream structure
  struct QuicStream {
    QuicStreamId stream_id;
    QuicStreamType type;
    QuicStreamState state;
    std::vector<uint8_t> send_buffer;
    std::vector<uint8_t> recv_buffer;
    bool fin_sent = false;
    bool fin_received = false;
    
    QuicStream(QuicStreamId id, QuicStreamType t) 
      : stream_id(id), type(t), state(QuicStreamState::IDLE) {}
  };

  // QUIC Connection structure
  struct QuicConnection {
    QuicConnectionId connection_id;
    std::unordered_map<QuicStreamId, std::unique_ptr<QuicStream>> streams;
    std::atomic<bool> is_active{true};
    std::mutex streams_mutex;
    
    // Connection metadata
    std::string peer_address;
    uint16_t peer_port;
    uint64_t created_time;
    
    QuicConnection(const QuicConnectionId& id, const std::string& addr, uint16_t port)
      : connection_id(id), peer_address(addr), peer_port(port) {
      created_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    }
  };

  // QUIC Packet types
  enum class QuicPacketType {
    INITIAL,
    HANDSHAKE,
    ZERO_RTT,
    RETRY,
    VERSION_NEGOTIATION,
    SHORT_HEADER
  };

  // QUIC Frame types
  enum class QuicFrameType {
    PADDING = 0x00,
    PING = 0x01,
    ACK = 0x02,
    RESET_STREAM = 0x04,
    STOP_SENDING = 0x05,
    CRYPTO = 0x06,
    NEW_TOKEN = 0x07,
    STREAM = 0x08,
    MAX_DATA = 0x10,
    MAX_STREAM_DATA = 0x11,
    MAX_STREAMS = 0x12,
    DATA_BLOCKED = 0x14,
    STREAM_DATA_BLOCKED = 0x15,
    STREAMS_BLOCKED = 0x16,
    NEW_CONNECTION_ID = 0x18,
    RETIRE_CONNECTION_ID = 0x19,
    PATH_CHALLENGE = 0x1a,
    PATH_RESPONSE = 0x1b,
    CONNECTION_CLOSE = 0x1c,
    HANDSHAKE_DONE = 0x1e
  };

  // QUIC Transport class
  class QuicTransport : public BaseTransport {
  private:
    // Server configuration
    std::string listen_address_;
    uint16_t listen_port_;
    std::atomic<bool> listening_{false};
    std::atomic<bool> should_stop_{false};
    
    // Server socket
    int server_socket_ = -1;
    
    // Connection management
    std::unordered_map<QuicConnectionId, std::unique_ptr<QuicConnection>, QuicConnectionIdHash> connections_;
    std::mutex connections_mutex_;
    
    // Threading
    std::vector<std::thread> worker_threads_;
    static constexpr size_t MAX_WORKER_THREADS = 10;
    
    // QUIC configuration
    uint32_t max_stream_data_ = 1048576; // 1MB
    uint32_t max_data_ = 10485760; // 10MB
    uint32_t max_streams_bidi_ = 100;
    uint32_t max_streams_uni_ = 100;
    uint32_t idle_timeout_ = 30000; // 30 seconds
    
    // Helper methods
    bool setupServerSocket();
    void acceptConnections();
    void handleConnection(std::unique_ptr<QuicConnection> connection);
    QuicConnectionId generateConnectionId();
    QuicStreamId generateStreamId(QuicStreamType type);
    
    // Packet processing
    bool processPacket(const std::vector<uint8_t>& packet, QuicConnection* connection);
    bool processInitialPacket(const std::vector<uint8_t>& packet, QuicConnection* connection);
    bool processHandshakePacket(const std::vector<uint8_t>& packet, QuicConnection* connection);
    bool processShortHeaderPacket(const std::vector<uint8_t>& packet, QuicConnection* connection);
    
    // Frame processing
    bool processFrame(const std::vector<uint8_t>& frame_data, QuicConnection* connection);
    bool processStreamFrame(const std::vector<uint8_t>& frame_data, QuicConnection* connection);
    bool processCryptoFrame(const std::vector<uint8_t>& frame_data, QuicConnection* connection);
    bool processAckFrame(const std::vector<uint8_t>& frame_data, QuicConnection* connection);
    
    // Stream management
    QuicStream* getOrCreateStream(QuicConnection* connection, QuicStreamId stream_id, QuicStreamType type);
    bool sendStreamData(QuicConnection* connection, QuicStreamId stream_id, const std::vector<uint8_t>& data, bool fin = false);
    std::vector<uint8_t> receiveStreamData(QuicConnection* connection, QuicStreamId stream_id);
    
    // Packet generation
    std::vector<uint8_t> createInitialPacket(const QuicConnectionId& connection_id, const std::vector<uint8_t>& payload);
    std::vector<uint8_t> createHandshakePacket(const QuicConnectionId& connection_id, const std::vector<uint8_t>& payload);
    std::vector<uint8_t> createShortHeaderPacket(const QuicConnectionId& connection_id, const std::vector<uint8_t>& payload);
    std::vector<uint8_t> createStreamFrame(QuicStreamId stream_id, const std::vector<uint8_t>& data, bool fin = false);
    
    // Connection management
    void cleanupConnection(const QuicConnectionId& connection_id);
    void sendConnectionClose(QuicConnection* connection, uint64_t error_code, const std::string& reason);

  public:
    QuicTransport();
    ~QuicTransport();
    
    // BaseTransport interface
    bool start(const std::string& address) override;
    void stop() override;
    bool isListening() const override;
    int acceptConnection() override;
    void closeConnection(int socket_fd) override;
    bool connect(const std::string& address) override;
    void disconnect() override;
    int getSocket() const override;
    bool sendData(int socket_fd, const std::string& data) override;
    std::string receiveData(int socket_fd) override;
    bool hasData(int socket_fd) override;
    
    // Cancellation support
    void cancelOperation(int socket_fd) override;
    
    const std::string& getAddress() const override;
    
    // QUIC-specific interface
    bool start(const std::string& address, uint16_t port);
    uint16_t getPort() const;
    
    // QUIC-specific configuration
    void setMaxStreamData(uint32_t max_data);
    void setMaxData(uint32_t max_data);
    void setMaxStreams(uint32_t max_bidi, uint32_t max_uni);
    void setIdleTimeout(uint32_t timeout_ms);
    
    // Connection information
    size_t getActiveConnections() const;
    std::vector<QuicConnectionId> getConnectionIds() const;
    
    // Stream operations
    bool sendData(QuicConnectionId connection_id, QuicStreamId stream_id, const std::vector<uint8_t>& data);
    std::vector<uint8_t> receiveData(QuicConnectionId connection_id, QuicStreamId stream_id);
    bool closeStream(QuicConnectionId connection_id, QuicStreamId stream_id);
    
    // Event callbacks (for HTTP/3 integration)
    std::function<void(QuicConnectionId, QuicStreamId, const std::vector<uint8_t>&)> on_stream_data;
    std::function<void(QuicConnectionId)> on_connection_closed;
    std::function<void(QuicConnectionId, QuicStreamId)> on_stream_closed;
  };

} // namespace netd::shared

#endif // NETD_SHARED_QUIC_HPP
