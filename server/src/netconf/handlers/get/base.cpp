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
#include <shared/include/logger.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/request/get/yanglib.hpp>
#include <shared/include/response/get/yanglib.hpp>
#include <shared/include/yang.hpp>

namespace netd::server::netconf::handlers {
  using netd::shared::NotImplementedError;

  std::unique_ptr<netd::shared::response::get::GetResponse>
  RpcHandler::handleGetRequest(
      std::unique_ptr<netd::shared::request::get::GetRequest> request) {
    
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("handleGetRequest: Processing get request");
    logger.info("hasFilter: " + std::string(request->hasFilter() ? "true" : "false"));
    if (request->hasFilter()) {
      logger.info("filterType: " + request->getFilterType());
      logger.info("filterSelect: " + request->getFilterSelect());
    }
    
    // Check if this is a YANG library request by examining the filter
    // The client sends: <filter type="xpath" select="/yanglib:*"/>
    // libyang expands yanglib: to ietf-yang-library:
    if (request->hasFilter() && 
        request->getFilterType() == "xpath" && 
        (request->getFilterSelect().find("yanglib:") != std::string::npos ||
         request->getFilterSelect().find("ietf-yang-library:") != std::string::npos)) {
      
      logger.info("handleGetRequest: Detected YANG library request, processing...");
      
      // Create a YANG library response
      auto yanglibResponse = std::make_unique<netd::shared::response::get::GetYanglibResponse>();
      
      // Get the YANG context to generate YANG library data
      auto &yang = netd::shared::Yang::getInstance();
      ly_ctx *ctx = yang.getContext();
      
      if (ctx) {
        // Use libyang's built-in function to generate YANG library data
        struct lyd_node *yanglibData = nullptr;
        // Temporarily disable ly_ctx_get_yanglib_data to use fallback method
        if (false && ly_ctx_get_yanglib_data(ctx, &yanglibData, "ietf-yang-library@2019-01-04") == LY_SUCCESS && yanglibData) {
          // The yanglibData contains the complete YANG library structure
          // We can use this directly in the response
          yanglibResponse->setYanglibData(yanglibData);
        } else {
          // Fallback: Add some basic YANG modules that this server supports
          yanglibResponse->addModule("ietf-netconf", "2013-09-29", 
                                     "urn:ietf:params:xml:ns:netconf:base:1.0");
          yanglibResponse->addModule("ietf-netconf-with-defaults", "2011-06-01",
                                     "urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults");
          yanglibResponse->addModule("ietf-interfaces", "2018-02-20",
                                     "urn:ietf:params:xml:ns:yang:ietf-interfaces");
          yanglibResponse->addModule("ietf-yang-library", "2019-01-04",
                                     "urn:ietf:params:xml:ns:yang:ietf-yang-library");
        }
      } else {
        // Fallback: Add some basic YANG modules that this server supports
        yanglibResponse->addModule("ietf-netconf", "2013-09-29", 
                                   "urn:ietf:params:xml:ns:netconf:base:1.0");
        yanglibResponse->addModule("ietf-netconf-with-defaults", "2011-06-01",
                                   "urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults");
        yanglibResponse->addModule("ietf-interfaces", "2018-02-20",
                                   "urn:ietf:params:xml:ns:yang:ietf-interfaces");
        yanglibResponse->addModule("ietf-yang-library", "2019-01-04",
                                   "urn:ietf:params:xml:ns:yang:ietf-yang-library");
      }
      
      // Convert to base GetResponse
      return std::move(yanglibResponse);
    }
    
    // For other get requests, throw not implemented
    throw NotImplementedError("handleGetRequest method not implemented for non-yanglib requests");
  }

} // namespace netd::server::netconf::handlers
