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

  std::unique_ptr<netd::shared::response::CloseResponse> RpcHandler::handleDestroySessionRequest(
      const netd::shared::request::session::DestroyRequest& /* request */, 
      netd::shared::netconf::NetconfSession* session) {
    if (!session) {
      return nullptr;
    }

    // Get session ID from request
    int session_id = 0; // TODO: Get from request
    
    ly_ctx* ctx = session->getContext();
    if (!ctx) {
      return nullptr;
    }

    // Create kill-session request
    lyd_node* kill_tree = nullptr;
    LY_ERR err = lyd_new_path(nullptr, ctx, "/ietf-netconf:kill-session", nullptr, 0, &kill_tree);
    if (err != LY_SUCCESS || !kill_tree) {
      return nullptr;
    }

    // Add session ID
    lyd_new_path(kill_tree, ctx, "session-id", std::to_string(session_id).c_str(), 0, nullptr);

    // Convert to XML
    char* xml_str = nullptr;
    if (lyd_print_mem(&xml_str, kill_tree, LYD_XML, 0) != LY_SUCCESS || !xml_str) {
      lyd_free_tree(kill_tree);
      return nullptr;
    }

    std::string response_xml(xml_str);
    free(xml_str);
    lyd_free_tree(kill_tree);

    // Create response object
    auto response = std::make_unique<netd::shared::response::CloseResponse>();
    // TODO: Set response data from response_xml
    return response;

  }

} // namespace netd::client::netconf
