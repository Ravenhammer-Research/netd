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

#pragma once

#include <lldpctl.h>
#include <lldp-const.h>
#include <shared/include/lldp/info.hpp>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <mutex>
#include <thread>
#include <memory>

namespace netd::shared::lldp {

class Discovery {
public:
    Discovery(lldpctl_conn_t* connection);
    ~Discovery();

    bool start();
    void stop();
    bool isRunning() const { return running_; }

    std::vector<ServiceInfo> getDiscoveredServices() const;
    std::vector<ServiceInfo> getDiscoveredServices(ServiceType service_type) const;
    std::vector<ServiceInfo> getDiscoveredServices(const std::string& service_name) const;

    bool processNeighbors();
    bool discoverOnce();

private:
    void discoveryLoop();
    ServiceInfo parseServiceTLV(const std::string& tlv_data) const;
    ServiceType stringToServiceType(const std::string& type_str) const;
    std::string serviceTypeToString(ServiceType type) const;

    lldpctl_conn_t* connection_;
    std::map<std::string, ServiceInfo> discovered_services_;
    mutable std::mutex services_mutex_;
    std::unique_ptr<std::thread> discovery_thread_;
    bool running_;
    bool stop_discovery_;
};

} // namespace netd::shared::lldp
