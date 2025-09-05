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

#include <freebsd/include/interface/lagg.hpp>
#include <shared/include/logger.hpp>

#include <net/if.h>
#include <net/if_var.h>
#include <net/ethernet.h>
#include <net/if_lagg.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <algorithm>

// Ensure ETHER_ADDR_LEN is defined
#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif

namespace netd::freebsd::interface {

    LagInterface::LagInterface()
        : netd::shared::interface::LagInterface(),
        name_(""),
        laggProtocol_("failover"),
        laggPorts_(),
        socket_(-1) {
    }

    LagInterface::LagInterface(const std::string& name)
        : netd::shared::interface::LagInterface(),
        name_(name),
        laggProtocol_("failover"),
        laggPorts_(),
        socket_(-1) {
    }

    LagInterface::~LagInterface() {
        closeSocket();
    }

    bool LagInterface::createInterface() {
        auto& logger = shared::Logger::getInstance();
        
        if (!openSocket()) {
            logger.error("Failed to open socket for creating LAGG interface");
            return false;
        }
        
        // Use SIOCIFCREATE to create LAGG interface
        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
        
        if (ioctl(socket_, SIOCIFCREATE, &ifr) < 0) {
            logger.error("Failed to create LAGG interface " + name_ + ": " + std::strerror(errno));
            closeSocket();
            return false;
        }
        
        logger.info("Created LAGG interface " + name_ + " with protocol " + laggProtocol_);
        return true;
    }

    bool LagInterface::destroyInterface() {
        auto& logger = shared::Logger::getInstance();
        
        if (!openSocket()) {
            logger.error("Failed to open socket for destroying LAGG interface");
            return false;
        }
        
        // Use SIOCIFDESTROY to destroy LAGG interface
        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
        
        if (ioctl(socket_, SIOCIFDESTROY, &ifr) < 0) {
            logger.error("Failed to destroy LAGG interface " + name_ + ": " + std::strerror(errno));
            closeSocket();
            return false;
        }
        
        logger.info("Destroyed LAGG interface " + name_);
        return true;
    }

    bool LagInterface::loadFromSystem() {
        auto& logger = shared::Logger::getInstance();
        
        if (!openSocket()) {
            return false;
        }
        
        if (!getLaggInfo()) {
            closeSocket();
            return false;
        }
        
        closeSocket();
        logger.info("Loaded LAGG interface information from system: " + name_);
        return true;
    }

    bool LagInterface::applyToSystem() {
        auto& logger = shared::Logger::getInstance();
        
        if (!openSocket()) {
            return false;
        }
        
        if (!setLaggInfo()) {
            closeSocket();
            return false;
        }
        
        closeSocket();
        logger.info("Applied LAGG interface configuration to system: " + name_);
        return true;
    }

    bool LagInterface::setLaggProtocol(const std::string& protocol) {
        laggProtocol_ = protocol;
        return true;
    }

    std::string LagInterface::getLaggProtocol() const {
        return laggProtocol_;
    }

    bool LagInterface::addLaggPort(const std::string& portName) {
        auto& logger = shared::Logger::getInstance();
        
        if (!openSocket()) {
            logger.error("Failed to open socket for adding LAGG port");
            return false;
        }
        
        // Use SIOCSLAGGPORT to add port to LAGG
        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
        std::strncpy(ifr.ifr_data, portName.c_str(), IFNAMSIZ - 1);
        
        if (ioctl(socket_, SIOCSLAGGPORT, &ifr) < 0) {
            logger.error("Failed to add port " + portName + " to LAGG interface " + name_ + ": " + std::strerror(errno));
            closeSocket();
            return false;
        }
        
        laggPorts_.push_back(portName);
        logger.info("Added port " + portName + " to LAGG interface " + name_);
        return true;
    }

    bool LagInterface::removeLaggPort(const std::string& portName) {
        auto& logger = shared::Logger::getInstance();
        
        if (!openSocket()) {
            logger.error("Failed to open socket for removing LAGG port");
            return false;
        }
        
        // Use SIOCSLAGGPORT with negative flag to remove port from LAGG
        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);
        std::strncpy(ifr.ifr_data, portName.c_str(), IFNAMSIZ - 1);
        
        if (ioctl(socket_, SIOCSLAGGPORT, &ifr) < 0) {
            logger.error("Failed to remove port " + portName + " from LAGG interface " + name_ + ": " + std::strerror(errno));
            closeSocket();
            return false;
        }
        
        // Remove from local list
        laggPorts_.erase(std::remove(laggPorts_.begin(), laggPorts_.end(), portName), laggPorts_.end());
        logger.info("Removed port " + portName + " from LAGG interface " + name_);
        return true;
    }

    std::vector<std::string> LagInterface::getLaggPorts() const {
        return laggPorts_;
    }

    LagInterface::operator netd::shared::interface::LagInterface() const {
        // Cast to shared interface - we inherit from it so this is safe
        return static_cast<const netd::shared::interface::LagInterface&>(*this);
    }

    bool LagInterface::openSocket() {
        if (socket_ >= 0) {
            return true; // Already open
        }

        socket_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_ < 0) {
            return false;
        }

        return true;
    }

    void LagInterface::closeSocket() {
        if (socket_ >= 0) {
            close(socket_);
            socket_ = -1;
        }
    }

    bool LagInterface::getLaggInfo() {
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

        // Get LAGG-specific information
        // TODO: Use SIOCGLAGG to get LAGG details

        return true;
    }

    bool LagInterface::setLaggInfo() const {
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

        // Set LAGG-specific information
        // TODO: Use SIOCSLAGG to set LAGG details

        return true;
    }

} // namespace netd::freebsd::interface
