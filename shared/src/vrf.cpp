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

#include <shared/include/vrf.hpp>
#include <shared/include/yang.hpp>
#include <libyang/tree_data.h>

namespace netd {

// Shared VRF implementation with YANG serialization
class VRF : public VRFAbstract {
public:
    VRF() = default;
    virtual ~VRF() = default;

    // Implement YANG serialization
    lyd_node* toYang(ly_ctx* ctx) const override {
        
        if (!ctx) {
            return nullptr;
        }
        
        // Create network-instance node using ietf-network-instance schema
        lyd_node* networkInstances = nullptr;
        lyd_node* networkInstance = nullptr;
        
        // Create the network-instances container
        if (lyd_new_path(nullptr, ctx, "/ietf-network-instance:network-instances", nullptr, 0, &networkInstances) != LY_SUCCESS) {
            return nullptr;
        }
        
        // Create the network-instance list entry
        std::string name = "default"; // TODO: Get actual VRF name
        std::string path = "/ietf-network-instance:network-instances/network-instance[name='" + name + "']";
        if (lyd_new_path(networkInstances, ctx, path.c_str(), nullptr, 0, &networkInstance) != LY_SUCCESS) {
            lyd_free_tree(networkInstances);
            return nullptr;
        }
        
        // Set network-instance type
        lyd_node* typeNode = nullptr;
        std::string typePath = path + "/type";
        if (lyd_new_path(networkInstance, ctx, typePath.c_str(), "ietf-network-instance:default-network-instance", 0, &typeNode) != LY_SUCCESS) {
            lyd_free_tree(networkInstances);
            return nullptr;
        }
        
        // Set network-instance enabled state
        lyd_node* enabledNode = nullptr;
        std::string enabledPath = path + "/enabled";
        std::string enabled = "true"; // TODO: Get actual VRF state
        if (lyd_new_path(networkInstance, ctx, enabledPath.c_str(), enabled.c_str(), 0, &enabledNode) != LY_SUCCESS) {
            lyd_free_tree(networkInstances);
            return nullptr;
        }
        
        // TODO: Add VRF-specific YANG extensions (interfaces, routing, etc.)
        
        return networkInstances;
    }
    
    static VRF fromYang(const ly_ctx* ctx, const lyd_node* node) {
        // TODO: Implement YANG deserialization for VRF
        return VRF();
    }

    // VRF properties
    uint32_t getId() const override { return 0; } // TODO: Get actual VRF ID
    std::string getName() const override { return "default"; } // TODO: Get actual VRF name
    bool isActive() const override { return true; } // TODO: Get actual VRF state
};

} // namespace netd
