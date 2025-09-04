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

#include <freebsd/include/interface/tun.hpp>
#include <shared/include/logger.hpp>

#include <net/if.h>
#include <net/if_var.h>
#include <net/if_tun.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

namespace netd {
namespace freebsd {
namespace interface {

TunInterface::TunInterface()
    : netd::TunInterface(),
      name_(""),
      tunUnit_(-1),
      tunMode_("tun"),
      socket_(-1) {
}

TunInterface::TunInterface(const std::string& name)
    : netd::TunInterface(),
      name_(name),
      tunUnit_(-1),
      tunMode_("tun"),
      socket_(-1) {
}

TunInterface::~TunInterface() {
    closeSocket();
}

bool TunInterface::createInterface() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        logger.error("Failed to open socket for creating TUN interface");
        return false;
    }
    
    // Use SIOCIFCREATE to create TUN interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
    
    if (ioctl(socket_, SIOCIFCREATE, &ifr) < 0) {
        logger.error("Failed to create TUN interface " + name_ + ": " + std::strerror(errno));
        closeSocket();
        return false;
    }
    
    logger.info("Created TUN interface " + name_);
    return true;
}

bool TunInterface::destroyInterface() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        logger.error("Failed to open socket for destroying TUN interface");
        return false;
    }
    
    // Use SIOCIFDESTROY to destroy TUN interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
    
    if (ioctl(socket_, SIOCIFDESTROY, &ifr) < 0) {
        logger.error("Failed to destroy TUN interface " + name_ + ": " + std::strerror(errno));
        closeSocket();
        return false;
    }
    
    logger.info("Destroyed TUN interface " + name_);
    return true;
}

bool TunInterface::loadFromSystem() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        return false;
    }
    
    if (!getTunInfo()) {
        closeSocket();
        return false;
    }
    
    closeSocket();
    logger.info("Loaded TUN interface information from system: " + name_);
    return true;
}

bool TunInterface::applyToSystem() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        return false;
    }
    
    if (!setTunInfo()) {
        closeSocket();
        return false;
    }
    
    closeSocket();
    logger.info("Applied TUN interface configuration to system: " + name_);
    return true;
}

bool TunInterface::setTunUnit(int unit) {
    tunUnit_ = unit;
    return true;
}

int TunInterface::getTunUnit() const {
    return tunUnit_;
}

bool TunInterface::setTunMode(const std::string& mode) {
    tunMode_ = mode;
    return true;
}

std::string TunInterface::getTunMode() const {
    return tunMode_;
}

TunInterface::operator netd::TunInterface() const {
    // Cast to shared interface - we inherit from it so this is safe
    return static_cast<const netd::TunInterface&>(*this);
}

bool TunInterface::openSocket() {
    if (socket_ >= 0) {
        return true; // Already open
    }

    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) {
        return false;
    }

    return true;
}

void TunInterface::closeSocket() {
    if (socket_ >= 0) {
        close(socket_);
        socket_ = -1;
    }
}

bool TunInterface::getTunInfo() {
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

    // Get TUN-specific information
    // TODO: Use TUN-specific ioctls to get TUN details

    return true;
}

bool TunInterface::setTunInfo() const {
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

    // Set TUN-specific information
    // TODO: Use TUN-specific ioctls to set TUN details

    return true;
}

} // namespace interface
} // namespace freebsd
} // namespace netd
