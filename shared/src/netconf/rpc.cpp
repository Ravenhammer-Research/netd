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

#include <shared/include/netconf/rpc.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/exception.hpp>
#include <libyang/libyang.h>
#include <libyang/tree_data.h>
#include <libyang/parser_data.h>
#include <libyang/in.h>

namespace netd::shared::netconf {

  RpcData::RpcData(lyd_node* node) : node_(node) {
  }

  RpcData::~RpcData() {
    if (node_) {
      lyd_free_tree(node_);
    }
  }

  std::string RpcData::toString() const {
    if (!node_) {
      return "";
    }

    char* xml_str = nullptr;
    if (lyd_print_mem(&xml_str, node_, LYD_XML, 0) == LY_SUCCESS && xml_str) {
      std::string result(xml_str);
      free(xml_str);
      return result;
    }
    
    return "";
  }

  std::unique_ptr<RpcData> Rpc::createRpcRequest(ly_ctx* ctx, 
                                                const std::string& message_id,
                                                const std::string& operation_name,
                                                lyd_node* child_data) {
    if (!ctx) {
      throw netd::shared::ArgumentError("YANG context is null");
    }

    if (operation_name.empty()) {
      throw netd::shared::ArgumentError("Operation name cannot be empty");
    }

    auto &logger = netd::shared::Logger::getInstance();
    logger.debug("Creating RPC request: " + operation_name + " (message-id: " + message_id + ")");

    // Create the RPC envelope
    lyd_node* rpc_node = nullptr;
    if (lyd_new_path(nullptr, ctx, "/ietf-netconf:rpc", nullptr, 0, &rpc_node) != LY_SUCCESS) {
      throw netd::shared::RpcException("Failed to create RPC envelope node");
    }

    // Set message-id attribute
    if (!message_id.empty()) {
      lyd_new_path(rpc_node, ctx, "message-id", message_id.c_str(), 0, nullptr);
    }

    // Create the operation node
    lyd_node* op_node = nullptr;
    if (lyd_new_path(rpc_node, ctx, operation_name.c_str(), nullptr, 0, &op_node) != LY_SUCCESS) {
      lyd_free_tree(rpc_node);
      throw netd::shared::RpcException("Failed to create operation node: " + operation_name);
    }

    // Attach child data if provided
    if (child_data) {
      if (lyd_insert_child(op_node, child_data) != LY_SUCCESS) {
        lyd_free_tree(rpc_node);
        throw netd::shared::RpcException("Failed to attach child data to operation node");
      }
    }

    return std::make_unique<RpcData>(rpc_node);
  }

  std::unique_ptr<RpcData> Rpc::createRpcReply(ly_ctx* ctx,
                                              const std::string& message_id,
                                              lyd_node* child_data) {
    if (!ctx) {
      throw netd::shared::ArgumentError("YANG context is null");
    }

    auto &logger = netd::shared::Logger::getInstance();
    logger.debug("Creating RPC reply (message-id: " + message_id + ")");

    // Create the RPC reply envelope
    lyd_node* reply_node = nullptr;
    if (lyd_new_path(nullptr, ctx, "/ietf-netconf:rpc-reply", nullptr, 0, &reply_node) != LY_SUCCESS) {
      throw netd::shared::RpcException("Failed to create RPC reply envelope node");
    }

    // Set message-id attribute
    if (!message_id.empty()) {
      lyd_new_path(reply_node, ctx, "message-id", message_id.c_str(), 0, nullptr);
    }

    // Attach child data if provided
    if (child_data) {
      if (lyd_insert_child(reply_node, child_data) != LY_SUCCESS) {
        lyd_free_tree(reply_node);
        throw netd::shared::RpcException("Failed to attach child data to reply node");
      }
    }

    return std::make_unique<RpcData>(reply_node);
  }

  lyd_node* Rpc::extractFromEnvelope(const std::string& rpc_xml) {
    // Parse the RPC XML to extract the data (handles both <rpc> and <rpc-reply>)
    struct lyd_node* tree = nullptr;
    struct ly_in* in = nullptr;
    if (ly_in_new_memory(rpc_xml.c_str(), &in) != LY_SUCCESS) {
      return nullptr;
    }
    if (!in) {
      return nullptr;
    }

    // Parse as RPC operation
    if (lyd_parse_op(nullptr, nullptr, in, LYD_XML, LYD_TYPE_RPC_YANG, &tree, nullptr) != LY_SUCCESS) {
      ly_in_free(in, 0);
      return nullptr;
    }
    ly_in_free(in, 0);

    if (!tree) {
      return nullptr;
    }

    // Find the first child node (the actual request/reply data)
    struct lyd_node* child = lyd_child(tree);
    if (!child) {
      lyd_free_tree(tree);
      return nullptr;
    }

    // Detach the child node from the tree and return it
    // The caller is responsible for freeing the returned node
    lyd_unlink_tree(child);
    lyd_free_tree(tree);

    return child;
  }

} // namespace netd::shared::netconf
