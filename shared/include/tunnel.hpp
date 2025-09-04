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

#ifndef NETD_TUNNEL_HPP
#define NETD_TUNNEL_HPP

#include <shared/include/interface/base/tunnel.hpp>

namespace netd {

class Tunnel : public interface::base::Tunnel {
public:
    Tunnel() = default;
    virtual ~Tunnel() = default;

    // Implement all virtual methods from base::Tunnel
    bool setLocalAddr(const std::shared_ptr<Address>& localAddr) override;
    std::shared_ptr<Address> getLocalAddr() const override;
    bool setRemoteAddr(const std::shared_ptr<Address>& remoteAddr) override;
    std::shared_ptr<Address> getRemoteAddr() const override;
    bool setTunnelVRF(uint32_t vrfId) override;
    uint32_t getTunnelVRF() const override;
    bool setTunnelMTU(uint16_t mtu) override;
    uint16_t getTunnelMTU() const override;
    bool isTunnelEstablished() const override;
    bool establishTunnel() override;
    bool teardownTunnel() override;
};

} // namespace netd

#endif // NETD_TUNNEL_HPP
