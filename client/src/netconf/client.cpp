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

#include <client/include/netconf/client.hpp>
#include <client/include/netconf/rpc.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/unix.hpp>
#include <shared/include/http.hpp>
#include <shared/include/sctp.hpp>
#include <shared/include/quic.hpp>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sstream>
#include <stdexcept>

namespace netd::client::netconf {

  NetconfClient& NetconfClient::getInstance() {
    static NetconfClient instance;
    return instance;
  }

  NetconfClient::NetconfClient() 
      : connected_(false), auto_reconnect_enabled_(false), should_stop_reconnect_(false),
        connection_socket_fd_(-1), max_retries_(5), initial_delay_ms_(1000) {
    // Default to Unix transport
    transport_ = std::make_unique<netd::shared::UnixTransport>();
    // Initialize RPC handler
  }

  NetconfClient::~NetconfClient() {
    stopAutoReconnect();
    disconnect();
  }

  void NetconfClient::setTransport(std::unique_ptr<netd::shared::BaseTransport> transport) {
    if (!transport) {
      throw netd::shared::ArgumentError("Transport cannot be null");
    }
    
    if (isConnected()) {
      disconnect();
    }
    
    transport_ = std::move(transport);
  }

  netd::shared::BaseTransport* NetconfClient::getTransport() const {
    return transport_.get();
  }

  void NetconfClient::connect(const std::string& address) {
    validateTransport();
    
    if (isConnected()) {
      disconnect();
    }

    try {
      if (!transport_->start(address)) {
        throw netd::shared::ConnectionError("Failed to start transport on address: " + address);
      }
      
      connection_address_ = address;
      connected_ = true;
      
      auto& logger = netd::shared::Logger::getInstance();
      logger.info("Connected to " + address + " using " + typeid(decltype(*transport_)).name());
      
    } catch (const netd::shared::TransportError& e) {
      throw netd::shared::ConnectionError("Transport error during connection: " + std::string(e.what()));
    } catch (const std::exception& e) {
      throw netd::shared::ConnectionError("Unexpected error during connection: " + std::string(e.what()));
    }
  }

  void NetconfClient::connectWithRetry(const std::string& address, int max_retries, int initial_delay_ms) {
    max_retries_ = max_retries;
    initial_delay_ms_ = initial_delay_ms;
    
    for (int attempt = 0; attempt < max_retries; ++attempt) {
      try {
        connect(address);
        return; // Success
      } catch (const netd::shared::ConnectionError& e) {
        auto& logger = netd::shared::Logger::getInstance();
        logger.warning("Connection attempt " + std::to_string(attempt + 1) + " failed: " + e.what());
        
        if (attempt < max_retries - 1) {
          sleepWithBackoff(attempt, initial_delay_ms);
        }
      }
    }
    
    throw netd::shared::ConnectionError("Failed to connect after " + std::to_string(max_retries) + " attempts");
  }

  void NetconfClient::disconnect() {
    if (transport_ && connected_) {
      try {
        transport_->stop();
      } catch (const std::exception& e) {
        auto& logger = netd::shared::Logger::getInstance();
        logger.warning("Error during transport stop: " + std::string(e.what()));
      }
    }
    
    connected_ = false;
    connection_address_.clear();
    connection_socket_fd_ = -1;
  }

  bool NetconfClient::isConnected() const {
    return connected_ && transport_ && transport_->isListening();
  }

  void NetconfClient::sendData(const std::string& data) {
    validateConnection();
    
    try {
      // For now, we'll use a default socket FD of 0 for the transport
      // In a real implementation, we'd need to track the actual connection FD
      if (!transport_->sendData(connection_socket_fd_, data)) {
        throw netd::shared::SendError("Failed to send data via transport");
      }
    } catch (const netd::shared::TransportError& e) {
      throw netd::shared::SendError("Transport error during send: " + std::string(e.what()));
    } catch (const std::exception& e) {
      throw netd::shared::SendError("Unexpected error during send: " + std::string(e.what()));
    }
  }

  std::string NetconfClient::receiveData() {
    validateConnection();
    
    try {
      // For now, we'll use a default socket FD of 0 for the transport
      std::string data = transport_->receiveData(connection_socket_fd_);
      if (data.empty()) {
        throw netd::shared::ReceiveError("No data received from transport");
      }
      return data;
    } catch (const netd::shared::TransportError& e) {
      throw netd::shared::ReceiveError("Transport error during receive: " + std::string(e.what()));
    } catch (const std::exception& e) {
      throw netd::shared::ReceiveError("Unexpected error during receive: " + std::string(e.what()));
    }
  }

  void NetconfClient::sendRpc(std::istream& rpc_stream) {
    // Suppress unused parameter warning
    (void)rpc_stream;
    
    // TODO: Implement RPC sending
    throw netd::shared::NotImplementedError("RPC sending not implemented");
  }

  std::string NetconfClient::receiveRpcReply() {
    // TODO: Implement RPC reply receiving
    throw netd::shared::NotImplementedError("RPC reply receiving not implemented");
  }


  void NetconfClient::startAutoReconnect(const std::string& address, int max_retries, int initial_delay_ms) {
    if (auto_reconnect_enabled_) {
      stopAutoReconnect();
    }
    
    auto_reconnect_enabled_ = true;
    should_stop_reconnect_ = false;
    max_retries_ = max_retries;
    initial_delay_ms_ = initial_delay_ms;
    connection_address_ = address;
    
    reconnect_thread_ = std::thread(&NetconfClient::reconnectLoop, this);
  }

  void NetconfClient::stopAutoReconnect() {
    if (auto_reconnect_enabled_) {
      should_stop_reconnect_ = true;
      auto_reconnect_enabled_ = false;
      
      if (reconnect_thread_.joinable()) {
        reconnect_thread_.join();
      }
    }
  }

  void NetconfClient::reconnectLoop() {
    while (!should_stop_reconnect_ && auto_reconnect_enabled_) {
      if (!isConnected()) {
        try {
          connectWithRetry(connection_address_, max_retries_, initial_delay_ms_);
        } catch (const netd::shared::ConnectionError& e) {
          auto& logger = netd::shared::Logger::getInstance();
          logger.error("Auto-reconnect failed: " + std::string(e.what()));
          std::this_thread::sleep_for(std::chrono::milliseconds(initial_delay_ms_));
        }
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
    }
  }

  void NetconfClient::sleepWithBackoff(int attempt, int base_delay_ms) {
    int delay = base_delay_ms * (1 << attempt); // Exponential backoff
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
  }

  void NetconfClient::validateTransport() const {
    if (!transport_) {
      throw netd::shared::ConfigurationError("No transport configured");
    }
  }

  void NetconfClient::validateConnection() const {
    validateTransport();
    if (!isConnected()) {
      throw netd::shared::ConnectionError("Not connected to server");
    }
  }


} // namespace netd::client::netconf
