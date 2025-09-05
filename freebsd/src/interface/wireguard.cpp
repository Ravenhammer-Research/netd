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

#include <freebsd/include/interface/wireguard.hpp>
#include <shared/include/logger.hpp>

#include <net/if.h>
#include <net/if_var.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <algorithm>

namespace netd::freebsd::interface {

    WireguardInterface::WireguardInterface()
        : netd::shared::interface::WireguardInterface(),
        name_(""),
        privateKey_(""),
        listenPort_(0),
        peers_(),
        socket_(-1) {
    }

    WireguardInterface::WireguardInterface(const std::string& name)
        : netd::shared::interface::WireguardInterface(),
        name_(name),
        privateKey_(""),
        listenPort_(0),
        peers_(),
        socket_(-1) {
    }

    WireguardInterface::~WireguardInterface() {
        closeSocket();
    }

    bool WireguardInterface::createInterface() {
        auto& logger = shared::Logger::getInstance();
        
        if (!openSocket()) {
            logger.error("Failed to open socket for creating WireGuard interface");
            return false;
        }
        
        // Use SIOCIFCREATE to create WireGuard interface
        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
        
        if (ioctl(socket_, SIOCIFCREATE, &ifr) < 0) {
            logger.error("Failed to create WireGuard interface " + name_ + ": " + std::strerror(errno));
            closeSocket();
            return false;
        }
        
        logger.info("Created WireGuard interface " + name_);
        return true;
    }

    bool WireguardInterface::destroyInterface() {
        auto& logger = shared::Logger::getInstance();
        
        if (!openSocket()) {
            logger.error("Failed to open socket for destroying WireGuard interface");
            return false;
        }
        
        // Use SIOCIFDESTROY to destroy WireGuard interface
        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
        
        if (ioctl(socket_, SIOCIFDESTROY, &ifr) < 0) {
            logger.error("Failed to destroy WireGuard interface " + name_ + ": " + std::strerror(errno));
            closeSocket();
            return false;
        }
        
        logger.info("Destroyed WireGuard interface " + name_);
        return true;
    }

    bool WireguardInterface::loadFromSystem() {
        auto& logger = shared::Logger::getInstance();
        
        if (!openSocket()) {
            return false;
        }
        
        if (!getWireguardInfo()) {
            closeSocket();
            return false;
        }
        
        closeSocket();
        logger.info("Loaded WireGuard interface information from system: " + name_);
        return true;
    }

    bool WireguardInterface::applyToSystem() {
        auto& logger = shared::Logger::getInstance();
        
        if (!openSocket()) {
            return false;
        }
        
        if (!setWireguardInfo()) {
            closeSocket();
            return false;
        }
        
        closeSocket();
        logger.info("Applied WireGuard interface configuration to system: " + name_);
        return true;
    }

    bool WireguardInterface::setPrivateKey(const std::string& privateKey) {
        privateKey_ = privateKey;
        return true;
    }

    std::string WireguardInterface::getPrivateKey() const {
        return privateKey_;
    }

    bool WireguardInterface::setListenPort(uint16_t port) {
        listenPort_ = port;
        return true;
    }

    uint16_t WireguardInterface::getListenPort() const {
        return listenPort_;
    }

    bool WireguardInterface::addPeer(const std::string& publicKey, const std::string& endpoint) {
        // Remove existing peer if it exists
        removePeer(publicKey);
        
        // Add new peer
        peers_.emplace_back(publicKey, endpoint);
        return true;
    }

    bool WireguardInterface::removePeer(const std::string& publicKey) {
        peers_.erase(
            std::remove_if(peers_.begin(), peers_.end(),
                [&publicKey](const auto& peer) { return peer.first == publicKey; }),
            peers_.end()
        );
        return true;
    }

    WireguardInterface::operator netd::shared::interface::WireguardInterface() const {
        // Cast to shared interface - we inherit from it so this is safe
        return static_cast<const netd::shared::interface::WireguardInterface&>(*this);
    }

    bool WireguardInterface::openSocket() {
        if (socket_ >= 0) {
            return true; // Already open
        }

        socket_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_ < 0) {
            return false;
        }

        return true;
    }

    void WireguardInterface::closeSocket() {
        if (socket_ >= 0) {
            close(socket_);
            socket_ = -1;
        }
    }

    bool WireguardInterface::getWireguardInfo() {
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

        // Get WireGuard-specific information
        // TODO: Use WireGuard-specific ioctls to get WireGuard details

        return true;
    }

    bool WireguardInterface::setWireguardInfo() const {
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

        // Set WireGuard-specific information
        // TODO: Use WireGuard-specific ioctls to set WireGuard details

        return true;
    }

} // namespace netd::freebsd::interface
