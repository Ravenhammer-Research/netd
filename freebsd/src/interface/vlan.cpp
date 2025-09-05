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

#include <freebsd/include/interface/vlan.hpp>
#include <shared/include/logger.hpp>

#include <net/if.h>
#include <net/if_var.h>
#include <net/if_vlan_var.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

namespace netd::freebsd::interface {

    VlanInterface::VlanInterface()
        : netd::shared::interface::VlanInterface(),
        name_(""),
        vlanId_(0),
        parentInterface_(""),
        vlanProtocol_("8021q"),
        socket_(-1) {
    }

    VlanInterface::VlanInterface(const std::string& name)
        : netd::shared::interface::VlanInterface(),
        name_(name),
        vlanId_(0),
        parentInterface_(""),
        vlanProtocol_("8021q"),
        socket_(-1) {
    }

    VlanInterface::~VlanInterface() {
        closeSocket();
    }

    bool VlanInterface::createInterface() {
        auto& logger = shared::Logger::getInstance();
        
        if (parentInterface_.empty()) {
            logger.error("Cannot create VLAN interface without parent interface");
            return false;
        }
        
        if (!openSocket()) {
            logger.error("Failed to open socket for creating VLAN interface");
            return false;
        }
        
        // Use SIOCIFCREATE to create VLAN interface
        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
        
        if (ioctl(socket_, SIOCIFCREATE, &ifr) < 0) {
            logger.error("Failed to create VLAN interface " + name_ + ": " + std::strerror(errno));
            closeSocket();
            return false;
        }
        
        logger.info("Created VLAN interface " + name_ + " on " + parentInterface_);
        return true;
    }

    bool VlanInterface::destroyInterface() {
        auto& logger = shared::Logger::getInstance();
        
        if (!openSocket()) {
            logger.error("Failed to open socket for destroying VLAN interface");
            return false;
        }
        
        // Use SIOCIFDESTROY to destroy VLAN interface
        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
        
        if (ioctl(socket_, SIOCIFDESTROY, &ifr) < 0) {
            logger.error("Failed to destroy VLAN interface " + name_ + ": " + std::strerror(errno));
            closeSocket();
            return false;
        }
        
        logger.info("Destroyed VLAN interface " + name_);
        return true;
    }

    bool VlanInterface::loadFromSystem() {
        auto& logger = shared::Logger::getInstance();
        
        if (!openSocket()) {
            return false;
        }
        
        if (!getVlanInfo()) {
            closeSocket();
            return false;
        }
        
        closeSocket();
        logger.info("Loaded VLAN interface information from system: " + name_);
        return true;
    }

    bool VlanInterface::applyToSystem() {
        auto& logger = shared::Logger::getInstance();
        
        if (!openSocket()) {
            return false;
        }
        
        if (!setVlanInfo()) {
            closeSocket();
            return false;
        }
        
        closeSocket();
        logger.info("Applied VLAN interface configuration to system: " + name_);
        return true;
    }

    bool VlanInterface::setVlanId(uint16_t vlanId) {
        vlanId_ = vlanId;
        return true;
    }

    uint16_t VlanInterface::getVlanId() const {
        return vlanId_;
    }

    bool VlanInterface::setParentInterface(const std::string& parentInterface) {
        parentInterface_ = parentInterface;
        return true;
    }

    std::string VlanInterface::getParentInterface() const {
        return parentInterface_;
    }

    bool VlanInterface::setVlanProtocol(const std::string& protocol) {
        vlanProtocol_ = protocol;
        return true;
    }

    std::string VlanInterface::getVlanProtocol() const {
        return vlanProtocol_;
    }

    VlanInterface::operator netd::shared::interface::VlanInterface() const {
        // Cast to shared interface - we inherit from it so this is safe
        return static_cast<const netd::shared::interface::VlanInterface&>(*this);
    }

    bool VlanInterface::openSocket() {
        if (socket_ >= 0) {
            return true; // Already open
        }

        socket_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_ < 0) {
            return false;
        }

        return true;
    }

    void VlanInterface::closeSocket() {
        if (socket_ >= 0) {
            close(socket_);
            socket_ = -1;
        }
    }

    bool VlanInterface::getVlanInfo() {
        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);

        // Get interface flags
        if (ioctl(socket_, SIOCGIFFLAGS, &ifr) < 0) {
            return false;
        }

        // Get interface MTU
        if (ioctl(socket_, SIOCGIFMTU, &ifr) < 0) {
            return false;
        }

        // Get VLAN-specific information
        // TODO: Use SIOCGIFVLAN to get VLAN details

        return true;
    }

    bool VlanInterface::setVlanInfo() const {
        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);

        // Set interface flags
        if (ioctl(socket_, SIOCSIFFLAGS, &ifr) < 0) {
            return false;
        }

        // Set interface MTU
        if (ioctl(socket_, SIOCSIFMTU, &ifr) < 0) {
            return false;
        }

        // Set VLAN-specific information
        // TODO: Use SIOCSIFVLAN to set VLAN details

        return true;
    }

} // namespace netd::freebsd::interface
