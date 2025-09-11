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

#include <shared/include/lldp/discovery.hpp>
#include <shared/include/lldp/neighbor.hpp>
#include <shared/include/logger.hpp>
#include <sstream>
#include <algorithm>

namespace netd::shared::lldp {

Discovery::Discovery(lldpctl_conn_t* connection)
    : connection_(connection)
    , running_(false)
    , stop_discovery_(false)
{
}

Discovery::~Discovery() {
    stop();
}

bool Discovery::start() {
    if (running_) {
        return true;
    }

    stop_discovery_ = false;
    discovery_thread_ = std::make_unique<std::thread>(&Discovery::discoveryLoop, this);
    running_ = true;

    auto& logger = Logger::getInstance();
    logger.info("Started LLDP discovery process");
    return true;
}

void Discovery::stop() {
    if (!running_) {
        return;
    }

    stop_discovery_ = true;
    if (discovery_thread_ && discovery_thread_->joinable()) {
        discovery_thread_->join();
    }
    discovery_thread_.reset();
    running_ = false;

    auto& logger = Logger::getInstance();
    logger.info("Stopped LLDP discovery process");
}

std::vector<ServiceInfo> Discovery::getDiscoveredServices() const {
    std::lock_guard<std::mutex> lock(services_mutex_);
    std::vector<ServiceInfo> services;
    
    for (const auto& pair : discovered_services_) {
        services.push_back(pair.second);
    }
    
    return services;
}

std::vector<ServiceInfo> Discovery::getDiscoveredServices(ServiceType service_type) const {
    std::lock_guard<std::mutex> lock(services_mutex_);
    std::vector<ServiceInfo> services;
    
    for (const auto& pair : discovered_services_) {
        if (pair.second.getServiceType() == service_type) {
            services.push_back(pair.second);
        }
    }
    
    return services;
}

std::vector<ServiceInfo> Discovery::getDiscoveredServices(const std::string& service_name) const {
    std::lock_guard<std::mutex> lock(services_mutex_);
    std::vector<ServiceInfo> services;
    
    for (const auto& pair : discovered_services_) {
        if (pair.second.getServiceName() == service_name) {
            services.push_back(pair.second);
        }
    }
    
    return services;
}

bool Discovery::processNeighbors() {
    if (!connection_) {
        return false;
    }

    try {
        std::lock_guard<std::mutex> lock(services_mutex_);
        
        lldpctl_atom_t* interfaces_atom = lldpctl_get_interfaces(connection_);
        if (!interfaces_atom) {
            return false;
        }

        lldpctl_atom_t* interface_atom;
        lldpctl_atom_foreach(interfaces_atom, interface_atom) {
            const char* interface_name = lldpctl_atom_get_str(interface_atom, lldpctl_k_interface_name);
            if (!interface_name) {
                continue;
            }

            // Get the port for this interface
            lldpctl_atom_t* port_atom = lldpctl_get_port(interface_atom);
            if (!port_atom) {
                continue;
            }

            // Get neighbors for this port
            lldpctl_atom_t* neighbors_atom = lldpctl_atom_get(port_atom, lldpctl_k_port_neighbors);
            if (!neighbors_atom) {
                lldpctl_atom_dec_ref(port_atom);
                continue;
            }

            lldpctl_atom_t* neighbor_atom;
            lldpctl_atom_foreach(neighbors_atom, neighbor_atom) {
                // Create Neighbor object to handle neighbor information
                Neighbor neighbor(neighbor_atom);
                if (neighbor.isValid()) {
                    ServiceInfo service_info = neighbor.getServiceInfo();
                    if (!service_info.getServiceName().empty()) {
                        // Create unique key for this service
                        std::string key = service_info.getServiceName() + ":" + 
                                         neighbor.getSystemName() + ":" + 
                                         service_info.getServiceData();
                        
                        // Create ServiceInfo with proper timing
                        ServiceInfo updated_service = std::move(service_info);
                        updated_service.setLastSeen(std::chrono::system_clock::now());
                        updated_service.setAddresses(neighbor.getManagementAddresses());
                        discovered_services_[key] = std::move(updated_service);
                    }
                }
            }
            
            lldpctl_atom_dec_ref(neighbors_atom);
            lldpctl_atom_dec_ref(port_atom);
        }
        
        lldpctl_atom_dec_ref(interfaces_atom);
        return true;
    } catch (const std::exception& e) {
        auto& logger = Logger::getInstance();
        logger.error("Exception processing LLDP neighbors: " + std::string(e.what()));
        return false;
    }
}

void Discovery::discoveryLoop() {
    auto& logger = Logger::getInstance();
    
    while (!stop_discovery_) {
        try {
            processNeighbors();
            
            // Sleep for 30 seconds between discovery cycles
            std::this_thread::sleep_for(std::chrono::seconds(30));
        } catch (const std::exception& e) {
            logger.error("Exception in discovery loop: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}

std::string Discovery::serviceTypeToString(ServiceType type) const {
    switch (type) {
        case ServiceType::NETD_SERVER:
            return "NETD_SERVER";
        case ServiceType::NETD_CLIENT:
            return "NETD_CLIENT";
        default:
            return "UNKNOWN";
    }
}

ServiceType Discovery::stringToServiceType(const std::string& type_str) const {
    if (type_str == "NETD_SERVER") {
        return ServiceType::NETD_SERVER;
    } else if (type_str == "NETD_CLIENT") {
        return ServiceType::NETD_CLIENT;
    }
    return ServiceType::NETD_SERVER; // Default
}

ServiceInfo Discovery::parseServiceTLV(const std::string& tlv_data) const {
    ServiceInfo service_info;
    
    // Check if this is a NETD service TLV
    if (tlv_data.find("NETD_SERVICE:") != 0) {
        return service_info; // Empty service info
    }
    
    std::vector<std::string> parts;
    std::istringstream iss(tlv_data);
    std::string part;
    
    while (std::getline(iss, part, ':')) {
        parts.push_back(part);
    }
    
    if (parts.size() >= 6) {
        service_info.setServiceName(parts[1]);
        service_info.setServiceType(stringToServiceType(parts[2]));
        service_info.setHostname(parts[3]);
        service_info.setServiceData(parts[4] + ":" + parts[5]); // Combine IP and port into service_data
        
        // Parse additional info
        std::map<std::string, std::string> additional_info;
        for (size_t i = 6; i < parts.size(); ++i) {
            size_t eq_pos = parts[i].find('=');
            if (eq_pos != std::string::npos) {
                std::string key = parts[i].substr(0, eq_pos);
                std::string value = parts[i].substr(eq_pos + 1);
                additional_info[key] = value;
            }
        }
        service_info.setAdditionalInfo(additional_info);
    }
    
    return service_info;
}

bool Discovery::discoverOnce() {
    if (!connection_) {
        return false;
    }

    try {
        // Clear existing services for fresh discovery
        {
            std::lock_guard<std::mutex> lock(services_mutex_);
            discovered_services_.clear();
        }

        // Process neighbors once
        return processNeighbors();

    } catch (const std::exception& e) {
        auto& logger = Logger::getInstance();
        logger.error("Exception in discoverOnce: " + std::string(e.what()));
        return false;
    }
}

} // namespace netd::shared::lldp
