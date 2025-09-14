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
#include <shared/include/exception.hpp>
#include <shared/include/unix.hpp>
#include <shared/include/netconf/session.hpp>
#include <shared/include/stream.hpp>
#include <shared/include/socket.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/yang.hpp>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <sstream>
#include <thread>
#include <chrono>

namespace netd::client::netconf {

  NetconfClient::NetconfClient(netd::shared::TransportType transport_type, 
                               const std::string& server_address, 
                               int port)
      : transport_type_(transport_type), server_address_(server_address), port_(port), connected_(false) {
  }

  NetconfClient::~NetconfClient() {
    disconnect(true); // Close session when client is exiting
  }

  bool NetconfClient::connect() {
    if (connected_) {
      return true;
    }

    // Create transport using factory method
    transport_ = netd::shared::BaseTransport::create(transport_type_);
    if (!transport_) {
      throw netd::shared::TransportError("Failed to create transport");
    }

    // Format the server address based on transport type
    std::string formatted_address = netd::shared::BaseTransport::formatAddress(transport_type_, server_address_, port_);

    // Connect to server
    if (!transport_->connect(formatted_address)) {
      throw netd::shared::ConnectionError("Failed to connect to server at " + formatted_address);
    }

    // Get socket for session
    int socket = transport_->getSocket();
    if (socket < 0) {
      throw netd::shared::TransportError("Invalid socket after connection");
    }

    // Create YANG context
    auto& yang = netd::shared::Yang::getInstance();
    ly_ctx* ctx = yang.getContext();

    // Create NETCONF session
    session_ = std::make_unique<netd::shared::netconf::NetconfSession>(
        ctx, socket, transport_type_);

    connected_ = true;
    return true;
  }

  void NetconfClient::disconnect(bool close_session) {
    if (!connected_) {
      return;
    }

    if (session_) {
      if (close_session) {
        session_->close();
      }
      session_.reset();
    }

    if (transport_) {
      transport_->disconnect();
      transport_.reset();
    }

    connected_ = false;
  }

  bool NetconfClient::isConnected() const {
    return connected_ && session_ && session_->isConnected();
  }


  netd::shared::netconf::NetconfSession* NetconfClient::getSession() const {
    return session_.get();
  }

  void NetconfClient::handleServerSession() {
    if (!isConnected()) {
      throw netd::shared::TransportError("Not connected to server");
    }

    if (!transport_) {
      throw netd::shared::TransportError("Transport not available");
    }

    if (!session_) {
      throw netd::shared::TransportError("Session not available");
    }
    
    netd::shared::Logger::getInstance().debug("Starting client message processing loop");
    while (isConnected() && session_->isConnected()) {
      netd::shared::ClientSocket client_socket(transport_->getSocket());
      netd::shared::RpcRxStream rpc_stream(client_socket);
      
      if (rpc_stream.hasData()) {
        netd::client::netconf::ClientRpc::processRpc(rpc_stream, session_.get());
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
  }

} // namespace netd::client::netconf
