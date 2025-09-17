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
#include <libyang/tree_schema.h>
#include <server/include/netconf/handlers.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/request/get/library.hpp>
#include <shared/include/response/get/library.hpp>
#include <shared/include/yang.hpp>

namespace netd::server::netconf::handlers {
  using netd::shared::NotImplementedError;

  std::unique_ptr<netd::shared::response::get::GetResponse>
  RpcHandler::handleGetRequest(
      netd::shared::request::get::GetRequest *request) {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("handleGetRequest: Processing get request");

    // Check if this is a YANG library request by examining the filter
    bool isYanglibRequest = false;

    if (request->hasFilter() && (request->getFilterType() == "subtree" ||
                                 request->getFilterType() == "xpath")) {
      // Check if the filter contains yang-library
      std::string filterSelect = request->getFilterSelect();
      if (filterSelect.find("yang-library") != std::string::npos ||
          filterSelect.find("ietf-yang-library") != std::string::npos) {
        isYanglibRequest = true;
        logger.info("handleGetRequest: Detected yang-library filter in " +
                    request->getFilterType());
      }
    }

    if (isYanglibRequest) {
      logger.info(
          "handleGetRequest: Detected YANG library request, processing...");

      // Create a YANG library response
      auto libraryResponse =
          std::make_unique<netd::shared::response::get::GetLibraryResponse>();

      // Get the YANG context to generate YANG library data
      auto &yang = netd::shared::Yang::getInstance();
      ly_ctx *ctx = yang.getContext();

      if (ctx) {
        // Use libyang's built-in function to generate YANG library data
        struct lyd_node *yanglibData = nullptr;
        // Use the change count as the content-id parameter
        uint32_t changeCount = ly_ctx_get_change_count(ctx);
        if (ly_ctx_get_yanglib_data(ctx, &yanglibData, "%u", changeCount) ==
                LY_SUCCESS &&
            yanglibData) {
          logger.info(
              "handleGetRequest: Generated YANG library data successfully");
          // The yanglibData contains the complete YANG library structure
          // We can use this directly in the response
          libraryResponse->setLibraryData(yanglibData);
        } else {
          logger.error(
              "handleGetRequest: Failed to generate YANG library data");
          throw netd::shared::NotImplementedError(
              "YANG library data generation failed");
        }
      } else {
        throw netd::shared::NotImplementedError("YANG context not available");
      }

      // Convert to base GetResponse
      return std::move(libraryResponse);
    }

    // For other get requests, throw not implemented
    throw NotImplementedError(
        "handleGetRequest method not implemented for non-yanglib requests");
  }

} // namespace netd::server::netconf::handlers
