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

#include <freebsd/include/interface/tap.hpp>
#include <shared/include/logger.hpp>

#include <net/if.h>
#include <net/if_var.h>
#include <net/if_tap.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

namespace netd {
namespace freebsd {
namespace interface {

TapInterface::TapInterface()
    : netd::TapInterface(),
      name_(""),
      tapUnit_(-1),
      tapMode_("tap"),
      socket_(-1) {
}

TapInterface::TapInterface(const std::string& name)
    : netd::TapInterface(),
      name_(name),
      tapUnit_(-1),
      tapMode_("tap"),
      socket_(-1) {
}

TapInterface::~TapInterface() {
    closeSocket();
}

bool TapInterface::createInterface() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        logger.error("Failed to open socket for creating TAP interface");
        return false;
    }
    
    // Use SIOCIFCREATE to create TAP interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
    
    if (ioctl(socket_, SIOCIFCREATE, &ifr) < 0) {
        logger.error("Failed to create TAP interface " + name_ + ": " + std::strerror(errno));
        closeSocket();
        return false;
    }
    
    logger.info("Created TAP interface " + name_);
    return true;
}

bool TapInterface::destroyInterface() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        logger.error("Failed to open socket for destroying TAP interface");
        return false;
    }
    
    // Use SIOCIFDESTROY to destroy TAP interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
    
    if (ioctl(socket_, SIOCIFDESTROY, &ifr) < 0) {
        logger.error("Failed to destroy TAP interface " + name_ + ": " + std::strerror(errno));
        closeSocket();
        return false;
    }
    
    logger.info("Destroyed TAP interface " + name_);
    return true;
}

bool TapInterface::loadFromSystem() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        return false;
    }
    
    if (!getTapInfo()) {
        closeSocket();
        return false;
    }
    
    closeSocket();
    logger.info("Loaded TAP interface information from system: " + name_);
    return true;
}

bool TapInterface::applyToSystem() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        return false;
    }
    
    if (!setTapInfo()) {
        closeSocket();
        return false;
    }
    
    closeSocket();
    logger.info("Applied TAP interface configuration to system: " + name_);
    return true;
}

bool TapInterface::setTapUnit(int unit) {
    tapUnit_ = unit;
    return true;
}

int TapInterface::getTapUnit() const {
    return tapUnit_;
}

bool TapInterface::setTapMode(const std::string& mode) {
    tapMode_ = mode;
    return true;
}

std::string TapInterface::getTapMode() const {
    return tapMode_;
}

TapInterface::operator netd::TapInterface() const {
    // Cast to shared interface - we inherit from it so this is safe
    return static_cast<const netd::TapInterface&>(*this);
}

bool TapInterface::openSocket() {
    if (socket_ >= 0) {
        return true; // Already open
    }

    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) {
        return false;
    }

    return true;
}

void TapInterface::closeSocket() {
    if (socket_ >= 0) {
        close(socket_);
        socket_ = -1;
    }
}

bool TapInterface::getTapInfo() {
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

    // Get TAP-specific information
    // TODO: Use TAP-specific ioctls to get TAP details

    return true;
}

bool TapInterface::setTapInfo() const {
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

    // Set TAP-specific information
    // TODO: Use TAP-specific ioctls to set TAP details

    return true;
}

} // namespace interface
} // namespace freebsd
} // namespace netd
