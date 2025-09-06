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
#include <libnetconf2/netconf.h>
#include <shared/include/exception.hpp>
#include <shared/include/request/unlock.hpp>
#include <shared/include/yang.hpp>

namespace netd::shared::request {



  lyd_node *UnlockRequest::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      return nullptr;
    }

    // Get the ietf-netconf module
    const struct lys_module *mod =
        ly_ctx_get_module_implemented(ctx, "ietf-netconf");
    if (!mod) {
      return nullptr;
    }

    // Create the complete RPC structure with envelope
    lyd_node *rpcNode = nullptr;
    if (lyd_new_inner(nullptr, mod, "rpc", 0, &rpcNode) != LY_SUCCESS) {
      return nullptr;
    }

    // Add message-id attribute to the RPC envelope
    if (lyd_new_meta(nullptr, rpcNode, nullptr, "message-id", "1", 0,
                     nullptr) != LY_SUCCESS) {
      lyd_free_tree(rpcNode);
      return nullptr;
    }

    // Create the unlock operation inside the RPC
    lyd_node *unlockNode = nullptr;
    if (lyd_new_inner(rpcNode, mod, "unlock", 0, &unlockNode) != LY_SUCCESS) {
      lyd_free_tree(rpcNode);
      return nullptr;
    }

    // Create the target container
    lyd_node *targetNode = nullptr;
    if (lyd_new_inner(unlockNode, mod, "target", 0, &targetNode) !=
        LY_SUCCESS) {
      lyd_free_tree(rpcNode);
      return nullptr;
    }

    // Add running datastore as default
    if (lyd_new_term(targetNode, mod, "running", nullptr, 0, nullptr) !=
        LY_SUCCESS) {
      lyd_free_tree(rpcNode);
      return nullptr;
    }

    return rpcNode;
  }

  std::unique_ptr<UnlockRequest>
  UnlockRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                          const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to UnlockRequest::fromYang");
    }

    return std::make_unique<UnlockRequest>();
  }

} // namespace netd::shared::request
