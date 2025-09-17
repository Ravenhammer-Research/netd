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
#include <libyang/libyang.h>
#include <server/include/netconf/handlers.hpp>
#include <server/include/store/candidate.hpp>
#include <server/include/store/running.hpp>
#include <server/include/store/startup.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/request/edit.hpp>
#include <shared/include/response/edit.hpp>

namespace netd::server::netconf::handlers {

  std::unique_ptr<netd::shared::response::EditConfigResponse>
  RpcHandler::handleEditConfigRequest(
      netd::shared::request::EditConfigRequest *request) {
    auto &logger = netd::shared::Logger::getInstance();
    auto response =
        std::make_unique<netd::shared::response::EditConfigResponse>();

    logger.info("Handling edit-config request");

    // Get the target datastore
    auto target = request->getTarget();

    // Clone the whole RPC request for the store
    lyd_node *rpcData = nullptr;
    LY_ERR err =
        lyd_dup_single(request->getRpc(), nullptr, LYD_DUP_RECURSIVE, &rpcData);
    if (err != LY_SUCCESS || !rpcData) {
      response->setProtocolError(
          netd::shared::marshalling::ErrorTag::OPERATION_FAILED,
          "Failed to clone RPC data");
      return response;
    }

    // Get the appropriate store based on target and write the whole RPC
    switch (target) {
    case netd::shared::request::get::Datastore::RUNNING: {
      auto &runningStore =
          netd::server::store::running::RunningStore::getInstance();
      runningStore.setDataTree(rpcData);
      break;
    }
    case netd::shared::request::get::Datastore::CANDIDATE: {
      auto &candidateStore =
          netd::server::store::candidate::CandidateStore::getInstance();
      candidateStore.setDataTree(rpcData);
      break;
    }
    case netd::shared::request::get::Datastore::STARTUP: {
      lyd_free_tree(rpcData);
      response->setProtocolError(
          netd::shared::marshalling::ErrorTag::OPERATION_FAILED,
          "Startup store is read-only");
      return response;
    }
    default: {
      lyd_free_tree(rpcData);
      response->setProtocolError(
          netd::shared::marshalling::ErrorTag::OPERATION_FAILED,
          "Unknown datastore target");
      return response;
    }
    }

    return response;
  }

} // namespace netd::server::netconf::handlers
