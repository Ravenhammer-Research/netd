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
#include <shared/include/logger.hpp>
#include <shared/include/marshalling/interface.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/response/get/config.hpp>

namespace netd::server::netconf::handlers {

  struct nc_server_reply *
  RpcHandler::handleGetConfigRequest(struct nc_session *session,
                                     struct lyd_node *rpc) {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Handling get-config request");

    try {
      // Parse the request using GetConfigRequest
      auto request =
          std::make_unique<netd::shared::request::get::GetConfigRequest>();
      auto parsedRequest = request->fromYang(nc_session_get_ctx(session), rpc);

      // Create response object
      auto response =
          std::make_unique<netd::shared::response::get::GetConfigResponse>();

      // Get all network interfaces from the system using interface handler
      std::vector<std::string> interfaceNames = getAllInterfaceNames();
      logger.info("Found " + std::to_string(interfaceNames.size()) +
                  " interfaces");

      // Create interface data container
      auto interfaceData =
          std::make_unique<netd::shared::marshalling::Interface>();

      // Process each interface using interface handler functions
      for (const auto &ifName : interfaceNames) {
        logger.info("Processing interface: " + ifName);

        // Try to determine interface type and create appropriate handler
        auto interfaceInfo = getInterfaceInfo(ifName);
        if (interfaceInfo) {
          // TODO: Convert interface-specific data to YANG format
          // This is where we would convert each interface type to its YANG
          // representation
          logger.info("Interface " + ifName +
                      " type: " + interfaceInfo->getType());
        }
      }

      // Set the interface data in the response
      response->setData(std::move(interfaceData));

      // Convert response to YANG and return
      auto yangResponse = response->toYang(nc_session_get_ctx(session));
      if (yangResponse) {
        return nc_server_reply_data(yangResponse);
      } else {
        logger.error("Failed to convert response to YANG");
        return nc_server_reply_err(
            nc_err(NC_ERR_OP_FAILED, "Failed to generate response"));
      }

    } catch (const std::exception &e) {
      logger.error("Exception in get-config handler: " + std::string(e.what()));
      return nc_server_reply_err(nc_err(NC_ERR_OP_FAILED, e.what()));
    }
  }

} // namespace netd::server::netconf::handlers
