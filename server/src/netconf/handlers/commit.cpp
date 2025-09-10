/*
 * Copyright (c) 2024
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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

#include <iostream>
#include <libnetconf2/netconf.h>
#include <libyang/libyang.h>
#include <server/include/netconf/handlers.hpp>
#include <server/include/store/candidate.hpp>
#include <server/include/store/running.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/request/commit.hpp>
#include <shared/include/response/commit.hpp>

namespace netd::server::netconf::handlers {

  std::unique_ptr<netd::shared::response::CommitResponse>
  RpcHandler::handleCommitRequest(
      std::unique_ptr<netd::shared::request::CommitRequest> /* request */) {
    auto &logger = netd::shared::Logger::getInstance();
    auto response =
        std::make_unique<netd::shared::response::CommitResponse>();

    logger.info("Handling commit request");

    // Get the candidate and running stores
    auto &candidateStore =
        netd::server::store::candidate::CandidateStore::getInstance();
    auto &runningStore =
        netd::server::store::running::RunningStore::getInstance();

    // Get the candidate configuration
    auto candidateData = candidateStore.getDataTree();
    if (!candidateData) {
      response->error = std::make_unique<netd::shared::marshalling::Error>(
          netd::shared::marshalling::ErrorType::PROTOCOL,
          netd::shared::marshalling::ErrorTag::OPERATION_FAILED,
          netd::shared::marshalling::ErrorSeverity::ERROR);
      response->error->setMessage("No candidate configuration to commit");
      return response;
    }

    // Clone the candidate data for the running store
    lyd_node *runningData = nullptr;
    LY_ERR err = lyd_dup_single(candidateData, nullptr, LYD_DUP_RECURSIVE,
                                &runningData);
    if (err != LY_SUCCESS || !runningData) {
      response->error = std::make_unique<netd::shared::marshalling::Error>(
          netd::shared::marshalling::ErrorType::PROTOCOL,
          netd::shared::marshalling::ErrorTag::OPERATION_FAILED,
          netd::shared::marshalling::ErrorSeverity::ERROR);
      response->error->setMessage("Failed to clone candidate configuration");
      return response;
    }

    // Set the running configuration
    runningStore.setDataTree(runningData);

    logger.info("Successfully committed candidate configuration to running");
    return response;
  }

} // namespace netd::server::netconf::handlers
