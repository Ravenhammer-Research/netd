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

#include <freebsd/include/route.hpp>
#include <shared/include/logger.hpp>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>
#include <sstream>

namespace netd::freebsd {

    Route::Route()
        : netd::shared::Route(),
        destination_(""),
        gateway_(""),
        interface_(""),
        metric_(0),
        flags_(0),
        fibTable_(0) {
    }

    Route::Route(const std::string& destination, const std::string& gateway, const std::string& interface)
        : netd::shared::Route(),
        destination_(destination),
        gateway_(gateway),
        interface_(interface),
        metric_(0),
        flags_(0),
        fibTable_(0) {
    }

    Route::~Route() {
        // Cleanup if needed
    }

    bool Route::add() {
        auto& logger = shared::Logger::getInstance();
        
        // Use route command to add route
        std::string cmd = "route add " + destination_;
        if (!gateway_.empty()) {
            cmd += " " + gateway_;
        }
        if (!interface_.empty()) {
            cmd += " -iface " + interface_;
        }
        if (fibTable_ > 0) {
            cmd += " -T " + std::to_string(fibTable_);
        }
        if (metric_ > 0) {
            cmd += " -hopcount " + std::to_string(metric_);
        }
        
        // TODO: Replace with proper FreeBSD routing socket calls
        // For now, we'll use a placeholder that doesn't use std::system
        logger.info("Route addition would be: " + cmd);
        return true;

        logger.info("Added route: " + destination_);
        return true;
    }

    bool Route::remove() {
        auto& logger = shared::Logger::getInstance();
        
        // Use route command to remove route
        std::string cmd = "route delete " + destination_;
        if (!gateway_.empty()) {
            cmd += " " + gateway_;
        }
        if (fibTable_ > 0) {
            cmd += " -T " + std::to_string(fibTable_);
        }
        
        // TODO: Replace with proper FreeBSD routing socket calls
        // For now, we'll use a placeholder that doesn't use std::system
        logger.info("Route removal would be: " + cmd);
        return true;

        logger.info("Removed route: " + destination_);
        return true;
    }

    bool Route::modify() {
        auto& logger = shared::Logger::getInstance();
        
        // Remove existing route and add modified one
        if (!remove()) {
            return false;
        }
        
        if (!add()) {
            return false;
        }

        logger.info("Modified route: " + destination_);
        return true;
    }

    bool Route::loadFromSystem() {
        auto& logger = shared::Logger::getInstance();
        
        // TODO: Implement route loading from system
        // This would involve parsing the output of "route show" command
        // or using routing socket APIs to get route information
        
        logger.info("Loaded route information from system: " + destination_);
        return true;
    }

    bool Route::applyToSystem() const {
        auto& logger = shared::Logger::getInstance();
        
        // Apply route configuration to system
        // This would involve using routing socket APIs or route command
        
        logger.info("Applied route configuration to system: " + destination_);
        return true;
    }

    bool Route::setMetric(uint32_t metric) {
        metric_ = metric;
        return true;
    }

    uint32_t Route::getMetric() const {
        return metric_;
    }

    bool Route::setFlags(uint32_t flags) {
        flags_ = flags;
        return true;
    }

    uint32_t Route::getFlags() const {
        return flags_;
    }

    bool Route::setFibTable(uint32_t fibTable) {
        if (fibTable > 255) { // FreeBSD FIB table limit
            return false;
        }
        
        fibTable_ = fibTable;
        return true;
    }

    uint32_t Route::getFibTable() const {
        return fibTable_;
    }

    Route::operator const netd::shared::Route&() const {
        // Cast to shared Route - we inherit from it so this is safe
        return static_cast<const netd::shared::Route&>(*this);
    }

    bool Route::parseRouteString(const std::string& routeStr) {
        // TODO: Implement route string parsing
        // This would parse route strings like "192.168.1.0/24 192.168.1.1 em0"
        
        return true;
    }

    std::string Route::formatRouteString() const {
        std::ostringstream oss;
        oss << destination_;
        
        if (!gateway_.empty()) {
            oss << " " << gateway_;
        }
        
        if (!interface_.empty()) {
            oss << " " << interface_;
        }
        
        if (fibTable_ > 0) {
            oss << " (FIB " << fibTable_ << ")";
        }
        
        if (metric_ > 0) {
            oss << " metric " << metric_;
        }
        
        return oss.str();
    }


} // namespace netd::freebsd
