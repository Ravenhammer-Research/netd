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

#include <libnetconf2/netconf.h>
#include <libyang/libyang.h>
#include <libyang/tree_data.h>
#include <shared/include/exception.hpp>
#include <shared/include/request/get/base.hpp>
#include <shared/include/yang.hpp>
#include <shared/include/logger.hpp>

namespace netd::shared::request::get {

  using netd::shared::ArgumentError;
  using netd::shared::NotImplementedError;

  lyd_node *GetRequest::toYang([[maybe_unused]] ly_ctx *ctx) const {
    throw netd::shared::NotImplementedError("GetRequest::toYang not implemented");
  }


  void GetRequest::parseRpcData(const lyd_node *node) {
    if (!node) {
      return;
    }

    auto &logger = netd::shared::Logger::getInstance();
    logger.info("parseRpcData: Starting to parse RPC node");
    
    // Debug: Check what type of node we're starting with
    const char *rootNodeName = nullptr;
    if (node->schema) {
      rootNodeName = node->schema->name;
    } else {
      const struct lyd_node_opaq *opaqNode = (const struct lyd_node_opaq *)node;
      rootNodeName = opaqNode->name.name;
    }
    logger.info("parseRpcData: Root node name: " + std::string(rootNodeName ? rootNodeName : "null"));

    // Determine if we're starting from the rpc node or the get node
    lyd_node *getNode = nullptr;
    if (rootNodeName && strcmp(rootNodeName, "get") == 0) {
      // We're already at the get node
      getNode = const_cast<lyd_node*>(node);
      logger.info("parseRpcData: Already at get node, using it directly");
    } else {
      // We're at the rpc node, need to find the get node
      getNode = lyd_child(node);
      logger.info("parseRpcData: Looking for get node, first child is: " + std::string(getNode ? "found" : "null"));
      while (getNode) {
        const char *nodeName = nullptr;
        if (getNode->schema) {
          nodeName = getNode->schema->name;
        } else {
          // For opaque nodes, get the name from the opaque structure
          const struct lyd_node_opaq *opaqNode = (const struct lyd_node_opaq *)getNode;
          nodeName = opaqNode->name.name;
        }
        
        logger.info("parseRpcData: Found RPC child node: " + std::string(nodeName ? nodeName : "null"));
        
        if (nodeName && strcmp(nodeName, "get") == 0) {
          logger.info("parseRpcData: Found get node!");
          break;
        }
        getNode = getNode->next;
      }
    }

    if (!getNode) {
      logger.info("parseRpcData: No get node found");
      return; // No get node found, that's okay
    }

    // Look for filter element
    lyd_node *filterNode = lyd_child(getNode);
    while (filterNode) {
      // Check if this is a filter node (could be opaque or regular)
      const char *nodeName = nullptr;
      if (filterNode->schema) {
        nodeName = filterNode->schema->name;
      } else {
        // For opaque nodes, get the name from the opaque structure
        const struct lyd_node_opaq *opaqNode = (const struct lyd_node_opaq *)filterNode;
        nodeName = opaqNode->name.name;
      }
      
      // Debug logging
      auto &logger = netd::shared::Logger::getInstance();
      logger.info("Found child node: " + std::string(nodeName ? nodeName : "null"));
      
      if (nodeName && strcmp(nodeName, "filter") == 0) {
        logger.info("Found filter node!");
        break;
      }
      filterNode = filterNode->next;
    }

    if (filterNode) {
      hasFilter_ = true;
      logger.info("parseRpcData: Processing filter node attributes");
      
      // Debug: Check what type of node this is
      if (filterNode->schema) {
        logger.info("parseRpcData: Filter node has schema: " + std::string(filterNode->schema->name ? filterNode->schema->name : "null"));
      } else {
        logger.info("parseRpcData: Filter node has NO schema (opaque node)");
      }
      
      // Get filter type and select from metadata (like libnetconf2 does)
      logger.info("parseRpcData: Checking filter metadata for type and select attributes");
      
      // Process metadata like libnetconf2 does
      if (filterNode->meta) {
        logger.info("parseRpcData: Filter node has metadata");
        const struct lyd_meta *meta = filterNode->meta;
        while (meta) {
          if (meta->name) {
            const char *metaValue = lyd_get_meta_value(meta);
            std::string metaName(meta->name);
            std::string metaVal(metaValue ? metaValue : "null");
            logger.info("parseRpcData: Found metadata: " + metaName + " = " + metaVal);
            
            if (strcmp(meta->name, "type") == 0) {
              filterType_ = metaValue ? std::string(metaValue) : "";
              logger.info("parseRpcData: Set filterType to: " + filterType_);
            } else if (strcmp(meta->name, "select") == 0) {
              filterSelect_ = metaValue ? std::string(metaValue) : "";
              logger.info("parseRpcData: Set filterSelect to: " + filterSelect_);
            }
          }
          meta = meta->next;
        }
      } else {
        logger.info("parseRpcData: Filter node has NO metadata");
      }
      
      // Also try to get the filter content as XML to extract yang-library info
      char *filterXml = nullptr;
      if (lyd_print_mem(&filterXml, filterNode, LYD_XML, 0) == LY_SUCCESS && filterXml) {
        logger.info("parseRpcData: Filter XML content: " + std::string(filterXml));
        
        // Check if this contains yang-library
        std::string filterContent(filterXml);
        if (filterContent.find("yang-library") != std::string::npos ||
            filterContent.find("ietf-yang-library") != std::string::npos) {
          logger.info("parseRpcData: Found yang-library in filter content!");
          // Set the filter select to the XML content for yanglib detection
          filterSelect_ = filterContent;
        }
        
        free(filterXml);
      }
      
      // Handle different filter types
      if (filterType_ == "subtree") {
        logger.info("parseRpcData: Detected subtree filter - assuming yang-library request");
        filterSelect_ = "yang-library";
      } else if (filterType_ == "xpath") {
        logger.info("parseRpcData: Detected XPath filter");
        // For XPath filters, check if the select contains yang-library
        if (filterSelect_.find("ietf-yang-library") != std::string::npos ||
            filterSelect_.find("yang-library") != std::string::npos) {
          logger.info("parseRpcData: Found yang-library in XPath filter");
          filterSelect_ = "yang-library";
        }
      }
    }
  }

  std::unique_ptr<GetRequest>
  GetRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                       const lyd_node *node) {
    if (!node) {
      throw netd::shared::ArgumentError("Invalid YANG node provided to GetRequest::fromYang");
    }

    auto request = std::make_unique<GetRequest>();
    request->parseRpcData(node);
    return request;
  }

} // namespace netd::shared::request::get
