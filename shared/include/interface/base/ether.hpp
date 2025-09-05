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

#include <cstdint>
#include <memory>
#include <shared/include/address.hpp>
#include <string>
#include <vector>

namespace netd::shared::interface::base {

  class Ether {
  public:
    virtual ~Ether() = default;

    // Address management
    virtual bool
    addAddress(const std::shared_ptr<netd::shared::Address> &address);
    virtual bool
    removeAddress(const std::shared_ptr<netd::shared::Address> &address);
    virtual std::vector<std::shared_ptr<netd::shared::Address>>
    getAddresses() const;

    // Group management
    virtual bool addGroup(const std::string &group);
    virtual bool removeGroup(const std::string &group);
    virtual std::vector<std::string> getGroups() const;

    // Interface configuration
    virtual bool setMTU(uint16_t mtu);
    virtual uint16_t getMTU() const;
    virtual bool setFlags(uint32_t flags);
    virtual uint32_t getFlags() const;
    virtual bool up();
    virtual bool down();
    virtual bool isUp() const;

    // VRF/FIB management
    virtual bool setVRF(uint32_t vrfId);
    virtual uint32_t getVRF() const;

    // Basic interface operations
    virtual bool create() { return false; }
    virtual bool destroy() { return false; }

    // Ethernet-specific configuration
    virtual bool setDuplex([[maybe_unused]] const std::string &duplex) {
      return false;
    }
    virtual std::string getDuplex() const { return "auto"; }
    virtual bool setSpeed([[maybe_unused]] uint32_t speed) { return false; }
    virtual uint32_t getSpeed() const { return 0; }
    virtual bool setAutoNegotiation([[maybe_unused]] bool enabled) {
      return false;
    }
    virtual bool isAutoNegotiationEnabled() const { return true; }
    virtual bool setFlowControl([[maybe_unused]] bool enabled) { return false; }
    virtual bool isFlowControlEnabled() const { return false; }

    // Statistics and information
    virtual std::string getName() const;
    virtual std::string getType() const { return "ethernet"; }
    void setName(const std::string &name);

    // Static interface discovery function
    static std::vector<std::unique_ptr<Ether>> getAllInterfaces();

  protected:
    std::vector<std::shared_ptr<netd::shared::Address>> addresses_;
    std::vector<std::string> groups_;
    uint16_t mtu_{1500};
    uint32_t flags_{0};
    std::string name_;
    bool up_{false};
    uint32_t vrfId_{0};
  };

} // namespace netd::shared::interface::base

#endif // NETD_INTERFACE_BASE_ETHER_HPP