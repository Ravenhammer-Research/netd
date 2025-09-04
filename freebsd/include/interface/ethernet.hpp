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

#ifndef NETD_FREEBSD_INTERFACE_ETHERNET_HPP
#define NETD_FREEBSD_INTERFACE_ETHERNET_HPP

#include <string>
#include <cstdint>
#include <memory>
#include <vector>

#include <shared/include/ethernet.hpp>
#include <shared/include/base/serialization.hpp>

namespace netd {
namespace freebsd {
namespace interface {

class EthernetInterface : public netd::Ethernet, 
                         public netd::base::Serialization<EthernetInterface> {
public:
    EthernetInterface();
    explicit EthernetInterface(const std::string& name);
    virtual ~EthernetInterface();

    // Interface name
    std::string getName() const { return name_; }

    // FreeBSD-specific operations
    bool createInterface();
    bool destroyInterface();
    bool loadFromSystem();
    bool applyToSystem();

    // Ethernet-specific configuration
    bool setDuplex(const std::string& duplex);
    std::string getDuplex() const;
    bool setSpeed(uint32_t speed);
    uint32_t getSpeed() const;
    bool setAutoNegotiation(bool enabled);
    bool isAutoNegotiationEnabled() const;
    bool setFlowControl(bool enabled);
    bool isFlowControlEnabled() const;

    // Statistics and information
    std::string getType() const { return "ethernet"; }

    // Serialization
    lyd_node* toYang() const override;
    static EthernetInterface fromYang(const lyd_node* node);

private:
    // Interface name
    std::string name_;
    
    // Ethernet-specific members
    std::string duplex_;
    uint32_t speed_;
    bool autoNegotiation_;
    bool flowControl_;
    
    // FreeBSD system interface
    int socket_;
    
    // Helper methods
    bool openSocket();
    void closeSocket();
    bool getInterfaceInfo();
    bool setInterfaceInfo() const;
};

} // namespace interface
} // namespace freebsd
} // namespace netd

#endif // NETD_FREEBSD_INTERFACE_ETHERNET_HPP
