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

#ifndef NETD_INTERFACE_WIREGUARD_HPP
#define NETD_INTERFACE_WIREGUARD_HPP

#include <shared/include/interface/base/ether.hpp>
#include <shared/include/tunnel.hpp>
#include <shared/include/base/serialization.hpp>
#include <string>
#include <cstdint>
#include <memory>
#include <vector>

namespace netd {

class WireguardInterface : public interface::base::Ether, public Tunnel, public base::Serialization<WireguardInterface> {
public:
    WireguardInterface();
    explicit WireguardInterface(const std::string& name);
    virtual ~WireguardInterface();

    // WireGuard-specific configuration
    virtual bool setPrivateKey(const std::string& privateKey) { return false; }
    virtual std::string getPrivateKey() const { return ""; }
    virtual bool setListenPort(uint16_t port) { return false; }
    virtual uint16_t getListenPort() const { return 51820; }
    virtual bool addPeer(const std::string& publicKey, const std::string& endpoint) { return false; }
    virtual bool removePeer(const std::string& publicKey) { return false; }

    // YANG serialization
    lyd_node* toYang(ly_ctx* ctx) const override;
    static WireguardInterface fromYang(const ly_ctx* ctx, const lyd_node* node);
};

} // namespace netd

#endif // NETD_INTERFACE_WIREGUARD_HPP
