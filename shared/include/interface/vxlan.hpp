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

#ifndef NETD_INTERFACE_VXLAN_HPP
#define NETD_INTERFACE_VXLAN_HPP

#include <shared/include/interface/base/ether.hpp>
#include <shared/include/interface/base/tunnel.hpp>
#include <shared/include/base/serialization.hpp>
#include <string>
#include <cstdint>
#include <memory>

namespace netd::shared::interface {

    class VxlanInterface : public netd::shared::interface::base::Ether, 
        public netd::shared::interface::base::Tunnel, 
        public netd::shared::base::Serialization<VxlanInterface> {
    public:
        VxlanInterface();
        explicit VxlanInterface(const std::string& name);
        virtual ~VxlanInterface();

        // VXLAN-specific configuration
        virtual bool setVni(uint32_t vni) { return false; }
        virtual uint32_t getVni() const { return 0; }
        virtual bool setLocalEndpoint(const std::string& endpoint) { return false; }
        virtual std::string getLocalEndpoint() const { return ""; }
        virtual bool setRemoteEndpoint(const std::string& endpoint) { return false; }
        virtual std::string getRemoteEndpoint() const { return ""; }
        virtual bool setUdpPort(uint16_t port) { return false; }
        virtual uint16_t getUdpPort() const { return 4789; }

        // YANG serialization
        lyd_node* toYang(ly_ctx* ctx) const override;
        static VxlanInterface fromYang(const ly_ctx* ctx, const lyd_node* node);
    };

} // namespace netd::shared::interface

#endif // NETD_INTERFACE_VXLAN_HPP
