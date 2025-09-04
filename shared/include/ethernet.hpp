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

#ifndef NETD_ETHERNET_HPP
#define NETD_ETHERNET_HPP

#include <shared/include/interface/base/ether.hpp>

namespace netd {

class Ethernet : public interface::base::Ether {
public:
    Ethernet() = default;
    virtual ~Ethernet() = default;

    // Implement all pure virtual methods from base::Ether
    bool addAddress(const std::shared_ptr<Address>& address) override;
    bool removeAddress(const std::shared_ptr<Address>& address) override;
    std::vector<std::shared_ptr<Address>> getAddresses() const override;
    bool addGroup(const std::string& group) override;
    bool removeGroup(const std::string& group) override;
    std::vector<std::string> getGroups() const override;
    bool setMTU(uint16_t mtu) override;
    uint16_t getMTU() const override;
    bool setFlags(uint32_t flags) override;
    uint32_t getFlags() const override;
    bool up() override;
    bool down() override;
    bool isUp() const override;
    bool setVRF(uint32_t vrfId) override;
    uint32_t getVRF() const override;
};

} // namespace netd

#endif // NETD_ETHERNET_HPP
