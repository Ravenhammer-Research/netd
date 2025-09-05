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

#include <libyang/tree_data.h>
#include <shared/include/base/serialization.hpp>
#include <shared/include/interface/wireguard.hpp>
#include <shared/include/yang.hpp>

namespace netd::shared::interface {

  WireguardInterface::WireguardInterface() {}

  WireguardInterface::WireguardInterface(const std::string &name) {
    setName(name);
  }

  WireguardInterface::~WireguardInterface() {}

  lyd_node *WireguardInterface::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      return nullptr;
    }

    // Create interface node using ietf-interfaces schema
    lyd_node *interfaces = nullptr;
    lyd_node *interface = nullptr;

    // Create the interfaces container
    if (lyd_new_path(nullptr, ctx, "/ietf-interfaces:interfaces", nullptr, 0,
                     &interfaces) != LY_SUCCESS) {
      return nullptr;
    }

    // Create the interface list entry
    std::string name = getName();
    std::string path =
        "/ietf-interfaces:interfaces/interface[name='" + name + "']";
    if (lyd_new_path(interfaces, ctx, path.c_str(), nullptr, 0, &interface) !=
        LY_SUCCESS) {
      lyd_free_tree(interfaces);
      return nullptr;
    }

    // Set interface type
    lyd_node *typeNode = nullptr;
    std::string typePath = path + "/type";
    if (lyd_new_path(interface, ctx, typePath.c_str(), "iana-if-type:tunnel", 0,
                     &typeNode) != LY_SUCCESS) {
      lyd_free_tree(interfaces);
      return nullptr;
    }

    // Set interface enabled state
    lyd_node *enabledNode = nullptr;
    std::string enabledPath = path + "/enabled";
    std::string enabled = isUp() ? "true" : "false";
    if (lyd_new_path(interface, ctx, enabledPath.c_str(), enabled.c_str(), 0,
                     &enabledNode) != LY_SUCCESS) {
      lyd_free_tree(interfaces);
      return nullptr;
    }

    // TODO: Add WireGuard-specific YANG extensions (private key, peers, etc.)

    return interfaces;
  }

  WireguardInterface
  WireguardInterface::fromYang([[maybe_unused]] const ly_ctx *ctx,
                               [[maybe_unused]] const lyd_node *node) {
    // TODO: Implement YANG deserialization for WireGuard interfaces
    return WireguardInterface();
  }

} // namespace netd::shared::interface
