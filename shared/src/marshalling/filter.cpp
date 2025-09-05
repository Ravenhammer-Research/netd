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
#include <shared/include/exception.hpp>
#include <shared/include/marshalling/filter.hpp>
#include <shared/include/yang.hpp>

namespace netd::shared::marshalling {

  // Base Filter class
  Filter::Filter() { type = FilterType::SUBTREE; }

  // SubtreeFilter implementation
  SubtreeFilter::SubtreeFilter() { type = FilterType::SUBTREE; }

  lyd_node *SubtreeFilter::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      return nullptr;
    }

    // Create filter element
    lyd_node *filterNode = nullptr;
    if (lyd_new_opaq2(nullptr, ctx, "filter", nullptr, nullptr,
                      "urn:ietf:params:xml:ns:netconf:base:1.0",
                      &filterNode) != LY_SUCCESS) {
      return nullptr;
    }

    // Add type attribute
    if (lyd_new_meta(nullptr, filterNode, nullptr, "type", "subtree", 0,
                     nullptr) != LY_SUCCESS) {
      lyd_free_tree(filterNode);
      return nullptr;
    }

    // Add subtree data as text content
    if (!subtreeData.empty()) {
      if (lyd_new_term(filterNode, nullptr, subtreeData.c_str(), nullptr, 0,
                       nullptr) != LY_SUCCESS) {
        lyd_free_tree(filterNode);
        return nullptr;
      }
    }

    return filterNode;
  }

  std::unique_ptr<Filter>
  SubtreeFilter::fromYang([[maybe_unused]] const ly_ctx *ctx,
                          const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to SubtreeFilter::fromYang");
    }

    auto filter = std::make_unique<SubtreeFilter>();
    // TODO: Parse subtree data from node
    return filter;
  }

  // XPathFilter implementation
  XPathFilter::XPathFilter() { type = FilterType::XPATH; }

  lyd_node *XPathFilter::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      return nullptr;
    }

    // Create filter element
    lyd_node *filterNode = nullptr;
    if (lyd_new_opaq2(nullptr, ctx, "filter", nullptr, nullptr,
                      "urn:ietf:params:xml:ns:netconf:base:1.0",
                      &filterNode) != LY_SUCCESS) {
      return nullptr;
    }

    // Add type attribute
    if (lyd_new_meta(nullptr, filterNode, nullptr, "type", "xpath", 0,
                     nullptr) != LY_SUCCESS) {
      lyd_free_tree(filterNode);
      return nullptr;
    }

    // Add select attribute for XPath
    if (!xpathData.empty()) {
      if (lyd_new_meta(nullptr, filterNode, nullptr, "select",
                       xpathData.c_str(), 0, nullptr) != LY_SUCCESS) {
        lyd_free_tree(filterNode);
        return nullptr;
      }
    }

    return filterNode;
  }

  std::unique_ptr<Filter>
  XPathFilter::fromYang([[maybe_unused]] const ly_ctx *ctx,
                        const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to XPathFilter::fromYang");
    }

    auto filter = std::make_unique<XPathFilter>();
    // TODO: Parse XPath data from node
    return filter;
  }

} // namespace netd::shared::marshalling
