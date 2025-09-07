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
#include <shared/include/logger.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/yang.hpp>

namespace netd::shared::request::get {

  using netd::shared::ArgumentError;
  using netd::shared::NotImplementedError;

  // Helper function to convert Datastore enum to string
  std::string datastoreToString(Datastore datastore) {
    switch (datastore) {
    case Datastore::RUNNING:
      return "running";
    case Datastore::CANDIDATE:
      return "candidate";
    case Datastore::STARTUP:
      return "startup";
    default:
      return "running";
    }
  }

  lyd_node *GetConfigRequest::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw netd::shared::ArgumentError("toYang: ctx is null");
    }

    // Get the ietf-netconf module - try different approaches
    const struct lys_module *netconf_module = ly_ctx_get_module(ctx, "ietf-netconf", nullptr);
    if (!netconf_module) {
      // Try with version
      netconf_module = ly_ctx_get_module(ctx, "ietf-netconf", "2013-09-29");
    }
    if (!netconf_module) {
      // Try with latest version
      netconf_module = ly_ctx_get_module(ctx, "ietf-netconf", "2011-06-01");
    }
    if (!netconf_module) {
      throw netd::shared::ArgumentError("toYang: ietf-netconf module not found in context");
    }

    // Create get-config element directly (nc_rpc_act_generic_xml will wrap it in <rpc>)
    lyd_node *getConfigNode = nullptr;
    if (lyd_new_opaq2(nullptr, ctx, "get-config", nullptr, nullptr,
                      "urn:ietf:params:xml:ns:netconf:base:1.0",
                      &getConfigNode) != LY_SUCCESS) {
      throw netd::shared::ArgumentError("toYang: failed to create get-config element");
    }

    // Create source element
    lyd_node *sourceNode = nullptr;
    if (lyd_new_opaq2(getConfigNode, ctx, "source", nullptr, nullptr,
                      "urn:ietf:params:xml:ns:netconf:base:1.0",
                      &sourceNode) != LY_SUCCESS) {
      lyd_free_tree(getConfigNode);
      throw netd::shared::ArgumentError("toYang: failed to create source element");
    }

    // Create datastore element (running, candidate, or startup)
    std::string datastoreName = datastoreToString(source_);
    lyd_node *datastoreNode = nullptr;
    if (lyd_new_opaq2(sourceNode, ctx, datastoreName.c_str(), nullptr, nullptr,
                      "urn:ietf:params:xml:ns:netconf:base:1.0",
                      &datastoreNode) != LY_SUCCESS) {
      lyd_free_tree(getConfigNode);
      throw netd::shared::ArgumentError("toYang: failed to create datastore element");
    }

    // Add filter if requested module is specified
    if (requestedModule_ != "all") {
      lyd_node *filterNode = nullptr;
      if (lyd_new_opaq2(getConfigNode, ctx, "filter", nullptr, nullptr,
                        "urn:ietf:params:xml:ns:netconf:base:1.0",
                        &filterNode) != LY_SUCCESS) {
        lyd_free_tree(getConfigNode);
        throw netd::shared::ArgumentError("toYang: failed to create filter element");
      }

      // Add type="subtree" attribute using lyd_new_attr for opaque nodes
      if (lyd_new_attr(filterNode, nullptr, "type", "subtree", nullptr) != LY_SUCCESS) {
        lyd_free_tree(getConfigNode);
        throw netd::shared::ArgumentError("toYang: failed to add type attribute to filter");
      }

      // Add subtree filter content - create the actual interface node
      if (lyd_new_opaq2(filterNode, ctx, requestedModule_.c_str(), nullptr, nullptr,
                        "urn:ietf:params:xml:ns:yang:ietf-interfaces", nullptr) != LY_SUCCESS) {
        lyd_free_tree(getConfigNode);
        throw netd::shared::ArgumentError("toYang: failed to add filter content");
      }
    }

    return getConfigNode;
  }

  std::unique_ptr<GetConfigRequest>
  GetConfigRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                             const lyd_node *node) {

    auto request = std::make_unique<GetConfigRequest>();

    // Find the get-config node
    lyd_node *getConfigNode = lyd_child(node);

    if (!getConfigNode) {
      throw ArgumentError("Invalid node: no children found");
    }

    while (getConfigNode &&
           strcmp(lyd_node_schema(getConfigNode)->name, "get-config") != 0) {
      getConfigNode = getConfigNode->next;
    }

    if (!getConfigNode) {
      throw ArgumentError("get-config node not found");
    }

    // Find the source container
    lyd_node *sourceNode = lyd_child(getConfigNode);

    while (sourceNode &&
           strcmp(lyd_node_schema(sourceNode)->name, "source") != 0) {
      sourceNode = sourceNode->next;
    }

    if (!sourceNode) {
      throw ArgumentError("source node not found in get-config");
    }

    // Find the datastore source (running, candidate, startup)
    lyd_node *datastoreNode = lyd_child(sourceNode);

    if (!datastoreNode) {
      throw ArgumentError("datastore node not found in source");
    }

    while (datastoreNode) {
      const char *nodeName = lyd_node_schema(datastoreNode)->name;
      std::string sourceStr(nodeName);

      // Validate datastore name
      if (sourceStr != "running" && sourceStr != "candidate" && sourceStr != "startup") {
        throw ArgumentError("Invalid datastore: " + sourceStr);
      }

      datastoreNode = datastoreNode->next;
    }

    // Look for filter element to determine requested module
    lyd_node *filterNode = lyd_child(getConfigNode);
    while (filterNode &&
           strcmp(lyd_node_schema(filterNode)->name, "filter") != 0) {
      filterNode = filterNode->next;
    }

    if (filterNode) {
      // Look for the first child element in the filter to determine module
      lyd_node *moduleNode = lyd_child(filterNode);
      if (moduleNode) {
        const struct lysc_node *schemaNode = lyd_node_schema(moduleNode);
        if (schemaNode && schemaNode->module) {
          request->requestedModule_ = std::string(schemaNode->module->name);
        }
      }
    }

    return request;
  }

} // namespace netd::shared::request::get

