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

#include <chrono>
#include <client/include/netconf/client.hpp>
#include <client/include/netconf/rpc.hpp>
#include <cstring>
#include <errno.h>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/netconf/rpc/stream.hpp>
#include <shared/include/netconf/session.hpp>
#include <shared/include/request/commit.hpp>
#include <shared/include/request/edit.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/request/get/library.hpp>
#include <shared/include/request/hello.hpp>
#include <shared/include/socket.hpp>
#include <shared/include/unix.hpp>
#include <shared/include/xml/envelope.hpp>
#include <shared/include/xml/hello.hpp>
#include <shared/include/yang.hpp>
#include <sstream>
#include <thread>
#include <unistd.h>

namespace netd::client::netconf {

  NetconfClient::NetconfClient(netd::shared::TransportType transport_type,
                               const std::string &server_address, int port)
      : transport_type_(transport_type), server_address_(server_address),
        port_(port), connected_(false) {
    auto &yang = netd::shared::Yang::getInstance();
    ly_ctx *ctx = yang.getContext();

    session_ = std::make_unique<netd::shared::netconf::NetconfSession>(
        ctx, -1, transport_type_);
  }

  NetconfClient::~NetconfClient() { disconnect(true); }

  bool NetconfClient::connect() {
    if (connected_) {
      return true;
    }

    transport_ = netd::shared::BaseTransport::create(transport_type_);
    if (!transport_) {
      throw netd::shared::TransportError("Failed to create transport");
    }

    std::string formatted_address = netd::shared::BaseTransport::formatAddress(
        transport_type_, server_address_, port_);

    if (!transport_->connect(formatted_address)) {
      throw netd::shared::ConnectionError("Failed to connect to server at " +
                                          formatted_address);
    }

    int socket = transport_->getSocket();
    if (socket < 0) {
      throw netd::shared::TransportError("Invalid socket after connection");
    }

    session_->updateSocket(socket);

    connected_ = true;
    netd::shared::ClientSocket client_socket(socket);

    netd::shared::netconf::Rpc::sendHelloToServer(client_socket,
                                                  session_.get());

    netd::shared::RpcRxStream rpc_stream(client_socket);

    // Wait for server hello message
    auto &logger = netd::shared::Logger::getInstance();
    logger.debug("connect: Waiting for server hello message");

    for (int attempt = 1; attempt <= 3; attempt++) {
      logger.debug("connect: Attempt " + std::to_string(attempt) +
                   " to receive server hello");

      if (rpc_stream.hasData()) {
        try {
          rpcReceive(rpc_stream, session_.get());
          attempt = 1;
          continue;
        } catch (const std::exception &e) {
          logger.error("connect: Exception in rpcReceive: " +
                       std::string(e.what()));
          return false;
        }
      } else {
        logger.debug("Error in rpcReceive: " +
                     std::to_string(attempt));
      }

      if (attempt < 3) {
        logger.debug("connect: Sleeping 1 second before next attempt");
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }

    logger.error("connect: Failed to receive server hello after 3 attempts");
    throw netd::shared::ConnectionError(
        "Failed to receive server hello message");
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

  netd::shared::netconf::NetconfSession *NetconfClient::getSession() const {
    return session_.get();
  }

  void
  NetconfClient::rpcReceive(netd::shared::RpcRxStream &rpc_stream,
                            netd::shared::netconf::NetconfSession *session) {

    auto &logger = netd::shared::Logger::getInstance();
    logger.debug("rpcReceive: Starting");

    if (!session) {
      logger.error("rpcReceive: session is null");
      throw netd::shared::SessionError("session not found");
    }

    logger.debug("rpcReceive: Reading next message");
    std::string xml = rpc_stream.readNextMessage();
    logger.debug("rpcReceive: Read message, length=" +
                 std::to_string(xml.length()));

    if (netd::shared::xml::isRpcMessage(xml)) {
      logger.debug("rpcReceive: Processing RPC message");
      rpc_stream.rewindOne();
      netd::client::netconf::ClientRpc::processRpc(rpc_stream, session);
    } else if (netd::shared::xml::isHelloMessage(xml)) {
      logger.debug("rpcReceive: Processing hello message");
      auto yang_ctx = session->getContext();
      // XXX
      // auto hello = netd::shared::xml::HelloToClient::fromXml(xml, yang_ctx);
      // session->processHelloRequest(*hello);

      logger.debug("rpcReceive: Sending Yang library request");
      netd::shared::request::get::GetLibraryRequest library_request;
      lyd_node *yang_node = library_request.toYang(yang_ctx);

      auto envelope = netd::shared::xml::RpcEnvelope::toXml(
          netd::shared::xml::RpcType::RPC, 1,
          netd::shared::netconf::NetconfOperation::GET, nullptr, yang_node,
          yang_ctx);

      logger.debug("rpcReceive: Creating tx stream and sending envelope");
      netd::shared::RpcTxStream tx_stream(rpc_stream.getSocket());
      std::stringstream envelope_stream = envelope->toXmlStream(yang_ctx);

      std::string line;
      while (std::getline(envelope_stream, line)) {
        tx_stream << line << "\n";
      }
      tx_stream.flush();

      lyd_free_tree(yang_node);
      logger.debug("rpcReceive: Envelope sent");
    } else {
      logger.error("rpcReceive: received unknown message");
    }
  }

  bool NetconfClient::sendRequest(
      const netd::shared::request::get::GetConfigRequest &) {
    throw netd::shared::NotImplementedError("Not implemented");
  }

  bool
  NetconfClient::sendRequest(const netd::shared::request::CommitRequest &) {
    throw netd::shared::NotImplementedError("Not implemented");
  }

  bool
  NetconfClient::sendRequest(const netd::shared::request::EditConfigRequest &) {
    throw netd::shared::NotImplementedError("Not implemented");
  }

} // namespace netd::client::netconf
