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

#ifndef NETD_FREEBSD_INTERFACE_BRIDGE_HPP
#define NETD_FREEBSD_INTERFACE_BRIDGE_HPP

#include <memory>
#include <shared/include/base/serialization.hpp>
#include <shared/include/interface/base/ether.hpp>
#include <shared/include/interface/base/master.hpp>
#include <shared/include/interface/bridge.hpp>
#include <string>
#include <vector>

namespace netd::freebsd::interface {

  class BridgeInterface : public netd::shared::interface::BridgeInterface {
  public:
    BridgeInterface();
    explicit BridgeInterface(const std::string &name);
    virtual ~BridgeInterface();

    // Interface name
    std::string getName() const { return name_; }

    // Bridge-specific operations
    bool addMember(const std::string &memberName);
    bool removeMember(const std::string &memberName);
    std::vector<std::string> getMembers() const;

    // Bridge configuration
    bool setStpEnabled(bool enabled);
    bool isStpEnabled() const;
    bool setMaxAge(uint16_t maxAge);
    uint16_t getMaxAge() const;
    bool setHelloTime(uint16_t helloTime);
    uint16_t getHelloTime() const;
    bool setForwardDelay(uint16_t forwardDelay);
    uint16_t getForwardDelay() const;

    // FreeBSD-specific operations
    bool createInterface();
    bool destroyInterface();
    bool loadFromSystem();
    bool applyToSystem() const;

    // Static interface discovery functions
    static std::vector<std::unique_ptr<BridgeInterface>>
    getAllBridgeInterfaces();

  private:
    // Interface name
    std::string name_;

    // FreeBSD bridge-specific members
    bool stpEnabled_;
    uint16_t maxAge_;
    uint16_t helloTime_;
    uint16_t forwardDelay_;
    std::vector<std::string> members_;

    // FreeBSD system interface
    int bridgeSocket_;

    // Helper methods
    bool openBridgeSocket();
    void closeBridgeSocket();
    bool getBridgeInfo();
    bool setBridgeInfo() const;
    bool openBridgeSocket() const;
  };

} // namespace netd::freebsd::interface

#endif // NETD_FREEBSD_INTERFACE_BRIDGE_HPP
