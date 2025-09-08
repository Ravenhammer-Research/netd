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
#include <shared/include/request/get/yanglib.hpp>
#include <shared/include/yang.hpp>

namespace netd::shared::request::get {

  using netd::shared::ArgumentError;
  using netd::shared::NotImplementedError;

  lyd_node *GetYanglibRequest::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw netd::shared::ArgumentError("toYang: ctx is null");
    }

    // Create get element directly (nc_rpc_act_generic_xml will wrap it in <rpc>)
    lyd_node *getNode = nullptr;
    if (lyd_new_opaq2(nullptr, ctx, "get", nullptr, nullptr,
                      "urn:ietf:params:xml:ns:netconf:base:1.0",
                      &getNode) != LY_SUCCESS) {
      throw netd::shared::ArgumentError("toYang: failed to create get element");
    }

    // Add filter if present
    if (hasYanglibFilter()) {
      lyd_node *filterNode = nullptr;
      if (lyd_new_opaq2(getNode, ctx, "filter", nullptr, nullptr,
                        "urn:ietf:params:xml:ns:netconf:base:1.0",
                        &filterNode) != LY_SUCCESS) {
        lyd_free_tree(getNode);
        throw netd::shared::ArgumentError("toYang: failed to create filter element");
      }

      // Add type attribute to filter
      if (lyd_new_attr(filterNode, nullptr, "type", getYanglibFilterType().c_str(), nullptr) != LY_SUCCESS) {
        lyd_free_tree(getNode);
        throw netd::shared::ArgumentError("toYang: failed to add type attribute to filter");
      }

      // Add select attribute for xpath filters
      if (getYanglibFilterType() == "xpath" && !getYanglibFilterSelect().empty()) {
        if (lyd_new_attr(filterNode, nullptr, "select", getYanglibFilterSelect().c_str(), nullptr) != LY_SUCCESS) {
          lyd_free_tree(getNode);
          throw netd::shared::ArgumentError("toYang: failed to add select attribute to filter");
        }
      }
    }

    return getNode;
  }

  std::unique_ptr<GetYanglibRequest>
  GetYanglibRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                              const lyd_node *node) {
    if (!node) {
      throw ArgumentError("Invalid YANG node provided to GetYanglibRequest::fromYang");
    }

    auto request = std::make_unique<GetYanglibRequest>();

    // Find the get node
    lyd_node *getNode = lyd_child(node);
    while (getNode && strcmp(lyd_node_schema(getNode)->name, "get") != 0) {
      getNode = getNode->next;
    }

    if (!getNode) {
      throw ArgumentError("get node not found");
    }

    // Look for filter element
    lyd_node *filterNode = lyd_child(getNode);
    while (filterNode && strcmp(lyd_node_schema(filterNode)->name, "filter") != 0) {
      filterNode = filterNode->next;
    }

    if (filterNode) {
      request->hasFilter_ = true;
      
      // Cast to opaque node to access attributes
      const struct lyd_node_opaq *opaqNode = (const struct lyd_node_opaq *)filterNode;
      
      // Get filter type attribute by iterating through attributes
      const struct lyd_attr *attr = opaqNode->attr;
      while (attr) {
        if (strcmp(attr->name.name, "type") == 0) {
          request->filterType_ = std::string(attr->value);
          break;
        }
        attr = attr->next;
      }

      // Get select attribute for xpath filters
      if (request->filterType_ == "xpath") {
        attr = opaqNode->attr;
        while (attr) {
          if (strcmp(attr->name.name, "select") == 0) {
            request->filterSelect_ = std::string(attr->value);
            break;
          }
          attr = attr->next;
        }
      }
    }

    return request;
  }

} // namespace netd::shared::request::get
