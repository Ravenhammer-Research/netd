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
#include <libyang/tree_data.h>
#include <libyang/tree_schema.h>
#include <server/include/netconf/handlers.hpp>
#include <server/include/store/candidate.hpp>
#include <server/include/store/running.hpp>
#include <server/include/store/startup.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/marshalling/interface.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/response/get/config.hpp>

namespace netd::server::netconf::handlers {

  std::unique_ptr<netd::shared::response::get::GetConfigResponse>
  RpcHandler::handleGetInterfaceRequest(
      std::unique_ptr<netd::shared::request::get::GetConfigRequest> request) {
    try {
      // Create response object
      auto response =
          std::make_unique<netd::shared::response::get::GetConfigResponse>();

      // Create interface data container
      auto interfaceData =
          std::make_unique<netd::shared::marshalling::Interface>();

      // Get the ietf-interfaces module from the request's session context
      auto session = request->getSession();
      auto ctx = nc_session_get_ctx(session);
      const struct lys_module *ietfInterfaces =
          ly_ctx_get_module(ctx, "ietf-interfaces", nullptr);
      if (!ietfInterfaces) {
        throw std::runtime_error("ietf-interfaces module not found");
      }

      // Create interfaces container
      lyd_node *interfacesContainer = nullptr;
      if (lyd_new_inner(nullptr, ietfInterfaces, "interfaces", 0,
                        &interfacesContainer) != LY_SUCCESS) {
        throw std::runtime_error("Failed to create interfaces container");
      }

      // Determine the source from the request and get the appropriate store
      auto source = request->getSource();
      auto &logger = netd::shared::Logger::getInstance();

      std::vector<lyd_node *> interfaceNodes;

      switch (source) {
      case netd::shared::request::get::Datastore::RUNNING: {
        logger.info(
            "Retrieving interface configuration from running datastore");
        auto &runningStore =
            netd::server::store::running::RunningStore::getInstance();
        interfaceNodes = runningStore.searchInterface();
        break;
      }
      case netd::shared::request::get::Datastore::CANDIDATE: {
        logger.info(
            "Retrieving interface configuration from candidate datastore");
        auto &candidateStore =
            netd::server::store::candidate::CandidateStore::getInstance();
        interfaceNodes = candidateStore.searchInterface();
        break;
      }
      case netd::shared::request::get::Datastore::STARTUP: {
        logger.info(
            "Retrieving interface configuration from startup datastore");
        auto &startupStore =
            netd::server::store::startup::StartupStore::getInstance();
        interfaceNodes = startupStore.searchInterface();
        break;
      }
      default: {
        throw std::runtime_error("Unknown datastore source");
      }
      }

      // Add each interface node to the container
      for (auto *interfaceNode : interfaceNodes) {
        if (interfaceNode) {
          // Clone the interface node to avoid ownership issues
          lyd_node *clonedNode = nullptr;
          if (lyd_dup_single(interfaceNode, nullptr, LYD_DUP_RECURSIVE,
                             &clonedNode) == LY_SUCCESS) {
            lyd_insert_child(interfacesContainer, clonedNode);
          }
        }
      }

      // Set the complete interfaces tree in the data container
      interfaceData->setData(interfacesContainer);

      // Set the interface data in the response
      response->setData(std::move(interfaceData));

      return response;

    } catch (const std::exception &e) {
      // Return a response with error information
      auto errorResponse =
          std::make_unique<netd::shared::response::get::GetConfigResponse>();
      // TODO: Set error information in the response
      return errorResponse;
    }
  }

} // namespace netd::server::netconf::handlers