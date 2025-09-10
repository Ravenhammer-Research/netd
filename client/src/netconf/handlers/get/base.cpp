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

#include <client/include/netconf/handlers.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/exception.hpp>
#include <libyang/libyang.h>
#include <sstream>

namespace netd::client::netconf {

  std::unique_ptr<netd::shared::response::get::GetResponse> RpcHandler::handleGetRequest(
      const netd::shared::request::get::GetRequest& /* request */, 
      netd::shared::netconf::NetconfSession* session) {
    if (!session) {
      return nullptr;
    }

    // Get YANG context from session
    ly_ctx* ctx = session->getContext();
    if (!ctx) {
      return nullptr;
    }

    // Create a simple get request for all data
    lyd_node* get_tree = nullptr;
    LY_ERR err = lyd_new_path(nullptr, ctx, "/ietf-netconf:get", nullptr, 0, &get_tree);
    if (err != LY_SUCCESS || !get_tree) {
      return nullptr;
    }

    // Create response object
    auto response = std::make_unique<netd::shared::response::get::GetResponse>();
    
    // Set the data from the get request
    response->setData(get_tree);

    // Convert response to XML and display
    lyd_node* response_tree = response->toYang(ctx);
    if (response_tree) {
      char* xml_str = nullptr;
      if (lyd_print_mem(&xml_str, response_tree, LYD_XML, 0) == LY_SUCCESS && xml_str) {
        // TODO: Process response data
        free(xml_str);
      }
      lyd_free_tree(response_tree);
    }

    // Return the response object for the caller
    return response;
  }

} // namespace netd::client::netconf
