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

#include <shared/include/lldp/neighbor.hpp>
#include <shared/include/lldp/discovery.hpp>
#include <shared/include/logger.hpp>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sstream>
#include <regex>

namespace netd::shared::lldp {

Neighbor::Neighbor(lldpctl_atom_t* neighbor_atom)
    : neighbor_atom_(neighbor_atom) {
    if (neighbor_atom_) {
        lldpctl_atom_inc_ref(neighbor_atom_);
    }
}

Neighbor::~Neighbor() {
    if (neighbor_atom_) {
        lldpctl_atom_dec_ref(neighbor_atom_);
    }
}

std::string Neighbor::getChassisId() const {
    return getStringValue(lldpctl_k_chassis_id);
}

std::string Neighbor::getPortId() const {
    return getStringValue(lldpctl_k_port_id);
}

std::string Neighbor::getSystemName() const {
    // lldpctl_k_system_name doesn't exist, use port name as fallback
    return getStringValue(lldpctl_k_port_name);
}

std::string Neighbor::getSystemDescription() const {
    // lldpctl_k_system_descr doesn't exist, use port description as fallback
    return getStringValue(lldpctl_k_port_descr);
}

std::string Neighbor::getPortDescription() const {
    return getStringValue(lldpctl_k_port_descr);
}

std::chrono::seconds Neighbor::getTTL() const {
    // lldpctl_k_ttl doesn't exist, return default TTL
    return std::chrono::seconds(120); // Default LLDP TTL
}

std::chrono::system_clock::time_point Neighbor::getLastUpdate() const {
    // Get the last update time from the neighbor atom
    // This is a placeholder - actual implementation would depend on lldpctl API
    return std::chrono::system_clock::now();
}

ServiceInfo Neighbor::getServiceInfo() const {
    std::string sysdesc = getSystemDescription();
    return parseServiceTLV(sysdesc);
}

std::vector<std::unique_ptr<netd::shared::Address>> Neighbor::getManagementAddresses() const {
    std::vector<std::unique_ptr<netd::shared::Address>> addresses;
    
    if (!neighbor_atom_) {
        return addresses;
    }
    
    // Get management addresses from the neighbor atom
    lldpctl_atom_t* mgmt_addrs = lldpctl_atom_get(neighbor_atom_, lldpctl_k_chassis_mgmt);
    if (!mgmt_addrs) {
        return addresses;
    }
    
    lldpctl_atom_t* mgmt_addr;
    lldpctl_atom_foreach(mgmt_addrs, mgmt_addr) {
        // Get IP address directly (lldpctl_k_mgmt_addr_type doesn't exist)
        const char* addr_data = lldpctl_atom_get_str(mgmt_addr, lldpctl_k_mgmt_ip);
        
        if (addr_data) {
            // Try to parse as IPv4 first
            struct in_addr addr4;
            if (inet_pton(AF_INET, addr_data, &addr4) == 1) {
                auto ipv4_addr = std::make_unique<netd::shared::IPv4Address>(
                    ntohl(addr4.s_addr), 32);
                addresses.push_back(std::move(ipv4_addr));
            } else {
                // Try to parse as IPv6
                struct in6_addr addr6;
                if (inet_pton(AF_INET6, addr_data, &addr6) == 1) {
                    auto ipv6_addr = std::make_unique<netd::shared::IPv6Address>(
                        addr6.s6_addr, 128);
                    addresses.push_back(std::move(ipv6_addr));
                }
            }
        }
    }
    
    lldpctl_atom_dec_ref(mgmt_addrs);
    return addresses;
}

bool Neighbor::isValid() const {
    return neighbor_atom_ != nullptr && 
           !getChassisId().empty() && 
           !getPortId().empty();
}

std::string Neighbor::getStringValue(lldpctl_key_t key) const {
    if (!neighbor_atom_) {
        return "";
    }
    
    const char* value = lldpctl_atom_get_str(neighbor_atom_, key);
    return value ? std::string(value) : "";
}

std::chrono::seconds Neighbor::getSecondsValue(lldpctl_key_t key) const {
    if (!neighbor_atom_) {
        return std::chrono::seconds(0);
    }
    
    int value = lldpctl_atom_get_int(neighbor_atom_, key);
    return std::chrono::seconds(value);
}

ServiceInfo Neighbor::parseServiceTLV(const std::string& sysdesc) const {
    ServiceInfo service_info;
    
    // Simple regex to parse service information from system description
    // This is a placeholder implementation - actual parsing would depend on
    // the specific TLV format used
    std::regex service_regex(R"(SERVICE:(\w+):(\w+):(.+))");
    std::smatch matches;
    
    if (std::regex_search(sysdesc, matches, service_regex)) {
        if (matches.size() >= 4) {
            service_info.setServiceName(matches[1].str());
            // Convert string to ServiceType enum
            std::string type_str = matches[2].str();
            if (type_str == "NETD_SERVER") {
                service_info.setServiceType(ServiceType::NETD_SERVER);
            } else if (type_str == "NETD_CLIENT") {
                service_info.setServiceType(ServiceType::NETD_CLIENT);
            }
            service_info.setServiceData(matches[3].str());
            service_info.setHostname(getSystemName());
        }
    }
    
    return service_info;
}

} // namespace netd::shared::lldp
