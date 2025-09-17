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
#include <shared/include/logger.hpp>
#include <shared/include/response/get/config.hpp>
#include <shared/include/yang.hpp>

namespace netd::shared::response::get {

  GetConfigResponse::GetConfigResponse() {}

  GetConfigResponse::GetConfigResponse(GetConfigResponse &&other) noexcept
      : Response(std::move(other)) {}

  GetConfigResponse &
  GetConfigResponse::operator=(GetConfigResponse &&other) noexcept {
    if (this != &other) {
      Response::operator=(std::move(other));
    }
    return *this;
  }

  GetConfigResponse::~GetConfigResponse() {}

  lyd_node *GetConfigResponse::toYang(ly_ctx *ctx) const {
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

    return replyNode;
  }

  std::unique_ptr<Response>
  GetConfigResponse::fromYang([[maybe_unused]] const ly_ctx *ctx,
                              const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to GetConfigResponse::fromYang");
    }

    auto response = std::make_unique<GetConfigResponse>();

    // Parse the received data node - walk through the YANG tree
    const lyd_node *child = lyd_child(node);
    while (child) {
      const char *nodeName =
          lyd_node_schema(child) ? lyd_node_schema(child)->name : nullptr;

      if (nodeName && strcmp(nodeName, "interfaces") == 0) {
        // Found interfaces container, parse interface data
        const lyd_node *interfaceChild = lyd_child(child);
        while (interfaceChild) {
          const char *interfaceName =
              lyd_node_schema(interfaceChild)
                  ? lyd_node_schema(interfaceChild)->name
                  : nullptr;

          if (interfaceName && strcmp(interfaceName, "interface") == 0) {
            // Found an interface, extract its data
            const lyd_node *interfaceData = lyd_child(interfaceChild);
            while (interfaceData) {
              const char *dataName = lyd_node_schema(interfaceData)
                                         ? lyd_node_schema(interfaceData)->name
                                         : nullptr;
              if (dataName) {
                // Extract interface properties like name, type, enabled, etc.
                // TODO: Store the actual interface data in the response object
              }
              interfaceData = interfaceData->next;
            }
          }
          interfaceChild = interfaceChild->next;
        }
      }
      child = child->next;
    }

    return response;
  }

} // namespace netd::shared::response::get
