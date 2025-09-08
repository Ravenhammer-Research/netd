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

#include <libyang/libyang.h>
#include <libyang/tree_data.h>
#include <libnetconf2/messages_server.h>
#include <libnetconf2/netconf.h>
#include <shared/include/exception.hpp>
#include <shared/include/response/get/yanglib.hpp>
#include <shared/include/yang.hpp>
#include <shared/include/logger.hpp>

namespace netd::shared::response::get {

  // Copy of the private struct from libnetconf2/src/messages_p.h
  struct nc_server_reply_data {
    NC_RPL type;
    struct lyd_node *data;  /**< always points to the operation, for both RPCs and actions */
    int free;
    NC_WD_MODE wd;
  };

  GetYanglibResponse::GetYanglibResponse() {}

  GetYanglibResponse::GetYanglibResponse(GetYanglibResponse &&other) noexcept
      : GetResponse(), modules_(std::move(other.modules_)) {}

  GetYanglibResponse &
  GetYanglibResponse::operator=(GetYanglibResponse &&other) noexcept {
    if (this != &other) {
      modules_ = std::move(other.modules_);
    }
    return *this;
  }

  GetYanglibResponse::~GetYanglibResponse() {}

  void GetYanglibResponse::addModule(const YangModule &module) {
    modules_.push_back(module);
  }

  void GetYanglibResponse::addModule(const std::string &name, 
                                     const std::string &revision,
                                     const std::string &namespace_) {
    YangModule module;
    module.name = name;
    module.revision = revision;
    module.namespace_ = namespace_;
    modules_.push_back(module);
  }

  void GetYanglibResponse::setYanglibData(struct lyd_node *data) {
    yanglibData_ = data;
  }

  lyd_node *GetYanglibResponse::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      return nullptr;
    }

    // If we have pre-built YANG library data, use it directly
    if (yanglibData_) {
      // Create the complete rpc-reply structure with envelope
      lyd_node *replyNode = nullptr;
      if (lyd_new_opaq2(nullptr, ctx, "rpc-reply", nullptr, nullptr,
                        "urn:ietf:params:xml:ns:netconf:base:1.0",
                        &replyNode) != LY_SUCCESS) {
        return nullptr;
      }

      // Add message-id attribute to the rpc-reply envelope
      if (lyd_new_attr(replyNode, nullptr, "message-id", "1", 0) != LY_SUCCESS) {
        lyd_free_tree(replyNode);
        return nullptr;
      }

      // Create the data element inside the rpc-reply
      lyd_node *dataNode = nullptr;
      if (lyd_new_opaq2(replyNode, nullptr, "data", nullptr, nullptr,
                        "urn:ietf:params:xml:ns:netconf:base:1.0",
                        &dataNode) != LY_SUCCESS) {
        lyd_free_tree(replyNode);
        return nullptr;
      }

      // Copy the pre-built YANG library data into the data node
      if (lyd_dup_siblings(yanglibData_, (struct lyd_node_inner *)dataNode, LYD_DUP_RECURSIVE, nullptr) != LY_SUCCESS) {
        lyd_free_tree(replyNode);
        return nullptr;
      }

      return replyNode;
    }

    // Fallback: Create the complete rpc-reply structure with envelope manually
    lyd_node *replyNode = nullptr;
    if (lyd_new_opaq2(nullptr, ctx, "rpc-reply", nullptr, nullptr,
                      "urn:ietf:params:xml:ns:netconf:base:1.0",
                      &replyNode) != LY_SUCCESS) {
      return nullptr;
    }

    // Add message-id attribute to the rpc-reply envelope
    if (lyd_new_attr(replyNode, nullptr, "message-id", "1", 0) != LY_SUCCESS) {
      lyd_free_tree(replyNode);
      return nullptr;
    }

    // Create the data element inside the rpc-reply
    lyd_node *dataNode = nullptr;
    if (lyd_new_opaq2(replyNode, nullptr, "data", nullptr, nullptr,
                      "urn:ietf:params:xml:ns:netconf:base:1.0",
                      &dataNode) != LY_SUCCESS) {
      lyd_free_tree(replyNode);
      return nullptr;
    }

    // Create yang-library container
    lyd_node *yanglibNode = nullptr;
    if (lyd_new_opaq2(dataNode, nullptr, "yang-library", nullptr, nullptr,
                      "urn:ietf:params:xml:ns:yang:ietf-yang-library",
                      &yanglibNode) != LY_SUCCESS) {
      lyd_free_tree(replyNode);
      return nullptr;
    }

    // Create module-set container
    lyd_node *moduleSetNode = nullptr;
    if (lyd_new_opaq2(yanglibNode, nullptr, "module-set", nullptr, nullptr,
                      "urn:ietf:params:xml:ns:yang:ietf-yang-library",
                      &moduleSetNode) != LY_SUCCESS) {
      lyd_free_tree(replyNode);
      return nullptr;
    }

    // Add name attribute to module-set
    if (lyd_new_attr(moduleSetNode, nullptr, "name", "default", nullptr) != LY_SUCCESS) {
      lyd_free_tree(replyNode);
      return nullptr;
    }

    // Add each module to the module-set
    for (const auto &module : modules_) {
      lyd_node *moduleNode = nullptr;
      if (lyd_new_opaq2(moduleSetNode, nullptr, "module", nullptr, nullptr,
                        "urn:ietf:params:xml:ns:yang:ietf-yang-library",
                        &moduleNode) != LY_SUCCESS) {
        lyd_free_tree(replyNode);
        return nullptr;
      }

      // Add name attribute
      if (lyd_new_attr(moduleNode, nullptr, "name", module.name.c_str(), nullptr) != LY_SUCCESS) {
        lyd_free_tree(replyNode);
        return nullptr;
      }

      // Add revision attribute if present
      if (!module.revision.empty()) {
        if (lyd_new_attr(moduleNode, nullptr, "revision", module.revision.c_str(), nullptr) != LY_SUCCESS) {
          lyd_free_tree(replyNode);
          return nullptr;
        }
      }

      // Add namespace attribute
      if (!module.namespace_.empty()) {
        if (lyd_new_attr(moduleNode, nullptr, "namespace", module.namespace_.c_str(), nullptr) != LY_SUCCESS) {
          lyd_free_tree(replyNode);
          return nullptr;
        }
      }
    }

    return replyNode;
  }

  std::unique_ptr<Response>
  GetYanglibResponse::fromYang([[maybe_unused]] const ly_ctx *ctx,
                               const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to GetYanglibResponse::fromYang");
    }

    auto response = std::make_unique<GetYanglibResponse>();
    
    // Parse the received data node - walk through the YANG tree
    const lyd_node *child = lyd_child(node);
    while (child) {
      const char *nodeName = lyd_node_schema(child) ? lyd_node_schema(child)->name : nullptr;
      
      if (nodeName && strcmp(nodeName, "yang-library") == 0) {
        // Found yang-library container, parse module data
        const lyd_node *yanglibChild = lyd_child(child);
        while (yanglibChild) {
          const char *yanglibChildName = lyd_node_schema(yanglibChild) ? lyd_node_schema(yanglibChild)->name : nullptr;
          
          if (yanglibChildName && strcmp(yanglibChildName, "module-set") == 0) {
            // Found module-set, parse modules
            const lyd_node *moduleChild = lyd_child(yanglibChild);
            while (moduleChild) {
              const char *moduleName = lyd_node_schema(moduleChild) ? lyd_node_schema(moduleChild)->name : nullptr;
              
              if (moduleName && strcmp(moduleName, "module") == 0) {
                // Found a module, extract its data
                YangModule module;
                
                // Cast to opaque node to access attributes
                const struct lyd_node_opaq *opaqNode = (const struct lyd_node_opaq *)moduleChild;
                
                // Extract module attributes by iterating through attributes
                const struct lyd_attr *attr = opaqNode->attr;
                while (attr) {
                  if (strcmp(attr->name.name, "name") == 0) {
                    module.name = std::string(attr->value);
                  } else if (strcmp(attr->name.name, "revision") == 0) {
                    module.revision = std::string(attr->value);
                  } else if (strcmp(attr->name.name, "namespace") == 0) {
                    module.namespace_ = std::string(attr->value);
                  }
                  attr = attr->next;
                }
                
                response->addModule(module);
              }
              moduleChild = moduleChild->next;
            }
          }
          yanglibChild = yanglibChild->next;
        }
      }
      child = child->next;
    }
    
    return response;
  }

  struct nc_server_reply *
  GetYanglibResponse::toNetconfReply(struct nc_session *session) const {
    if (error) {
      return nc_server_reply_err(nc_err(nc_session_get_ctx(session),
                                        NC_ERR_OP_FAILED,
                                        error->getMessage().c_str()));
    }

    // Get the YANG context from the session
    const ly_ctx *const_ctx = nc_session_get_ctx(session);
    if (!const_ctx) {
      return nc_server_reply_err(nc_err(nullptr, NC_ERR_OP_FAILED,
                                        "Failed to get YANG context from session"));
    }

    // Cast to non-const for toYang method (it doesn't actually modify the context)
    ly_ctx *ctx = const_cast<ly_ctx *>(const_ctx);

    // The solution: Create a proper get RPC output structure following the libnetconf2 example
    // We need to create a get RPC node and add the YANG library data as its output
    
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Creating proper get RPC output structure...");
    
    // First, get the YANG library data
    struct lyd_node *yanglibData = nullptr;
    if (ly_ctx_get_yanglib_data(ctx, &yanglibData, "ietf-yang-library@2019-01-04") == LY_SUCCESS && yanglibData) {
      logger.info("Got YANG library data, creating get RPC output structure...");
      
      // Create a new get RPC node (this will have the proper RPC schema)
      const struct lys_module *netconf_module = ly_ctx_get_module(ctx, "ietf-netconf", "2013-09-29");
      if (netconf_module) {
        // Create a new get RPC node
        struct lyd_node *get_rpc_node = nullptr;
        if (lyd_new_inner(nullptr, netconf_module, "get", 0, &get_rpc_node) == LY_SUCCESS) {
          logger.info("Created get RPC node, adding data output...");
          
          // Create the get RPC anyxml "data" output node with the YANG library data
          if (lyd_new_any(get_rpc_node, nullptr, "data", yanglibData, LYD_ANYDATA_DATATREE, 
                         LYD_NEW_ANY_USE_VALUE | LYD_NEW_VAL_OUTPUT, nullptr) == LY_SUCCESS) {
            logger.info("Added data output to get RPC, creating reply structure...");
            
            // Create the nc_server_reply_data struct directly
            struct nc_server_reply_data *reply_data = (struct nc_server_reply_data *)malloc(sizeof(struct nc_server_reply_data));
            if (!reply_data) {
              logger.info("Failed to allocate nc_server_reply_data struct");
              lyd_free_tree(get_rpc_node);
              return nc_server_reply_err(nc_err(ctx, NC_ERR_OP_FAILED, "Failed to allocate reply structure"));
            }
            
            // Fill in the structure
            reply_data->type = NC_RPL_DATA;
            reply_data->data = get_rpc_node;  // The get RPC node with proper schema
            reply_data->free = 1;             // We want it to be freed when the reply is freed
            reply_data->wd = NC_WD_UNKNOWN;   // with-defaults mode
            
            logger.info("Created proper get RPC output structure, returning it...");
            
            // Cast to nc_server_reply and return
            return (struct nc_server_reply *)reply_data;
          } else {
            logger.info("Failed to add data output to get RPC");
            lyd_free_tree(get_rpc_node);
          }
        } else {
          logger.info("Failed to create get RPC node");
        }
      } else {
        logger.info("ietf-netconf module not found");
      }
      
      // Free the YANG library data if we couldn't use it
      lyd_free_tree(yanglibData);
    } else {
      logger.info("Failed to get YANG library data");
    }
    
    // Fallback: If we couldn't get YANG library data, send an OK response
    logger.info("Failed to get YANG library data, sending OK response");
    
    struct nc_server_reply *reply = nc_server_reply_ok();
    if (!reply) {
      return nc_server_reply_err(nc_err(ctx, NC_ERR_OP_FAILED,
                                        "Failed to create server reply"));
    }
    return reply;
  }


} // namespace netd::shared::response::get
