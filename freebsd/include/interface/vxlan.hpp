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

#ifndef NETD_FREEBSD_INTERFACE_VXLAN_HPP
#define NETD_FREEBSD_INTERFACE_VXLAN_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <shared/include/base/serialization.hpp>
#include <shared/include/interface/vxlan.hpp>

namespace netd::freebsd::interface {

  class VxlanInterface : public netd::shared::interface::VxlanInterface {
  public:
    VxlanInterface();
    explicit VxlanInterface(const std::string &name);
    virtual ~VxlanInterface();

    // Interface name
    std::string getName() const { return name_; }

    // FreeBSD-specific operations
    bool createInterface();
    bool destroyInterface();
    bool loadFromSystem();
    bool applyToSystem();

    // VXLAN-specific configuration
    bool setVni(uint32_t vni);
    uint32_t getVni() const;
    bool setLocalEndpoint(const std::string &endpoint);
    std::string getLocalEndpoint() const;
    bool setRemoteEndpoint(const std::string &endpoint);
    std::string getRemoteEndpoint() const;
    bool setUdpPort(uint16_t port);
    uint16_t getUdpPort() const;

    // Statistics and information
    std::string getType() const { return "vxlan"; }

    // Conversion to shared interface for serialization
    operator netd::shared::interface::VxlanInterface() const;

  private:
    // Interface name
    std::string name_;

    // VXLAN-specific members
    uint32_t vni_;
    std::string localEndpoint_;
    std::string remoteEndpoint_;
    uint16_t udpPort_;

    // FreeBSD system interface
    int socket_;

    // Helper methods
    bool openSocket();
    void closeSocket();
    bool getVxlanInfo();
    bool setVxlanInfo() const;
  };

} // namespace netd::freebsd::interface

#endif // NETD_FREEBSD_INTERFACE_VXLAN_HPP
