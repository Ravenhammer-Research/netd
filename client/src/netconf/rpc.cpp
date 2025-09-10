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

#include <client/include/netconf/rpc.hpp>
#include <client/include/netconf/handlers.hpp>
#include <shared/include/exception.hpp>
#include <libyang/libyang.h>
#include <libyang/in.h>
#include <sstream>

namespace netd::client::netconf {

  NetconfRpc::NetconfRpc() {
  }

  NetconfRpc::~NetconfRpc() {
  }

  std::string NetconfRpc::processRpc(const std::string& xml_request, 
                                    netd::shared::netconf::NetconfSession* session) {
    if (!session) {
      throw netd::shared::ArgumentError("Session is null");
    }

    ly_ctx* ctx = session->getContext();
    if (!ctx) {
      throw netd::shared::RpcException("No YANG context available in session");
    }

    // Parse the incoming XML
    struct lyd_node* tree = nullptr;
    struct ly_in* in = nullptr;
    if (ly_in_new_memory(xml_request.c_str(), &in) != LY_SUCCESS) {
      throw netd::shared::RpcException("Failed to create input stream from XML data");
    }

    if (lyd_parse_op(ctx, nullptr, in, LYD_XML, LYD_TYPE_RPC_YANG, &tree, nullptr) != LY_SUCCESS) {
      ly_in_free(in, 0);
      throw netd::shared::RpcException("Failed to parse XML request");
    }
    ly_in_free(in, 0);

    if (!tree) {
      throw netd::shared::RpcException("No data tree created from XML request");
    }

    // Extract operation name
    const char* op_name = nullptr;
    if (lyd_find_path(tree, "/*", 0, &tree) == LY_SUCCESS && tree) {
      op_name = LYD_NAME(tree);
    }

    if (!op_name) {
      lyd_free_tree(tree);
      throw netd::shared::RpcException("Could not determine RPC operation name");
    }

    // Handle hello message from server
    if (strcmp(op_name, "hello") == 0) {
      // Create HelloRequest object from the parsed YANG tree
      auto hello_request = std::make_unique<netd::shared::request::HelloRequest>();
      auto parsed_request = hello_request->fromYang(session->getContext(), tree);
      if (!parsed_request) {
        lyd_free_tree(tree);
        throw netd::shared::RpcException("Failed to create HelloRequest from YANG tree");
      }
      
      // Call the client's hello handler
      auto response = RpcHandler::handleHelloRequest(*hello_request, session);
      
      lyd_free_tree(tree);
      
      // Convert response to XML if we have one
      if (response) {
        lyd_node* response_tree = response->toYang(ctx);
        if (response_tree) {
          char* xml_str = nullptr;
          if (lyd_print_mem(&xml_str, response_tree, LYD_XML, 0) == LY_SUCCESS && xml_str) {
            std::string result(xml_str);
            free(xml_str);
            lyd_free_tree(response_tree);
            return result;
          }
          lyd_free_tree(response_tree);
        }
      }
      
      return ""; // Empty response if no response generated
    }

    // For other operations, we'd need to create the appropriate request objects
    // and call their respective handlers
    lyd_free_tree(tree);
    throw netd::shared::RpcException("Unsupported RPC operation: " + std::string(op_name));
  }

} // namespace netd::client::netconf
