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

#include <shared/include/response.hpp>
#include <shared/include/yang.hpp>
#include <libyang/tree_data.h>

namespace netd {

Response::Response(const std::string& messageId, bool success) 
    : messageId_(messageId), data_(""), success_(success) {
}

Response::Response(const std::string& messageId, const std::string& data, bool success) 
    : messageId_(messageId), data_(data), success_(success) {
}

lyd_node* Response::toYang(ly_ctx* ctx) const {
    
    if (!ctx) {
        return nullptr;
    }
    
    // Create NETCONF RPC reply node using standard NETCONF schema
    lyd_node* rpcReplyNode = nullptr;
    
    // Create the RPC reply container
    if (lyd_new_path(nullptr, ctx, "/nc:rpc-reply", nullptr, 0, &rpcReplyNode) != LY_SUCCESS) {
        return nullptr;
    }
    
    // Set message ID
    lyd_node* messageIdNode = nullptr;
    if (lyd_new_path(rpcReplyNode, ctx, "/nc:rpc-reply/message-id", messageId_.c_str(), 0, &messageIdNode) != LY_SUCCESS) {
        lyd_free_tree(rpcReplyNode);
        return nullptr;
    }
    
    if (success_) {
        if (!data_.empty()) {
            // Add data node for successful responses with data
            lyd_node* dataNode = nullptr;
            if (lyd_new_path(rpcReplyNode, ctx, "/nc:rpc-reply/data", nullptr, 0, &dataNode) != LY_SUCCESS) {
                lyd_free_tree(rpcReplyNode);
                return nullptr;
            }
        } else {
            // Add ok node for successful responses without data
            lyd_node* okNode = nullptr;
            if (lyd_new_path(rpcReplyNode, ctx, "/nc:rpc-reply/ok", nullptr, 0, &okNode) != LY_SUCCESS) {
                lyd_free_tree(rpcReplyNode);
                return nullptr;
            }
        }
    } else {
        // Add rpc-error node for failed responses
        lyd_node* errorNode = nullptr;
        if (lyd_new_path(rpcReplyNode, ctx, "/nc:rpc-reply/rpc-error", nullptr, 0, &errorNode) != LY_SUCCESS) {
            lyd_free_tree(rpcReplyNode);
            return nullptr;
        }
    }
    
    return rpcReplyNode;
}

Response Response::fromYang(const ly_ctx* ctx, const lyd_node* node) {
    // Parse NETCONF RPC reply node to extract response information
    std::string messageId = "";
    std::string data = "";
    bool success = true;
    
    // Find the message ID and response content
    lyd_node* child = lyd_child(node);
    while (child) {
        if (strcmp(child->schema->name, "message-id") == 0) {
            messageId = lyd_get_value(child);
        } else if (strcmp(child->schema->name, "data") == 0) {
            // Extract data content
            data = lyd_get_value(child);
        } else if (strcmp(child->schema->name, "ok") == 0) {
            // Success response without data
            success = true;
        } else if (strcmp(child->schema->name, "rpc-error") == 0) {
            // Error response
            success = false;
        }
        child = child->next;
    }
    
    return Response(messageId, data, success);
}

} // namespace netd
