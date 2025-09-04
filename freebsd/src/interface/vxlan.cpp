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

#include <freebsd/include/interface/vxlan.hpp>
#include <shared/include/logger.hpp>

#include <net/if.h>
#include <net/if_var.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

namespace netd {
namespace freebsd {
namespace interface {

VxlanInterface::VxlanInterface()
    : netd::VxlanInterface(),
      name_(""),
      vni_(0),
      localEndpoint_(""),
      remoteEndpoint_(""),
      udpPort_(4789),
      socket_(-1) {
}

VxlanInterface::VxlanInterface(const std::string& name)
    : netd::VxlanInterface(),
      name_(name),
      vni_(0),
      localEndpoint_(""),
      remoteEndpoint_(""),
      udpPort_(4789),
      socket_(-1) {
}

VxlanInterface::~VxlanInterface() {
    closeSocket();
}

bool VxlanInterface::createInterface() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        logger.error("Failed to open socket for creating VXLAN interface");
        return false;
    }
    
    // Use SIOCIFCREATE to create VXLAN interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
    
    if (ioctl(socket_, SIOCIFCREATE, &ifr) < 0) {
        logger.error("Failed to create VXLAN interface " + name_ + ": " + std::strerror(errno));
        closeSocket();
        return false;
    }
    
    logger.info("Created VXLAN interface " + name_);
    return true;
}

bool VxlanInterface::destroyInterface() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        logger.error("Failed to open socket for destroying VXLAN interface");
        return false;
    }
    
    // Use SIOCIFDESTROY to destroy VXLAN interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
    
    if (ioctl(socket_, SIOCIFDESTROY, &ifr) < 0) {
        logger.error("Failed to destroy VXLAN interface " + name_ + ": " + std::strerror(errno));
        closeSocket();
        return false;
    }
    
    logger.info("Destroyed VXLAN interface " + name_);
    return true;
}

bool VxlanInterface::loadFromSystem() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        return false;
    }
    
    if (!getVxlanInfo()) {
        closeSocket();
        return false;
    }
    
    closeSocket();
    logger.info("Loaded VXLAN interface information from system: " + name_);
    return true;
}

bool VxlanInterface::applyToSystem() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        return false;
    }
    
    if (!setVxlanInfo()) {
        closeSocket();
        return false;
    }
    
    closeSocket();
    logger.info("Applied VXLAN interface configuration to system: " + name_);
    return true;
}

bool VxlanInterface::setVni(uint32_t vni) {
    vni_ = vni;
    return true;
}

uint32_t VxlanInterface::getVni() const {
    return vni_;
}

bool VxlanInterface::setLocalEndpoint(const std::string& endpoint) {
    localEndpoint_ = endpoint;
    return true;
}

std::string VxlanInterface::getLocalEndpoint() const {
    return localEndpoint_;
}

bool VxlanInterface::setRemoteEndpoint(const std::string& endpoint) {
    remoteEndpoint_ = endpoint;
    return true;
}

std::string VxlanInterface::getRemoteEndpoint() const {
    return remoteEndpoint_;
}

bool VxlanInterface::setUdpPort(uint16_t port) {
    udpPort_ = port;
    return true;
}

uint16_t VxlanInterface::getUdpPort() const {
    return udpPort_;
}

VxlanInterface::operator netd::VxlanInterface() const {
    // Cast to shared interface - we inherit from it so this is safe
    return static_cast<const netd::VxlanInterface&>(*this);
}

bool VxlanInterface::openSocket() {
    if (socket_ >= 0) {
        return true; // Already open
    }

    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) {
        return false;
    }

    return true;
}

void VxlanInterface::closeSocket() {
    if (socket_ >= 0) {
        close(socket_);
        socket_ = -1;
    }
}

bool VxlanInterface::getVxlanInfo() {
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

    // Get VXLAN-specific information
    // TODO: Use VXLAN-specific ioctls to get VXLAN details

    return true;
}

bool VxlanInterface::setVxlanInfo() const {
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

    // Set VXLAN-specific information
    // TODO: Use VXLAN-specific ioctls to set VXLAN details

    return true;
}

} // namespace interface
} // namespace freebsd
} // namespace netd