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

#ifndef NETD_INTERFACE_ETHERNET_HPP
#define NETD_INTERFACE_ETHERNET_HPP

#include <shared/include/ethernet.hpp>
#include <shared/include/base/serialization.hpp>
#include <string>
#include <cstdint>
#include <memory>

namespace netd {

class EthernetInterface : public Ethernet, public base::Serialization<EthernetInterface> {
public:
    EthernetInterface();
    explicit EthernetInterface(const std::string& name);
    virtual ~EthernetInterface();

    // Ethernet-specific configuration
    virtual bool setDuplex(const std::string& duplex) { return false; }
    virtual std::string getDuplex() const { return "auto"; }
    virtual bool setSpeed(uint32_t speed) { return false; }
    virtual uint32_t getSpeed() const { return 0; }
    virtual bool setAutoNegotiation(bool enabled) { return false; }
    virtual bool isAutoNegotiationEnabled() const { return true; }
    virtual bool setFlowControl(bool enabled) { return false; }
    virtual bool isFlowControlEnabled() const { return false; }

    // YANG serialization
    lyd_node* toYang(ly_ctx* ctx) const override;
    static EthernetInterface fromYang(const ly_ctx* ctx, const lyd_node* node);
};

} // namespace netd

#endif // NETD_INTERFACE_ETHERNET_HPP
