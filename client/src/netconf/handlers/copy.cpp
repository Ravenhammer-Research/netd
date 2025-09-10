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

  std::unique_ptr<netd::shared::response::CopyConfigResponse> RpcHandler::handleCopyConfigRequest(
      const netd::shared::request::CopyConfigRequest& /* request */, 
      netd::shared::netconf::NetconfSession* session) {
    if (!session) {
      return nullptr;
    }

    // Get source and target from request
    std::string source = "running"; // TODO: Get from request
    std::string target = "candidate"; // TODO: Get from request

    ly_ctx* ctx = session->getContext();
    if (!ctx) {
      return nullptr;
    }

      // Create copy-config request
      lyd_node* copy_tree = nullptr;
      LY_ERR err = lyd_new_path(nullptr, ctx, "/ietf-netconf:copy-config", nullptr, 0, &copy_tree);
      if (err != LY_SUCCESS || !copy_tree) {
        return nullptr;
      }
      if (!copy_tree) {
        return nullptr;
      }

      // Add source and target
      lyd_node* source_node = nullptr;
      err = lyd_new_path(copy_tree, ctx, "source", nullptr, 0, &source_node);
      if (source_node) {
        lyd_new_path(source_node, ctx, source.c_str(), nullptr, 0, nullptr);
      }

      lyd_node* target_node = nullptr;
      err = lyd_new_path(copy_tree, ctx, "target", nullptr, 0, &target_node);
      if (target_node) {
        lyd_new_path(target_node, ctx, target.c_str(), nullptr, 0, nullptr);
      }

      // Convert to XML
      char* xml_str = nullptr;
      if (lyd_print_mem(&xml_str, copy_tree, LYD_XML, 0) != LY_SUCCESS || !xml_str) {
        lyd_free_tree(copy_tree);
        return nullptr;
      }

      std::string response_xml(xml_str);
      free(xml_str);
      lyd_free_tree(copy_tree);

      // Create response object
      auto response = std::make_unique<netd::shared::response::CopyConfigResponse>();
      // TODO: Set response data from response_xml
      return response;

  }

} // namespace netd::client::netconf
