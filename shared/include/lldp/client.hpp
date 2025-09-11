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

#include <memory>
#include <string>
#include <vector>
#include <map>
#include "connection.hpp"
#include "discovery.hpp"
#include "service.hpp"
#include "interface.hpp"

namespace netd::shared::lldp {

class Client {
public:
    Client();
    ~Client();

    bool initialize();
    void cleanup();

    bool registerService(const std::string& service_name,
                        ServiceType service_type,
                        const std::string& hostname,
                        uint16_t port,
                        const std::string& interface_name = "",
                        const std::map<std::string, std::string>& additional_info = {});
    
    bool unregisterService();
    bool startDiscovery();
    void stopDiscovery();
    bool discoverOnce();

    std::vector<ServiceInfo> getDiscoveredServices() const;
    std::vector<ServiceInfo> getDiscoveredServices(ServiceType service_type) const;
    std::vector<ServiceInfo> getDiscoveredServices(const std::string& service_name) const;

    std::vector<std::string> getLLDPInterfaces() const;
    std::map<std::string, std::string> getLinkLocalAddresses() const;

    bool isInitialized() const { return initialized_; }
    bool isRegistered() const;
    bool isDiscoveryRunning() const;

private:
    std::unique_ptr<Connection> connection_;
    std::unique_ptr<Discovery> discovery_;
    std::unique_ptr<Service> service_;
    std::unique_ptr<Interface> interface_;
    bool initialized_;
};

} // namespace netd::shared::lldp