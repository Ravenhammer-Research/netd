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

#include <client/include/netconf/reply.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/netconf/rpc.hpp>
#include <libyang/libyang.h>
#include <libyang/in.h>
#include <sstream>

namespace netd::client::netconf {

  NetconfReply::NetconfReply() {
  }

  NetconfReply::~NetconfReply() {
  }

  std::string NetconfReply::processReply(const std::string& xml_reply, 
                                        netd::shared::netconf::NetconfSession* session) {
    if (!session) {
      throw netd::shared::ArgumentError("Session is null");
    }

    ly_ctx* ctx = session->getContext();
    if (!ctx) {
      throw netd::shared::RpcException("No YANG context available in session");
    }

    // Parse the incoming XML reply
    struct lyd_node* tree = nullptr;
    struct ly_in* in = nullptr;
    if (ly_in_new_memory(xml_reply.c_str(), &in) != LY_SUCCESS) {
      throw netd::shared::RpcException("Failed to create input stream from XML reply");
    }

    if (lyd_parse_op(ctx, nullptr, in, LYD_XML, LYD_TYPE_RPC_YANG, &tree, nullptr) != LY_SUCCESS) {
      ly_in_free(in, 0);
      throw netd::shared::RpcException("Failed to parse XML reply");
    }
    ly_in_free(in, 0);

    if (!tree) {
      throw netd::shared::RpcException("No data tree created from XML reply");
    }

    // Extract operation name to determine reply type
    const char* op_name = nullptr;
    if (lyd_find_path(tree, "/*", 0, &tree) == LY_SUCCESS && tree) {
      op_name = LYD_NAME(tree);
    }

    if (!op_name) {
      lyd_free_tree(tree);
      throw netd::shared::RpcException("Could not determine reply operation name");
    }

    // Handle different reply types
    if (strcmp(op_name, "rpc-reply") == 0) {
      // This is a standard RPC reply - extract the actual response data
      lyd_free_tree(tree);
      lyd_node* reply_data = netd::shared::netconf::Rpc::extractFromEnvelope(xml_reply);
      if (reply_data) {
        char* xml_str = nullptr;
        if (lyd_print_mem(&xml_str, reply_data, LYD_XML, 0) == LY_SUCCESS && xml_str) {
          std::string result(xml_str);
          free(xml_str);
          lyd_free_tree(reply_data);
          return result;
        }
        lyd_free_tree(reply_data);
      }
      return "";
    } else if (strcmp(op_name, "hello") == 0) {
      // This is a hello reply from server
      lyd_free_tree(tree);
      return processHelloReply(xml_reply);
    } else {
      // Unknown reply type
      lyd_free_tree(tree);
      throw netd::shared::RpcException("Unknown reply type: " + std::string(op_name));
    }
  }


  std::string NetconfReply::processHelloReply(const std::string& /* hello_xml */) {
    // For hello replies, we just acknowledge receipt
    // In a real implementation, we might extract capabilities, session-id, etc.
    return ""; // Empty response for hello acknowledgment
  }

} // namespace netd::client::netconf
