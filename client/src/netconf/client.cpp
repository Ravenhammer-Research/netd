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
#include <shared/include/request/get/config.hpp>
#include <shared/include/request/commit.hpp>
#include <shared/include/request/edit.hpp>
#include <shared/include/xml/envelope.hpp>
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

    transport_ = netd::shared::BaseTransport::create(transport_type_);
    if (!transport_) {
      throw netd::shared::TransportError("Failed to create transport");
    }

    std::string formatted_address = netd::shared::BaseTransport::formatAddress(transport_type_, server_address_, port_);

    if (!transport_->connect(formatted_address)) {
      throw netd::shared::ConnectionError("Failed to connect to server at " + formatted_address);
    }

    int socket = transport_->getSocket();
    if (socket < 0) {
      throw netd::shared::TransportError("Invalid socket after connection");
    }

    auto& yang = netd::shared::Yang::getInstance();
    ly_ctx* ctx = yang.getContext();

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


  void NetconfClient::rpcResponseReceiveWait(
    netd::shared::RpcRxStream& rpc_stream, 
    netd::shared::netconf::NetconfSession* session) {
    
    if (!session) {
      return;
    }

    while (isConnected() && session->isConnected()) {
      if (rpc_stream.hasData()) {
        netd::client::netconf::ClientRpc::processRpc(rpc_stream, session);
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
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
    
    netd::shared::ClientSocket client_socket(transport_->getSocket());
    netd::shared::RpcRxStream rpc_stream(client_socket);
    
    rpcResponseReceiveWait(rpc_stream, session_.get());
  }

  std::string NetconfClient::sendRequest(const netd::shared::request::get::GetConfigRequest& request) {
    if (!connected_ || !session_) {
      throw std::runtime_error("Not connected to NETCONF server");
    }

    try {
      // Get YANG context
      auto& yang = netd::shared::Yang::getInstance();
      ly_ctx* ctx = yang.getContext();
      
      // Convert request to YANG
      lyd_node* rpc_node = request.toYang(ctx);
      if (!rpc_node) {
        throw std::runtime_error("Failed to create get-config YANG node");
      }
      
      // Create RPC envelope using the static toXml method
      auto envelope = netd::shared::xml::RpcEnvelope::toXml(
          netd::shared::xml::RpcType::RPC,
          session_->getNextMessageId(),
          netd::shared::netconf::NetconfOperation::GET_CONFIG,
          nullptr, // No filter
          rpc_node,
          ctx
      );
      
      // Send the request
      netd::shared::ClientSocket client_socket(transport_->getSocket());
      netd::shared::RpcTxStream tx_stream(client_socket);
      
      std::string envelope_xml = envelope->toString(ctx);
      tx_stream << envelope_xml;
      tx_stream.flush();
      
      // Read the response
      netd::shared::RpcRxStream rx_stream(client_socket);
      std::string response_xml = rx_stream.readToEnd();
      
      // Clean up YANG node
      lyd_free_tree(rpc_node);
      
      return response_xml;
      
    } catch (const std::exception& e) {
      throw std::runtime_error("Failed to send get-config request: " + std::string(e.what()));
    }
  }

  std::string NetconfClient::sendRequest(const netd::shared::request::CommitRequest& request) {
    if (!connected_ || !session_) {
      throw std::runtime_error("Not connected to NETCONF server");
    }

    try {
      // Get YANG context
      auto& yang = netd::shared::Yang::getInstance();
      ly_ctx* ctx = yang.getContext();
      
      // Convert request to YANG
      lyd_node* rpc_node = request.toYang(ctx);
      if (!rpc_node) {
        throw std::runtime_error("Failed to create commit YANG node");
      }
      
      // Create RPC envelope using the static toXml method
      auto envelope = netd::shared::xml::RpcEnvelope::toXml(
          netd::shared::xml::RpcType::RPC,
          session_->getNextMessageId(),
          netd::shared::netconf::NetconfOperation::COMMIT,
          nullptr, // No filter
          rpc_node,
          ctx
      );
      
      // Send the request
      netd::shared::ClientSocket client_socket(transport_->getSocket());
      netd::shared::RpcTxStream tx_stream(client_socket);
      
      std::string envelope_xml = envelope->toString(ctx);
      tx_stream << envelope_xml;
      tx_stream.flush();
      
      // Read the response
      netd::shared::RpcRxStream rx_stream(client_socket);
      std::string response_xml = rx_stream.readToEnd();
      
      // Clean up YANG node
      lyd_free_tree(rpc_node);
      
      return response_xml;
      
    } catch (const std::exception& e) {
      throw std::runtime_error("Failed to send commit request: " + std::string(e.what()));
    }
  }

  std::string NetconfClient::sendRequest(const netd::shared::request::EditConfigRequest& request) {
    if (!connected_ || !session_) {
      throw std::runtime_error("Not connected to NETCONF server");
    }

    try {
      // Get YANG context
      auto& yang = netd::shared::Yang::getInstance();
      ly_ctx* ctx = yang.getContext();
      
      // Convert request to YANG
      lyd_node* rpc_node = request.toYang(ctx);
      if (!rpc_node) {
        throw std::runtime_error("Failed to create edit-config YANG node");
      }
      
      // Create RPC envelope using the static toXml method
      auto envelope = netd::shared::xml::RpcEnvelope::toXml(
          netd::shared::xml::RpcType::RPC,
          session_->getNextMessageId(),
          netd::shared::netconf::NetconfOperation::EDIT_CONFIG,
          nullptr, // No filter
          rpc_node,
          ctx
      );
      
      // Send the request
      netd::shared::ClientSocket client_socket(transport_->getSocket());
      netd::shared::RpcTxStream tx_stream(client_socket);
      
      std::string envelope_xml = envelope->toString(ctx);
      tx_stream << envelope_xml;
      tx_stream.flush();
      
      // Read the response
      netd::shared::RpcRxStream rx_stream(client_socket);
      std::string response_xml = rx_stream.readToEnd();
      
      // Clean up YANG node
      lyd_free_tree(rpc_node);
      
      return response_xml;
      
    } catch (const std::exception& e) {
      throw std::runtime_error("Failed to send edit-config request: " + std::string(e.what()));
    }
  }

} // namespace netd::client::netconf
