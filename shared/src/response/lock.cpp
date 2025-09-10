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

#include <libnetconf2/messages_server.h>
#include <libnetconf2/netconf.h>
#include <libyang/libyang.h>
#include <libyang/tree_data.h>
#include <shared/include/exception.hpp>
#include <shared/include/response/lock.hpp>
#include <shared/include/yang.hpp>

namespace netd::shared::response {

  LockResponse::LockResponse() {}

  LockResponse::~LockResponse() {}

  lyd_node *LockResponse::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      return nullptr;
    }

    // Create the complete rpc-reply structure with envelope
    lyd_node *replyNode = nullptr;
    if (lyd_new_opaq2(nullptr, ctx, "rpc-reply", nullptr, nullptr,
                      "urn:ietf:params:xml:ns:netconf:base:1.0",
                      &replyNode) != LY_SUCCESS) {
      return nullptr;
    }

    // Add message-id attribute to the rpc-reply envelope
    if (lyd_new_meta(nullptr, replyNode, nullptr, "message-id", "1", 0,
                     nullptr) != LY_SUCCESS) {
      lyd_free_tree(replyNode);
      return nullptr;
    }

    // Create the ok element inside the rpc-reply
    lyd_node *okNode = nullptr;
    if (lyd_new_opaq2(replyNode, nullptr, "ok", nullptr, nullptr,
                      "urn:ietf:params:xml:ns:netconf:base:1.0",
                      &okNode) != LY_SUCCESS) {
      lyd_free_tree(replyNode);
      return nullptr;
    }

    return replyNode;
  }

  std::unique_ptr<Response>
  LockResponse::fromYang([[maybe_unused]] const ly_ctx *ctx,
                         const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to LockResponse::fromYang");
    }

    return std::make_unique<LockResponse>();
  }


} // namespace netd::shared::response
