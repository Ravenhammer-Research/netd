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

#ifndef NETD_INTERFACE_BASE_ETHER_HPP
#define NETD_INTERFACE_BASE_ETHER_HPP

#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <shared/include/address.hpp>

namespace netd {
namespace interface {
namespace base {

class Ether {
public:
    virtual ~Ether() = default;

    // Address management
    virtual bool addAddress(const std::shared_ptr<netd::Address>& address) = 0;
    virtual bool removeAddress(const std::shared_ptr<netd::Address>& address) = 0;
    virtual std::vector<std::shared_ptr<netd::Address>> getAddresses() const = 0;

    // Group management
    virtual bool addGroup(const std::string& group) = 0;
    virtual bool removeGroup(const std::string& group) = 0;
    virtual std::vector<std::string> getGroups() const = 0;

    // Interface configuration
    virtual bool setMTU(uint16_t mtu) = 0;
    virtual uint16_t getMTU() const = 0;
    virtual bool setFlags(uint32_t flags) = 0;
    virtual uint32_t getFlags() const = 0;
    virtual bool up() = 0;
    virtual bool down() = 0;
    virtual bool isUp() const = 0;

    // VRF/FIB management
    virtual bool setVRF(uint32_t vrfId) = 0;
    virtual uint32_t getVRF() const = 0;

    // Basic interface operations
    virtual bool create() { return false; }
    virtual bool destroy() { return false; }

    // Ethernet-specific configuration
    virtual bool setDuplex(const std::string& duplex) { return false; }
    virtual std::string getDuplex() const { return "auto"; }
    virtual bool setSpeed(uint32_t speed) { return false; }
    virtual uint32_t getSpeed() const { return 0; }
    virtual bool setAutoNegotiation(bool enabled) { return false; }
    virtual bool isAutoNegotiationEnabled() const { return true; }
    virtual bool setFlowControl(bool enabled) { return false; }
    virtual bool isFlowControlEnabled() const { return false; }

    // Statistics and information
    virtual std::string getName() const { return ""; }
    virtual std::string getType() const { return "ethernet"; }

protected:
    std::vector<std::shared_ptr<netd::Address>> addresses_;
    std::vector<std::string> groups_;
    uint16_t mtu_{1500};
    uint32_t flags_{0};
    bool up_{false};
    uint32_t vrfId_{0};
};

} // namespace base
} // namespace interface
} // namespace netd

#endif // NETD_INTERFACE_BASE_ETHER_HPP
