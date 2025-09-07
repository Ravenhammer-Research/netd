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

#include <cstring>
#include <libnetconf2/messages_client.h>
#include <libnetconf2/session_client.h>
#include <libyang/libyang.h>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/request/edit.hpp>
#include <shared/include/request/get/base.hpp>
#include <shared/include/response/get/config.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/yang.hpp>
#include <client/include/netconf.hpp>
#include <stdexcept>

namespace netd::client {
  using netd::shared::Logger;
  using netd::shared::Yang;

  // NetconfClient implementation
  NetconfClient::NetconfClient() : session_(nullptr), connected_(false) {
  }

  NetconfClient::~NetconfClient() {
    if (connected_) {
      disconnect();
    }
  }

  bool NetconfClient::connect(const std::string &socketPath) {
    auto &logger = netd::shared::Logger::getInstance();

    struct ly_ctx *ctx = Yang::getInstance().getContext();

    session_ = nc_connect_unix(socketPath.c_str(), ctx);
    if (!session_) {
      logger.error("Failed to connect to NETD server at " + socketPath);
      return false;
    }

    connected_ = true;
    logger.info("Connected to NETD server at " + socketPath);
    return true;
  }

  void NetconfClient::disconnect() {
    if (session_) {
      nc_session_free(session_, nullptr);
      session_ = nullptr;
    }
    connected_ = false;
  }

  bool NetconfClient::isConnected() const { 
    return connected_; 
  }
  
  struct nc_session *NetconfClient::getSession() const { 
    return session_; 
  }

  netd::shared::response::get::GetConfigResponse NetconfClient::sendRequest(struct nc_rpc *rpc) {
    if (!connected_) {
      throw netd::shared::ConnectionError("Not connected to server");
    }

    // Send the RPC
    uint64_t msgid;
    int ret = nc_send_rpc(session_, rpc, 1024, &msgid);
    if (ret != NC_MSG_RPC) {
      nc_rpc_free(rpc);
      throw netd::shared::NetworkError("Failed to send RPC");
    }

    // Receive response
    struct lyd_node *envp = nullptr;
    struct lyd_node *op = nullptr;
    ret = nc_recv_reply(session_, rpc, msgid, 1024, &envp, &op);
    nc_rpc_free(rpc);

    auto &logger = netd::shared::Logger::getInstance();
    logger.debug("nc_recv_reply returned: " + std::to_string(ret) + " (expected: " + std::to_string(NC_MSG_REPLY) + ")");

    if (ret != NC_MSG_REPLY) {
      throw netd::shared::NetworkError("Failed to receive response, got return code: " + std::to_string(ret));
    }

    // Check if we received valid operation data
    if (!op) {
      lyd_free_tree(envp);
      throw netd::shared::NetworkError("Received empty response from server");
    }

    // Create GetConfigResponse from the received data
    netd::shared::response::get::GetConfigResponse response;
    
    // Parse the received data using the response's fromYang method
    auto parsedResponse = response.fromYang(Yang::getInstance().getContext(), op);
    if (parsedResponse) {
      response = std::move(*static_cast<netd::shared::response::get::GetConfigResponse*>(parsedResponse.get()));
    }
    
    lyd_free_tree(envp);
    lyd_free_tree(op);
    
    return response;
  }


  // Global functions
  static NetconfClient *g_netconfClient = nullptr;
  
  NetconfClient &getNetconfClient() {
    if (!g_netconfClient) {
      g_netconfClient = new NetconfClient();
    }
    return *g_netconfClient;
  }
  
  bool connectToServer(const std::string &socketPath) {
    if (!g_netconfClient) {
      g_netconfClient = new NetconfClient();
    }
    return g_netconfClient->connect(socketPath);
  }

  void disconnectFromServer() {
    if (g_netconfClient) {
      g_netconfClient->disconnect();
    }
  }

  bool isConnectedToServer() {
    return g_netconfClient ? g_netconfClient->isConnected() : false;
  }


} // namespace netd::client