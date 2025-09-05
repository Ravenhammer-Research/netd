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

#include <libnetconf2/messages_server.h>
#include <libnetconf2/netconf.h>
#include <libnetconf2/server_config.h>
#include <libnetconf2/session_server.h>
#include <libyang/libyang.h>
#include <server/include/netconf/base.hpp>
#include <shared/include/exception.hpp>

namespace netd::server::netconf {

  Server::Server() : running_(false), ps_(nullptr) {}

  Server::~Server() {
    if (ps_) {
      nc_ps_clear(ps_, 1, nullptr);
      nc_ps_free(ps_);
    }
  }

  bool Server::isRunning() const { return running_; }

  struct nc_session *Server::acceptSession() {
    struct nc_session *newSession = nullptr;
    NC_MSG_TYPE msgType = nc_accept(
        0, netd::shared::Yang::getInstance().getContext(), &newSession);

    switch (msgType) {
    case NC_MSG_HELLO:
      return newSession;
    case NC_MSG_WOULDBLOCK:
    case NC_MSG_BAD_HELLO:
    case NC_MSG_ERROR:
    default:
      if (newSession) {
        nc_session_free(newSession, nullptr);
      }
      return nullptr;
    }
  }

  void Server::closeSession(struct nc_session *session) {
    if (session) {
      nc_session_free(session, nullptr);
    }
  }

  struct nc_server_reply *Server::handleRpc(struct nc_session *session,
                                            struct lyd_node *rpc) {
    if (!session || !rpc) {
      return nullptr;
    }

    // Get the RPC operation name
    const char *rpcName = LYD_NAME(rpc);
    if (!rpcName) {
      return nc_server_reply_err(
          nc_err(netd::shared::Yang::getInstance().getContext(),
                 NC_ERR_OP_FAILED, NC_ERR_TYPE_APP));
    }

    // Dispatch to appropriate handler based on RPC name
    if (strcmp(rpcName, "get") == 0) {
      return handleGetRequest(session, rpc);
    } else if (strcmp(rpcName, "get-config") == 0) {
      return handleGetConfigRequest(session, rpc);
    } else if (strcmp(rpcName, "edit-config") == 0) {
      return handleEditConfigRequest(session, rpc);
    } else if (strcmp(rpcName, "copy-config") == 0) {
      return handleCopyConfigRequest(session, rpc);
    } else if (strcmp(rpcName, "delete-config") == 0) {
      return handleDeleteConfigRequest(session, rpc);
    } else if (strcmp(rpcName, "lock") == 0) {
      return handleLockRequest(session, rpc);
    } else if (strcmp(rpcName, "unlock") == 0) {
      return handleUnlockRequest(session, rpc);
    } else if (strcmp(rpcName, "discard-changes") == 0) {
      return handleDiscardRequest(session, rpc);
    } else {
      // Unknown RPC
      return nc_server_reply_err(
          nc_err(netd::shared::Yang::getInstance().getContext(),
                 NC_ERR_OP_NOT_SUPPORTED, NC_ERR_TYPE_APP));
    }
  }

  struct nc_server_reply *Server::handleGetRequest(struct nc_session *session,
                                                   struct lyd_node *rpc) {
    throw netd::shared::NotImplementedError(
        "handleGetRequest not implemented in base Server class");
  }

  struct nc_server_reply *
  Server::handleGetConfigRequest(struct nc_session *session,
                                 struct lyd_node *rpc) {
    throw netd::shared::NotImplementedError(
        "handleGetConfigRequest not implemented in base Server class");
  }

  struct nc_server_reply *
  Server::handleEditConfigRequest(struct nc_session *session,
                                  struct lyd_node *rpc) {
    throw netd::shared::NotImplementedError(
        "handleEditConfigRequest not implemented in base Server class");
  }

  struct nc_server_reply *
  Server::handleCopyConfigRequest(struct nc_session *session,
                                  struct lyd_node *rpc) {
    throw netd::shared::NotImplementedError(
        "handleCopyConfigRequest not implemented in base Server class");
  }

  struct nc_server_reply *
  Server::handleDeleteConfigRequest(struct nc_session *session,
                                    struct lyd_node *rpc) {
    throw netd::shared::NotImplementedError(
        "handleDeleteConfigRequest not implemented in base Server class");
  }

  struct nc_server_reply *Server::handleLockRequest(struct nc_session *session,
                                                    struct lyd_node *rpc) {
    throw netd::shared::NotImplementedError(
        "handleLockRequest not implemented in base Server class");
  }

  struct nc_server_reply *
  Server::handleUnlockRequest(struct nc_session *session,
                              struct lyd_node *rpc) {
    throw netd::shared::NotImplementedError(
        "handleUnlockRequest not implemented in base Server class");
  }

  struct nc_server_reply *
  Server::handleDiscardRequest(struct nc_session *session,
                               struct lyd_node *rpc) {
    throw netd::shared::NotImplementedError(
        "handleDiscardRequest not implemented in base Server class");
  }

} // namespace netd::server::netconf
