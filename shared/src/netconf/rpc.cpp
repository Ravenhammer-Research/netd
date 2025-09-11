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
#include <shared/include/exception.hpp>
#include <shared/include/yang.hpp>
#include <libyang/libyang.h>
#include <sstream>

namespace netd::shared::netconf {


  std::istream* Rpc::processRpc(std::istream& rpc_stream, NetconfSession* session) {
    if (!session) {
      throw netd::shared::ArgumentError("session cannot be null");
    }
    if (!rpc_stream.good()) {
      throw netd::shared::ArgumentError("invalid input stream");
    }
    ly_ctx *ctx = session->getContext();
    
    std::stringstream buffer;
    buffer << rpc_stream.rdbuf();
    std::string xml_data = buffer.str();
    
    struct ly_in *in = nullptr;
    if (ly_in_new_memory(xml_data.c_str(), &in) != LY_SUCCESS) {
      throw netd::shared::YangParseError(ctx);
    }
    
    lyd_node *data = nullptr;
    LY_ERR ret = lyd_parse_data(ctx, nullptr, in, LYD_XML, LYD_PARSE_OPAQ, 0, &data);
    ly_in_free(in, 0);
    
    if (ret != LY_SUCCESS) {
      throw netd::shared::YangParseError(ctx);
    }
    
    struct lyd_node *msg_id_node = nullptr;
    if (lyd_find_path(data, "message-id", 0, &msg_id_node) != LY_SUCCESS || 
        !msg_id_node || msg_id_node->schema->nodetype != LYS_LEAF) {
      lyd_free_tree(data);
      throw netd::shared::YangValidationError(ctx);
    }
    std::string message_id_str = lyd_get_value(msg_id_node);
    int message_id = std::stoi(message_id_str);
    
    if (!data || !lyd_child(data)) {
      lyd_free_tree(data);
      throw netd::shared::YangValidationError(ctx);
    }
    std::string operation_name = lyd_child(data)->schema->name;
    
    std::string root_name = data->schema->name;
    if (root_name == "rpc") {
      NetconfOperation operation = stringToOperation(operation_name);
      return processRequest(data, message_id, operation, session);
    } else if (root_name == "rpc-reply") {
      return processReply(data, message_id, session);
    } else {
      lyd_free_tree(data);
      throw netd::shared::YangDataError(ctx);
    }
  }

  std::istream* Rpc::processRequest(lyd_node* data, int message_id, NetconfOperation operation, NetconfSession* session) {
    (void)data;
    (void)message_id;
    (void)operation;
    (void)session;
    throw netd::shared::NotImplementedError("processRequest not implemented");
  }

  std::istream* Rpc::processReply(lyd_node* data, int message_id, NetconfSession* session) {
    (void)data;
    (void)message_id;
    (void)session;
    throw netd::shared::NotImplementedError("processReply not implemented");
  }

  std::istream* Rpc::createRpcRequest(NetconfSession* session,
                                     NetconfOperation operation,
                                     lyd_node* child_data) {
    if (!session) {
      throw netd::shared::ArgumentError("session cannot be null");
    }

    const char* operation_name = operationToString(operation);
    if (!operation_name) {
      throw netd::shared::ArgumentError("invalid operation name");
    }

    ly_ctx* ctx = session->getContext();
    int message_id = session->getNextMessageId();
    std::string message_id_str = std::to_string(message_id);

    lyd_node* rpc_node = nullptr;
    if (lyd_new_opaq2(nullptr, ctx, "rpc", nullptr, nullptr, "urn:ietf:params:xml:ns:netconf:base:1.0", &rpc_node) != LY_SUCCESS) {
      throw netd::shared::YangDataError(ctx);
    }
    
    if (lyd_new_meta(ctx, rpc_node, nullptr, "message-id", message_id_str.c_str(), 0, nullptr) != LY_SUCCESS) {
      lyd_free_tree(rpc_node);
      throw netd::shared::YangDataError(ctx);
    }

    lyd_node* op_node = nullptr;
    if (lyd_new_opaq2(rpc_node, nullptr, operation_name, nullptr, nullptr, "urn:ietf:params:xml:ns:netconf:base:1.0", &op_node) != LY_SUCCESS) {
      lyd_free_tree(rpc_node);
      throw netd::shared::YangDataError(ctx);
    }

    if (child_data) {
      if (lyd_insert_child(op_node, child_data) != LY_SUCCESS) {
        lyd_free_tree(rpc_node);
        throw netd::shared::YangDataError(ctx);
      }
    }

    char* xml_str = nullptr;
    if (lyd_print_mem(&xml_str, rpc_node, LYD_XML, 0) == LY_SUCCESS && xml_str) {
      std::string xml_data(xml_str);
      free(xml_str);
      lyd_free_tree(rpc_node);
      return new std::istringstream(xml_data);
    }
    
    lyd_free_tree(rpc_node);
    throw netd::shared::YangDataError(ctx);
  }

  std::istream* Rpc::createRpcReply(NetconfSession* session,
                                     lyd_node* child_data) {
    if (!session) {
      throw netd::shared::ArgumentError("session cannot be null");
    }

    ly_ctx* ctx = session->getContext();
    int message_id = session->getNextMessageId();
    std::string message_id_str = std::to_string(message_id);

    lyd_node* reply_node = nullptr;
    if (lyd_new_opaq2(nullptr, ctx, "rpc-reply", nullptr, nullptr, "urn:ietf:params:xml:ns:netconf:base:1.0", &reply_node) != LY_SUCCESS) {
      throw netd::shared::YangDataError(ctx);
    }
    
    if (lyd_new_meta(ctx, reply_node, nullptr, "message-id", message_id_str.c_str(), 0, nullptr) != LY_SUCCESS) {
      lyd_free_tree(reply_node);
      throw netd::shared::YangDataError(ctx);
    }

    if (child_data) {
      if (lyd_insert_child(reply_node, child_data) != LY_SUCCESS) {
        lyd_free_tree(reply_node);
        throw netd::shared::YangDataError(ctx);
      }
    }

    char* xml_str = nullptr;
    if (lyd_print_mem(&xml_str, reply_node, LYD_XML, 0) == LY_SUCCESS && xml_str) {
      std::string xml_data(xml_str);
      free(xml_str);
      lyd_free_tree(reply_node);
      return new std::istringstream(xml_data);
    }
    
    lyd_free_tree(reply_node);
    throw netd::shared::YangDataError(ctx);
  }

} // namespace netd::shared::netconf