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
#include <shared/include/vrf.hpp>
#include <shared/include/yang.hpp>

namespace netd::shared {

  VRF::VRF(uint32_t id, const std::string &name, bool active)
      : id_(id), name_(name), active_(active) {}

  lyd_node *VRF::toYang(ly_ctx *ctx) const {
    // TODO: Implement YANG serialization for VRF
    if (!ctx) {
      return nullptr;
    }

    // Placeholder implementation - needs actual YANG node creation
    return nullptr;
  }

  VRF VRF::fromYang(const ly_ctx *ctx, const lyd_node *node) {
    // TODO: Implement YANG deserialization for VRF
    if (!ctx || !node) {
      return VRF();
    }

    // Placeholder implementation - needs actual YANG node parsing
    return VRF();
  }

  uint32_t VRF::getId() const { return id_; }

  std::string VRF::getName() const { return name_; }

  bool VRF::isActive() const { return active_; }

  void VRF::setId(uint32_t id) { id_ = id; }

  void VRF::setName(const std::string &name) { name_ = name; }

  void VRF::setActive(bool active) { active_ = active; }

} // namespace netd::shared
