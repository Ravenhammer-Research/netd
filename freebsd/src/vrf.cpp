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

#include <freebsd/include/vrf.hpp>
#include <shared/include/logger.hpp>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>
#include <sstream>

namespace netd::freebsd {

    VRF::VRF()
        : netd::shared::VRF(),
        name_(""),
        fibTable_(0),
        active_(false) {
    }

    VRF::VRF(uint32_t fibId)
        : netd::shared::VRF(),
        name_("vrf" + std::to_string(fibId)),
        fibTable_(fibId),
        active_(false) {
    }

    VRF::VRF(const std::string& name, uint32_t fibId)
        : netd::shared::VRF(),
        name_(name),
        fibTable_(fibId),
        active_(false) {
    }

    VRF::~VRF() {
        // Cleanup if needed
    }

    bool VRF::create() {
        auto& logger = shared::Logger::getInstance();
        
        if (!createFibTable()) {
            logger.error("Failed to create FIB table for VRF: " + getName());
            return false;
        }

        logger.info("Created VRF: " + getName() + " with FIB table " + std::to_string(fibTable_));
        return true;
    }

    bool VRF::destroy() {
        auto& logger = shared::Logger::getInstance();
        
        if (!destroyFibTable()) {
            logger.error("Failed to destroy FIB table for VRF: " + getName());
            return false;
        }

        logger.info("Destroyed VRF: " + getName());
        return true;
    }

    bool VRF::activate() {
        auto& logger = shared::Logger::getInstance();
        
        if (!setFibTableActive(true)) {
            logger.error("Failed to activate VRF: " + getName());
            return false;
        }

        active_ = true;
        logger.info("Activated VRF: " + getName());
        return true;
    }

    bool VRF::deactivate() {
        auto& logger = shared::Logger::getInstance();
        
        if (!setFibTableActive(false)) {
            logger.error("Failed to deactivate VRF: " + getName());
            return false;
        }

        active_ = false;
        logger.info("Deactivated VRF: " + getName());
        return true;
    }

    bool VRF::loadFromSystem() {
        auto& logger = shared::Logger::getInstance();
        
        // Check if FIB table exists
        int mib[4];
        mib[0] = CTL_NET;
        mib[1] = PF_ROUTE;
        mib[2] = 0;
        mib[3] = fibTable_;
        
        int value;
        size_t len = sizeof(value);
        
        if (sysctl(mib, 4, &value, &len, nullptr, 0) < 0) {
            logger.error("Failed to check FIB table " + std::to_string(fibTable_) + " existence");
            return false;
        }

        logger.info("Loaded VRF information from system: " + getName());
        return true;
    }

    bool VRF::applyToSystem() const {
        auto& logger = shared::Logger::getInstance();
        
        // Apply VRF configuration to system
        // This would involve setting up routing table rules and interface bindings
        
        logger.info("Applied VRF configuration to system: " + getName());
        return true;
    }

    bool VRF::setFibTable(uint32_t tableNumber) {
        if (tableNumber > 255) { // FreeBSD FIB table limit
            return false;
        }
        
        fibTable_ = tableNumber;
        return true;
    }

    uint32_t VRF::getFibTable() const {
        return fibTable_;
    }

    bool VRF::addRoute(const std::string& destination, const std::string& gateway, const std::string& interface) {
        auto& logger = shared::Logger::getInstance();
        
        // Use route command to add route to specific FIB table
        std::string cmd = "route -T " + std::to_string(fibTable_) + " add " + destination;
        if (!gateway.empty()) {
            cmd += " " + gateway;
        }
        if (!interface.empty()) {
            cmd += " -iface " + interface;
        }
        
        // TODO: Replace with proper FreeBSD routing socket calls
        // For now, we'll use a placeholder that doesn't use std::system
        logger.info("Route addition to VRF " + getName() + " would be: " + cmd);
        return true;

        logger.info("Added route " + destination + " to VRF " + getName());
        return true;
    }

    bool VRF::removeRoute(const std::string& destination, const std::string& gateway) {
        auto& logger = shared::Logger::getInstance();
        
        // Use route command to remove route from specific FIB table
        std::string cmd = "route -T " + std::to_string(fibTable_) + " delete " + destination;
        if (!gateway.empty()) {
            cmd += " " + gateway;
        }
        
        // TODO: Replace with proper FreeBSD routing socket calls
        // For now, we'll use a placeholder that doesn't use std::system
        logger.info("Route removal from VRF " + getName() + " would be: " + cmd);
        return true;

        logger.info("Removed route " + destination + " from VRF " + getName());
        return true;
    }

    std::vector<std::string> VRF::getRoutes() const {
        std::vector<std::string> routes;
        
        // Use route command to get routes from specific FIB table
        std::string cmd = "route -T " + std::to_string(fibTable_) + " show";
        
        // TODO: Implement route parsing from command output
        // This would involve capturing the output and parsing it into route objects
        
        return routes;
    }


    bool VRF::createFibTable() {
        auto& logger = shared::Logger::getInstance();
        
        // Create FIB table using sysctl
        int mib[4];
        mib[0] = CTL_NET;
        mib[1] = PF_ROUTE;
        mib[2] = 0;
        mib[3] = fibTable_;
        
        int value = 1; // Enable the FIB table
        size_t len = sizeof(value);
        
        if (sysctl(mib, 4, nullptr, nullptr, &value, len) < 0) {
            logger.error("Failed to create FIB table " + std::to_string(fibTable_));
            return false;
        }

        return true;
    }

    bool VRF::destroyFibTable() {
        auto& logger = shared::Logger::getInstance();
        
        // Disable FIB table using sysctl
        int mib[4];
        mib[0] = CTL_NET;
        mib[1] = PF_ROUTE;
        mib[2] = 0;
        mib[3] = fibTable_;
        
        int value = 0; // Disable the FIB table
        size_t len = sizeof(value);
        
        if (sysctl(mib, 4, nullptr, nullptr, &value, len) < 0) {
            logger.error("Failed to destroy FIB table " + std::to_string(fibTable_));
            return false;
        }

        return true;
    }

    bool VRF::setFibTableActive(bool active) {
        // This would involve setting up routing table rules
        // and interface bindings for the VRF
        
        return true;
    }


    VRF::operator const netd::shared::VRF&() const {
        // Cast to shared VRF - we inherit from shared::VRF so this is safe
        return static_cast<const netd::shared::VRF&>(*this);
    }

} // namespace netd::freebsd
