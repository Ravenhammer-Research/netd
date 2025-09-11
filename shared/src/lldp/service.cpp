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

#include <shared/include/lldp/service.hpp>
#include <shared/include/lldp/interface.hpp>
#include <shared/include/logger.hpp>
#include <sstream>

namespace netd::shared::lldp {

Service::Service(lldpctl_conn_t* connection)
    : connection_(connection)
    , registered_(false)
    , port_(0)
{
}

Service::~Service() {
    unregisterService();
}

bool Service::registerService(const std::string& service_name,
                             ServiceType service_type,
                             const std::string& hostname,
                             uint16_t port,
                             const std::string& interface_name,
                             const std::map<std::string, std::string>& additional_info) {
    if (!connection_) {
        auto& logger = Logger::getInstance();
        logger.error("LLDP connection not available");
        return false;
    }

    service_name_ = service_name;
    service_type_ = service_type;
    hostname_ = hostname;
    port_ = port;
    interface_name_ = interface_name;
    additional_info_ = additional_info;
    registered_ = true;

    // Auto-detect link-local addresses
    Interface interface(connection_);
    auto link_local_addrs = interface.getLinkLocalAddresses();
    if (!link_local_addrs.empty()) {
        // Use the first available link-local address as primary
        auto first_addr = link_local_addrs.begin()->second.get();
        if (first_addr) {
            ip_address_ = first_addr->getString();
        }
        
        // Add all link-local addresses to additional info
        std::ostringstream addr_list;
        bool first = true;
        for (const auto& pair : link_local_addrs) {
            if (!first) addr_list << ",";
            if (pair.second) {
                addr_list << pair.first << "=" << pair.second->getString();
            }
            first = false;
        }
        additional_info_["link_local_addresses"] = addr_list.str();
    } else {
        // Fallback to localhost if no link-local addresses found
        ip_address_ = "127.0.0.1";
    }

    auto& logger = Logger::getInstance();
    logger.info("Registered service: " + service_name + " (" + 
                serviceTypeToString(service_type) + ") on " + 
                hostname + ":" + std::to_string(port) + 
                " (link-local: " + ip_address_ + ")");

    return sendAdvertisement();
}

bool Service::unregisterService() {
    if (!registered_) {
        return true;
    }

    registered_ = false;
    
    auto& logger = Logger::getInstance();
    logger.info("Unregistered service: " + service_name_);
    
    return true;
}

bool Service::sendAdvertisement() {
    if (!connection_ || !registered_) {
        return false;
    }

    try {
        // Create a custom TLV with our service information
        ServiceInfo service_info;
        service_info.setServiceName(service_name_);
        service_info.setServiceType(service_type_);
        service_info.setHostname(hostname_);
        service_info.setServiceData(ip_address_ + ":" + std::to_string(port_));
        service_info.setInterfaceName(interface_name_);
        service_info.setAdditionalInfo(additional_info_);

        std::string tlv_data = createServiceTLV(service_info);
        
        // Get interfaces and set port descriptions
        lldpctl_atom_t* interfaces_atom = lldpctl_get_interfaces(connection_);
        if (!interfaces_atom) {
            return false;
        }

        lldpctl_atom_t* interface_atom;
        lldpctl_atom_foreach(interfaces_atom, interface_atom) {
            const char* interface_name = lldpctl_atom_get_str(interface_atom, lldpctl_k_interface_name);
            if (interface_name && (interface_name_.empty() || 
                                  std::string(interface_name) == interface_name_)) {
                
                // Get the port for this interface
                lldpctl_atom_t* port_atom = lldpctl_get_port(interface_atom);
                if (port_atom) {
                    // Set port description to include our service info
                    lldpctl_atom_t* result = lldpctl_atom_set_str(port_atom, lldpctl_k_port_descr, tlv_data.c_str());
                    if (result) {
                        lldpctl_atom_dec_ref(result);
                    }
                    lldpctl_atom_dec_ref(port_atom);
                }
            }
        }
        
        lldpctl_atom_dec_ref(interfaces_atom);
        return true;
    } catch (const std::exception& e) {
        auto& logger = Logger::getInstance();
        logger.error("Exception sending LLDP advertisement: " + std::string(e.what()));
        return false;
    }
}

std::string Service::createServiceTLV(const ServiceInfo& service) const {
    std::ostringstream oss;
    oss << "NETD_SERVICE:" << service.getServiceName() << ":" 
        << serviceTypeToString(service.getServiceType()) << ":"
        << service.getHostname() << ":" << service.getServiceData();
    
    for (const auto& pair : service.getAdditionalInfo()) {
        oss << ":" << pair.first << "=" << pair.second;
    }
    
    return oss.str();
}

std::string Service::serviceTypeToString(ServiceType type) const {
    switch (type) {
        case ServiceType::NETD_SERVER:
            return "NETD_SERVER";
        case ServiceType::NETD_CLIENT:
            return "NETD_CLIENT";
        default:
            return "UNKNOWN";
    }
}

} // namespace netd::shared::lldp
