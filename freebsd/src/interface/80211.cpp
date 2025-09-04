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

#include <freebsd/include/interface/80211.hpp>
#include <shared/include/logger.hpp>

#include <net/if.h>
#include <net/if_var.h>
#include <net80211/ieee80211_ioctl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

namespace netd {
namespace freebsd {
namespace interface {

WifiInterface::WifiInterface()
    : netd::Ieee80211Interface(),
      name_(""),
      ssid_(""),
      channel_(0),
      mode_("infrastructure"),
      security_("none"),
      socket_(-1) {
}

WifiInterface::WifiInterface(const std::string& name)
    : netd::Ieee80211Interface(),
      name_(name),
      ssid_(""),
      channel_(0),
      mode_("infrastructure"),
      security_("none"),
      socket_(-1) {
}

WifiInterface::~WifiInterface() {
    closeSocket();
}

bool WifiInterface::createInterface() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        logger.error("Failed to open socket for creating WiFi interface");
        return false;
    }
    
    // Use SIOCIFCREATE to create WiFi interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
    
    if (ioctl(socket_, SIOCIFCREATE, &ifr) < 0) {
        logger.error("Failed to create WiFi interface " + name_ + ": " + std::strerror(errno));
        closeSocket();
        return false;
    }
    
    logger.info("Created WiFi interface " + name_);
    return true;
}

bool WifiInterface::destroyInterface() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        logger.error("Failed to open socket for destroying WiFi interface");
        return false;
    }
    
    // Use SIOCIFDESTROY to destroy WiFi interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
    
    if (ioctl(socket_, SIOCIFDESTROY, &ifr) < 0) {
        logger.error("Failed to destroy WiFi interface " + name_ + ": " + std::strerror(errno));
        closeSocket();
        return false;
    }
    
    logger.info("Destroyed WiFi interface " + name_);
    return true;
}

bool WifiInterface::loadFromSystem() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        return false;
    }
    
    if (!getWifiInfo()) {
        closeSocket();
        return false;
    }
    
    closeSocket();
    logger.info("Loaded WiFi interface information from system: " + name_);
    return true;
}

bool WifiInterface::applyToSystem() {
    auto& logger = Logger::getInstance();
    
    if (!openSocket()) {
        return false;
    }
    
    if (!setWifiInfo()) {
        closeSocket();
        return false;
    }
    
    closeSocket();
    logger.info("Applied WiFi interface configuration to system: " + name_);
    return true;
}

bool WifiInterface::setSsid(const std::string& ssid) {
    ssid_ = ssid;
    return true;
}

std::string WifiInterface::getSsid() const {
    return ssid_;
}

bool WifiInterface::setChannel(uint8_t channel) {
    channel_ = channel;
    return true;
}

uint8_t WifiInterface::getChannel() const {
    return channel_;
}

bool WifiInterface::setMode(const std::string& mode) {
    mode_ = mode;
    return true;
}

std::string WifiInterface::getMode() const {
    return mode_;
}

bool WifiInterface::setSecurity(const std::string& security) {
    security_ = security;
    return true;
}

std::string WifiInterface::getSecurity() const {
    return security_;
}

WifiInterface::operator netd::Ieee80211Interface() const {
    // Cast to shared interface - we inherit from it so this is safe
    return static_cast<const netd::Ieee80211Interface&>(*this);
}

bool WifiInterface::openSocket() {
    if (socket_ >= 0) {
        return true; // Already open
    }

    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) {
        return false;
    }

    return true;
}

void WifiInterface::closeSocket() {
    if (socket_ >= 0) {
        close(socket_);
        socket_ = -1;
    }
}

bool WifiInterface::getWifiInfo() {
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

    // Get WiFi-specific information
    // TODO: Use IEEE80211-specific ioctls to get WiFi details

    return true;
}

bool WifiInterface::setWifiInfo() const {
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

    // Set WiFi-specific information
    // TODO: Use IEEE80211-specific ioctls to set WiFi details

    return true;
}

} // namespace interface
} // namespace freebsd
} // namespace netd
