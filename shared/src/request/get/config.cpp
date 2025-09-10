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

    // Get the ietf-netconf module
    const struct lys_module *mod = ly_ctx_get_module(ctx, "ietf-netconf", "2011-06-01");
    if (!mod) {
      throw netd::shared::ArgumentError("toYang: ietf-netconf module not found");
    }

    // Create get-config element
    lyd_node *getConfigNode = nullptr;
    if (lyd_new_inner(nullptr, mod, "get-config", 0, &getConfigNode) != LY_SUCCESS) {
      throw netd::shared::ArgumentError("toYang: failed to create get-config element");
    }

    // Create source element
    lyd_node *sourceNode = nullptr;
    if (lyd_new_inner(getConfigNode, mod, "source", 0, &sourceNode) != LY_SUCCESS) {
      lyd_free_tree(getConfigNode);
      throw netd::shared::ArgumentError("toYang: failed to create source element");
    }

    // Create datastore element (running, candidate, or startup)
    std::string datastoreName = datastoreToString(source_);
    lyd_node *datastoreNode = nullptr;
    if (lyd_new_inner(sourceNode, mod, datastoreName.c_str(), 0, &datastoreNode) != LY_SUCCESS) {
      lyd_free_tree(getConfigNode);
      throw netd::shared::ArgumentError("toYang: failed to create datastore element");
    }

    // Add filter if requested module is specified
    if (requestedModule_ != "all") {
      lyd_node *filterNode = nullptr;
      if (lyd_new_inner(getConfigNode, mod, "filter", 0, &filterNode) != LY_SUCCESS) {
        lyd_free_tree(getConfigNode);
        throw netd::shared::ArgumentError("toYang: failed to create filter element");
      }

      // Add type="subtree" attribute
      if (lyd_new_attr(filterNode, nullptr, "type", "subtree", nullptr) != LY_SUCCESS) {
        lyd_free_tree(getConfigNode);
        throw netd::shared::ArgumentError("toYang: failed to add type attribute to filter");
      }

      // Add subtree filter content - create the actual interface node
      std::string containerName;
      std::string namespaceUri;
      if (requestedModule_ == "ietf-interfaces") {
        containerName = "interfaces";
        namespaceUri = "urn:ietf:params:xml:ns:yang:ietf-interfaces";
      } else {
        // For other modules, use the module name as-is
        containerName = requestedModule_;
        namespaceUri = "urn:ietf:params:xml:ns:yang:" + requestedModule_;
      }
      
      // Create the container node in the filter
      const struct lys_module *target_mod = ly_ctx_get_module(ctx, requestedModule_.c_str(), nullptr);
      if (target_mod) {
        lyd_node *containerNode = nullptr;
        if (lyd_new_inner(filterNode, target_mod, containerName.c_str(), 0, &containerNode) != LY_SUCCESS) {
          lyd_free_tree(getConfigNode);
          throw netd::shared::ArgumentError("toYang: failed to add filter content");
        }
      }
    }

    return getConfigNode;
  }

  std::unique_ptr<GetConfigRequest>
  GetConfigRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                             [[maybe_unused]] const lyd_node *node) {
    throw netd::shared::NotImplementedError("GetConfigRequest::fromYang not implemented");
  }

} // namespace netd::shared::request::get