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
#include <shared/include/netconf/session.hpp>
#include <shared/include/request/commit.hpp>
#include <shared/include/request/edit.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/socket.hpp>
#include <shared/include/stream.hpp>
#include <shared/include/unix.hpp>
#include <shared/include/xml/envelope.hpp>
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

  netd::shared::netconf::NetconfSession *NetconfClient::getSession() const {
    return session_.get();
  }

  void NetconfClient::rpcResponseReceiveWait(
      netd::shared::RpcRxStream &rpc_stream,
      netd::shared::netconf::NetconfSession *session) {

    if (!session) {
      return;
    }

    if (rpc_stream.hasData()) {
      netd::client::netconf::ClientRpc::processRpc(rpc_stream, session);
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  std::string NetconfClient::sendRequest(
      const netd::shared::request::get::GetConfigRequest &) {
    throw netd::shared::NotImplementedError("Not implemented");
  }

  std::string
  NetconfClient::sendRequest(const netd::shared::request::CommitRequest &) {
    throw netd::shared::NotImplementedError("Not implemented");
  }

  std::string
  NetconfClient::sendRequest(const netd::shared::request::EditConfigRequest &) {
    throw netd::shared::NotImplementedError("Not implemented");
  }

} // namespace netd::client::netconf
