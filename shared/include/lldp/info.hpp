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

#include <shared/include/address.hpp>
#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace netd::shared::lldp {

enum class ServiceType {
    NETD_SERVER,
    NETD_CLIENT
};

class ServiceInfo {
public:
    // Default constructor
    ServiceInfo() = default;
    
    // Copy constructor
    ServiceInfo(const ServiceInfo& other);
    
    // Copy assignment operator
    ServiceInfo& operator=(const ServiceInfo& other);
    
    // Move constructor
    ServiceInfo(ServiceInfo&&) = default;
    
    // Move assignment operator
    ServiceInfo& operator=(ServiceInfo&&) = default;
    
    // Destructor
    ~ServiceInfo() = default;

    // Getters
    const std::string& getServiceName() const { return service_name_; }
    ServiceType getServiceType() const { return service_type_; }
    const std::string& getHostname() const { return hostname_; }
    const std::string& getServiceData() const { return service_data_; }
    const std::string& getInterfaceName() const { return interface_name_; }
    const std::vector<std::unique_ptr<netd::shared::Address>>& getAddresses() const { return addresses_; }
    const std::map<std::string, std::string>& getAdditionalInfo() const { return additional_info_; }
    const std::chrono::system_clock::time_point& getLastSeen() const { return last_seen_; }
    
    // Setters
    void setServiceName(const std::string& name) { service_name_ = name; }
    void setServiceType(ServiceType type) { service_type_ = type; }
    void setHostname(const std::string& hostname) { hostname_ = hostname; }
    void setServiceData(const std::string& data) { service_data_ = data; }
    void setInterfaceName(const std::string& interface) { interface_name_ = interface; }
    void setAddresses(std::vector<std::unique_ptr<netd::shared::Address>> addresses) { addresses_ = std::move(addresses); }
    void setAdditionalInfo(const std::map<std::string, std::string>& info) { additional_info_ = info; }
    void setLastSeen(const std::chrono::system_clock::time_point& time) { last_seen_ = time; }
    
    // Utility methods
    void addAddress(std::unique_ptr<netd::shared::Address> address);
    std::vector<std::string> getAddressStrings() const;
    bool isValid() const;

private:
    std::string service_name_;
    ServiceType service_type_;
    std::string hostname_;
    std::string service_data_;
    std::string interface_name_;
    std::vector<std::unique_ptr<netd::shared::Address>> addresses_;
    std::map<std::string, std::string> additional_info_;
    std::chrono::system_clock::time_point last_seen_;
};

} // namespace netd::shared::lldp
