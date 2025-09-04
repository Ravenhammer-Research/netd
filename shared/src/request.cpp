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

#include <shared/include/request.hpp>
#include <shared/include/yang.hpp>
#include <libyang/tree_data.h>
#include <atomic>

namespace netd {

// Base Request class implementations
Request::Request(const std::string& type, const std::string& data) 
    : type_(type), data_(data), messageId_(generateMessageId()) {
}

Request::Request(const std::string& type, const std::string& data, const std::string& messageId)
    : type_(type), data_(data), messageId_(messageId) {
}

std::string Request::generateMessageId() {
    // Generate incremental message ID
    static std::atomic<uint32_t> messageIdCounter{1};
    return std::to_string(messageIdCounter++);
}

RequestType Request::getRequestType() const {
    if (type_ == "get") {
        return RequestType::GET;
    } else if (type_ == "get-config") {
        return RequestType::GET_CONFIG;
    } else if (type_ == "edit-config") {
        return RequestType::EDIT_CONFIG;
    } else if (type_ == "commit") {
        return RequestType::COMMIT;
    } else {
        return RequestType::UNKNOWN;
    }
}


lyd_node* Request::toYang(ly_ctx* ctx) const {
    
    if (!ctx) {
        return nullptr;
    }
    
    // Create NETCONF RPC node using standard NETCONF schema
    lyd_node* rpcNode = nullptr;
    
    // Create the RPC container
    if (lyd_new_path(nullptr, ctx, "/nc:rpc", nullptr, 0, &rpcNode) != LY_SUCCESS) {
        return nullptr;
    }
    
    // Set message ID
    lyd_node* messageIdNode = nullptr;
    if (lyd_new_path(rpcNode, ctx, "/nc:rpc/message-id", messageId_.c_str(), 0, &messageIdNode) != LY_SUCCESS) {
        lyd_free_tree(rpcNode);
        return nullptr;
    }
    
    // Create the specific RPC operation based on type
    lyd_node* operationNode = nullptr;
    std::string operationPath = "/nc:rpc/" + type_;
    if (lyd_new_path(rpcNode, ctx, operationPath.c_str(), nullptr, 0, &operationNode) != LY_SUCCESS) {
        lyd_free_tree(rpcNode);
        return nullptr;
    }
    
    return rpcNode;
}

Request Request::fromYang(const ly_ctx* ctx, const lyd_node* node) {
    // Parse NETCONF RPC node to extract request information
    std::string type = "";
    std::string data = "";
    std::string messageId = "";
    
    // Find the RPC operation node
    lyd_node* child = lyd_child(node);
    while (child) {
        if (strcmp(child->schema->name, "message-id") == 0) {
            messageId = lyd_get_value(child);
        } else {
            // This is the operation node
            type = child->schema->name;
            // Extract operation-specific data
            lyd_node* opChild = lyd_child(child);
            while (opChild) {
                data += std::string(opChild->schema->name) + ":" + lyd_get_value(opChild) + ";";
                opChild = opChild->next;
            }
        }
        child = child->next;
    }
    
    return Request(type, data, messageId);
}


// GetConfigRequest implementations
GetConfigRequest::GetConfigRequest(const std::string& source)
    : Request("get-config", source), source_(source) {
}

GetConfigRequest::GetConfigRequest(const std::string& source, const std::string& filter)
    : Request("get-config", source), source_(source), filter_(filter) {
}


GetConfigRequest GetConfigRequest::fromYang(const ly_ctx* ctx, const lyd_node* node) {
    // Parse get-config specific data
    std::string source = "running";
    std::string messageId = "";
    
    // Find the get-config node
    lyd_node* child = lyd_child(node);
    while (child) {
        if (strcmp(child->schema->name, "message-id") == 0) {
            messageId = lyd_get_value(child);
        } else if (strcmp(child->schema->name, "get-config") == 0) {
            // Parse get-config specific data
            lyd_node* getConfigChild = lyd_child(child);
            while (getConfigChild) {
                if (strcmp(getConfigChild->schema->name, "source") == 0) {
                    lyd_node* sourceChild = lyd_child(getConfigChild);
                    if (sourceChild) {
                        source = sourceChild->schema->name;
                    }
                }
                getConfigChild = getConfigChild->next;
            }
        }
        child = child->next;
    }
    
    return GetConfigRequest(source);
}

// EditConfigRequest implementations
EditConfigRequest::EditConfigRequest(const std::string& target, const std::string& config)
    : Request("edit-config", config), target_(target), config_(config) {
}


EditConfigRequest EditConfigRequest::fromYang(const ly_ctx* ctx, const lyd_node* node) {
    // Parse edit-config specific data
    std::string target = "candidate";
    std::string config = "";
    std::string messageId = "";
    
    // Find the edit-config node
    lyd_node* child = lyd_child(node);
    while (child) {
        if (strcmp(child->schema->name, "message-id") == 0) {
            messageId = lyd_get_value(child);
        } else if (strcmp(child->schema->name, "edit-config") == 0) {
            // Parse edit-config specific data
            lyd_node* editConfigChild = lyd_child(child);
            while (editConfigChild) {
                if (strcmp(editConfigChild->schema->name, "target") == 0) {
                    lyd_node* targetChild = lyd_child(editConfigChild);
                    if (targetChild) {
                        target = targetChild->schema->name;
                    }
                } else if (strcmp(editConfigChild->schema->name, "config") == 0) {
                    // Extract config data
                    config = lyd_get_value(editConfigChild);
                }
                editConfigChild = editConfigChild->next;
            }
        }
        child = child->next;
    }
    
    return EditConfigRequest(target, config);
}

// CommitRequest implementations

CommitRequest CommitRequest::fromYang(const ly_ctx* ctx, const lyd_node* node) {
    // Parse commit request - just need to extract message ID
    std::string messageId = "";
    
    lyd_node* child = lyd_child(node);
    while (child) {
        if (strcmp(child->schema->name, "message-id") == 0) {
            messageId = lyd_get_value(child);
            break;
        }
        child = child->next;
    }
    
    return CommitRequest();
}

// GetRequest implementations
GetRequest::GetRequest(const std::string& filter)
    : Request("get", filter), filter_(filter) {
}

GetRequest GetRequest::fromYang(const ly_ctx* ctx, const lyd_node* node) {
    // TODO: Implement get request parsing from YANG
    return GetRequest("");
}

} // namespace netd
