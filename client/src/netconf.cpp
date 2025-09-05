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
#include <shared/include/logger.hpp>
#include <shared/include/request/edit.hpp>
#include <shared/include/request/get/base.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/yang.hpp>
#include <stdexcept>

namespace netd::client {
  using netd::shared::Logger;
  using netd::shared::Yang;

  class NetconfClient {
  public:
    NetconfClient() : session_(nullptr), connected_(false) {
      Yang::getInstance();
    }

    ~NetconfClient() {
      if (connected_) {
        disconnect();
      }
    }

    bool connect(const std::string &socketPath = "/tmp/netd.sock") {
      auto &logger = Logger::getInstance();

      // Use the shared YANG context
      struct ly_ctx *ctx = Yang::getInstance().getContext();

      // Connect to NETCONF server via Unix socket
      session_ = nc_connect_unix(socketPath.c_str(), ctx);
      if (!session_) {
        logger.error("Failed to connect to NETD server at " + socketPath);
        return false;
      }

      connected_ = true;
      logger.info("Connected to NETD server at " + socketPath);
      return true;
    }

    void disconnect() {
      if (session_) {
        nc_session_free(session_, nullptr);
        session_ = nullptr;
      }
      connected_ = false;
    }

    bool isConnected() const { return connected_; }

    // Send a NETCONF request and receive response
    std::string sendRequest(const std::string &request) {
      if (!connected_) {
        throw std::runtime_error("Not connected to server");
      }

      auto &logger = Logger::getInstance();

      // TODO: Parse request string to nc_rpc and send via nc_send_rpc
      // For now, return a placeholder response
      logger.debug("Sending request: " + request);
      return "NETCONF response placeholder";
    }

    // Convenience methods for common NETCONF operations
    std::string
    getConfig([[maybe_unused]] const std::string &source = "running") {
      if (!connected_) {
        throw std::runtime_error("Not connected to server");
      }

      // TODO: Implement actual get-config request
      return "get-config response placeholder";
    }

    std::string get([[maybe_unused]] const std::string &filter = "") {
      if (!connected_) {
        throw std::runtime_error("Not connected to server");
      }

      // TODO: Implement actual get request
      return "get response placeholder";
    }

    std::string
    editConfig([[maybe_unused]] const std::string &target = "candidate",
               [[maybe_unused]] const std::string &config = "") {
      if (!connected_) {
        throw std::runtime_error("Not connected to server");
      }

      // TODO: Implement actual edit-config request
      return "edit-config response placeholder";
    }

  private:
    struct nc_session *session_;
    bool connected_;
  };

  // Global NETCONF client instance
  static NetconfClient g_netconfClient;

  // Public interface functions
  bool connectToServer(const std::string &socketPath) {
    return g_netconfClient.connect(socketPath);
  }

  void disconnectFromServer() { g_netconfClient.disconnect(); }

  bool isConnectedToServer() { return g_netconfClient.isConnected(); }

  std::string sendNetconfRequest(const std::string &request) {
    return g_netconfClient.sendRequest(request);
  }

  std::string getConfig(const std::string &source) {
    return g_netconfClient.getConfig(source);
  }

  std::string get(const std::string &filter) {
    return g_netconfClient.get(filter);
  }

  std::string editConfig(const std::string &target, const std::string &config) {
    return g_netconfClient.editConfig(target, config);
  }

} // namespace netd::client
