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
#include <libyang/tree_data.h>
#include <shared/include/exception.hpp>
#include <shared/include/request/session/destroy.hpp>

namespace netd::shared::request::session {

  lyd_node *DestroyRequest::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw netd::shared::ArgumentError("toYang: ctx is null");
    }
    
    // Validate that the provided context matches the session context
    if (session_ && session_->getContext() != ctx) {
      throw netd::shared::ArgumentError("toYang: provided context does not match session context");
    }

    // Get the ietf-netconf module
    const struct lys_module *mod = ly_ctx_get_module(ctx, "ietf-netconf", "2011-06-01");
    if (!mod) {
      throw netd::shared::ArgumentError("toYang: ietf-netconf module not found");
    }

    // Create destroy-session element
    lyd_node *destroyNode = nullptr;
    if (lyd_new_inner(nullptr, mod, "destroy-session", 0, &destroyNode) != LY_SUCCESS) {
      throw netd::shared::ArgumentError("toYang: failed to create destroy-session element");
    }

    return destroyNode;
  }

  std::unique_ptr<DestroyRequest>
  DestroyRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                          const lyd_node *node) {
    if (!node) {
      throw netd::shared::ArgumentError("Invalid YANG node provided to DestroyRequest::fromYang");
    }

    auto request = std::make_unique<DestroyRequest>();

    // Find the destroy-session node
    lyd_node *destroyNode = lyd_child(node);
    while (destroyNode && strcmp(lyd_node_schema(destroyNode)->name, "destroy-session") != 0) {
      destroyNode = destroyNode->next;
    }

    if (!destroyNode) {
      throw netd::shared::ArgumentError("destroy-session element not found in RPC");
    }

    return request;
  }

} // namespace netd::shared::request::session
