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

#include <shared/include/interface/base/tunnel.hpp>

namespace netd::shared::interface::base {

    // Provide implementations for pure virtual methods to avoid linker errors
    // These are placeholder implementations that should be overridden by derived classes

    bool Tunnel::setLocalAddr(const std::shared_ptr<netd::shared::Address>& localAddr) {
        if (localAddr && localAddr->isValid()) {
            localAddr_ = localAddr;
            return true;
        }
        return false;
    }

    std::shared_ptr<netd::shared::Address> Tunnel::getLocalAddr() const {
        return localAddr_;
    }

    bool Tunnel::setRemoteAddr(const std::shared_ptr<netd::shared::Address>& remoteAddr) {
        if (remoteAddr && remoteAddr->isValid()) {
            remoteAddr_ = remoteAddr;
            return true;
        }
        return false;
    }

    std::shared_ptr<netd::shared::Address> Tunnel::getRemoteAddr() const {
        return remoteAddr_;
    }

    bool Tunnel::setTunnelVRF(uint32_t vrfId) {
        tunnelVrfId_ = vrfId;
        return true;
    }

    uint32_t Tunnel::getTunnelVRF() const {
        return tunnelVrfId_;
    }

    bool Tunnel::setTunnelMTU(uint16_t mtu) {
        if (mtu >= 68 && mtu <= 9000) { // Reasonable MTU range
            tunnelMtu_ = mtu;
            return true;
        }
        return false;
    }

    uint16_t Tunnel::getTunnelMTU() const {
        return tunnelMtu_;
    }

    bool Tunnel::isTunnelEstablished() const {
        return tunnelEstablished_;
    }

    bool Tunnel::establishTunnel() {
        // Basic validation that both addresses are set
        if (localAddr_ && remoteAddr_) {
            tunnelEstablished_ = true;
            return true;
        }
        return false;
    }

    bool Tunnel::teardownTunnel() {
        tunnelEstablished_ = false;
        return true;
    }

} // namespace netd::interface::base
