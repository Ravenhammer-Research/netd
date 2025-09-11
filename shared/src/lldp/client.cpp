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

#include <shared/include/lldp/client.hpp>
#include <shared/include/exception.hpp>

namespace netd::shared::lldp {

Client::Client()
    : initialized_(false)
{
}

Client::~Client() {
    cleanup();
}

void Client::initialize() {
    connection_ = std::make_unique<Connection>();
    connection_->initialize();

    interface_ = std::make_unique<Interface>(connection_->getConnection());

    initialized_ = true;
}

void Client::cleanup() {
    if (connection_) {
        connection_->cleanup();
    }
    
    initialized_ = false;
}

std::vector<std::string> Client::getLLDPInterfaces() const {
    if (!interface_) {
        return {};
    }
    return interface_->getLLDPInterfaces();
}

std::map<std::string, std::string> Client::getLinkLocalAddresses() const {
    if (!interface_) {
        return {};
    }
    
    auto address_map = interface_->getLinkLocalAddresses();
    std::map<std::string, std::string> string_map;
    
    for (const auto& pair : address_map) {
        if (pair.second) {
            string_map[pair.first] = pair.second->getString();
        }
    }
    
    return string_map;
}

std::vector<std::unique_ptr<Port>> Client::getPorts() const {
    std::vector<std::unique_ptr<Port>> ports;
    
    if (!initialized_ || !connection_) {
        return ports;
    }
    
    try {
        lldpctl_atom_t* interfaces_atom = lldpctl_get_interfaces(connection_->getConnection());
        if (!interfaces_atom) {
            return ports;
        }
        
        lldpctl_atom_t* interface_atom;
        lldpctl_atom_foreach(interfaces_atom, interface_atom) {
            lldpctl_atom_t* port_atom = lldpctl_get_port(interface_atom);
            if (port_atom) {
                ports.push_back(std::make_unique<Port>(port_atom, connection_->getConnection()));
            }
        }
        
        lldpctl_atom_dec_ref(interfaces_atom);
    } catch (const std::exception& e) {
        // Return empty vector on error
    }
    
    return ports;
}

std::unique_ptr<Port> Client::getLocalPort() const {
    if (!initialized_ || !connection_) {
        return nullptr;
    }
    
    try {
        // First get the local chassis to compare against
        lldpctl_atom_t* local_chassis_atom = lldpctl_get_local_chassis(connection_->getConnection());
        if (!local_chassis_atom) {
            return nullptr;
        }
        
        // Get interfaces and find a port that belongs to the local chassis
        lldpctl_atom_t* interfaces_atom = lldpctl_get_interfaces(connection_->getConnection());
        if (interfaces_atom) {
            lldpctl_atom_t* interface_atom;
            lldpctl_atom_foreach(interfaces_atom, interface_atom) {
                lldpctl_atom_t* port_atom = lldpctl_get_port(interface_atom);
                if (port_atom) {
                    // Get the chassis associated with this port
                    lldpctl_atom_t* port_chassis_atom = lldpctl_atom_get(port_atom, lldpctl_k_port_chassis);
                    if (port_chassis_atom) {
                        // Compare chassis IDs to ensure this port belongs to the local chassis
                        const char* local_chassis_id = lldpctl_atom_get_str(local_chassis_atom, lldpctl_k_chassis_id);
                        const char* port_chassis_id = lldpctl_atom_get_str(port_chassis_atom, lldpctl_k_chassis_id);
                        
                        if (local_chassis_id && port_chassis_id && 
                            strcmp(local_chassis_id, port_chassis_id) == 0) {
                            // This port belongs to the local chassis
                            lldpctl_atom_dec_ref(port_chassis_atom);
                            lldpctl_atom_dec_ref(interfaces_atom);
                            lldpctl_atom_dec_ref(local_chassis_atom);
                            return std::make_unique<Port>(port_atom, connection_->getConnection());
                        }
                        lldpctl_atom_dec_ref(port_chassis_atom);
                    }
                    lldpctl_atom_dec_ref(port_atom);
                }
            }
            lldpctl_atom_dec_ref(interfaces_atom);
        }
        lldpctl_atom_dec_ref(local_chassis_atom);
    } catch (const std::exception& e) {
        // Return nullptr on error
    }
    
    return nullptr;
}

std::vector<std::unique_ptr<Port>> Client::getAllLocalPorts() const {
    std::vector<std::unique_ptr<Port>> local_ports;
    
    if (!initialized_ || !connection_) {
        return local_ports;
    }
    
    try {
        // First get the local chassis to compare against
        lldpctl_atom_t* local_chassis_atom = lldpctl_get_local_chassis(connection_->getConnection());
        if (!local_chassis_atom) {
            return local_ports;
        }
        
        // Get interfaces and find all ports that belong to the local chassis
        lldpctl_atom_t* interfaces_atom = lldpctl_get_interfaces(connection_->getConnection());
        if (interfaces_atom) {
            lldpctl_atom_t* interface_atom;
            lldpctl_atom_foreach(interfaces_atom, interface_atom) {
                lldpctl_atom_t* port_atom = lldpctl_get_port(interface_atom);
                if (port_atom) {
                    // Get the chassis associated with this port
                    lldpctl_atom_t* port_chassis_atom = lldpctl_atom_get(port_atom, lldpctl_k_port_chassis);
                    if (port_chassis_atom) {
                        // Compare chassis IDs to ensure this port belongs to the local chassis
                        const char* local_chassis_id = lldpctl_atom_get_str(local_chassis_atom, lldpctl_k_chassis_id);
                        const char* port_chassis_id = lldpctl_atom_get_str(port_chassis_atom, lldpctl_k_chassis_id);
                        
                        if (local_chassis_id && port_chassis_id && 
                            strcmp(local_chassis_id, port_chassis_id) == 0) {
                            // This port belongs to the local chassis, add it to the list
                            local_ports.push_back(std::make_unique<Port>(port_atom, connection_->getConnection()));
                        }
                        lldpctl_atom_dec_ref(port_chassis_atom);
                    }
                    lldpctl_atom_dec_ref(port_atom);
                }
            }
            lldpctl_atom_dec_ref(interfaces_atom);
        }
        lldpctl_atom_dec_ref(local_chassis_atom);
    } catch (const std::exception& e) {
        // Return empty vector on error
    }
    
    return local_ports;
}

std::unique_ptr<Chassis> Client::getLocalChassis() const {
    if (!initialized_ || !connection_) {
        return nullptr;
    }
    
    try {
        lldpctl_atom_t* chassis_atom = lldpctl_get_local_chassis(connection_->getConnection());
        if (!chassis_atom) {
            return nullptr;
        }
        
        return std::make_unique<Chassis>(chassis_atom);
    } catch (const std::exception& e) {
        return nullptr;
    }
}

std::unique_ptr<Config> Client::getConfiguration() const {
    if (!initialized_ || !connection_) {
        return nullptr;
    }
    
    return std::make_unique<Config>(connection_->getConnection());
}

std::unique_ptr<CustomTLVManager> Client::getCustomTLVManager() const {
    if (!initialized_ || !connection_) {
        return nullptr;
    }
    
    return std::make_unique<CustomTLVManager>(connection_->getConnection());
}

} // namespace netd::shared::lldp