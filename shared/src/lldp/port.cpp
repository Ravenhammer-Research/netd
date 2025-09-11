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

#include <shared/include/lldp/port.hpp>
#include <shared/include/lldp/custom.hpp>
#include <shared/include/lldp/error.hpp>
#include <shared/include/logger.hpp>
#include <sstream>
#include <iomanip>

namespace netd::shared::lldp {

Port::Port(lldpctl_atom_t* port_atom, lldpctl_conn_t* connection)
    : port_atom_(port_atom), connection_(connection), atom_modified_(false) {
    if (port_atom_) {
        lldpctl_atom_inc_ref(port_atom_);
    }
}

Port::~Port() {
    if (port_atom_ && !atom_modified_) {
        lldpctl_atom_dec_ref(port_atom_);
    }
}

std::string Port::getPortName() const {
    return getStringValue(lldpctl_k_port_name);
}

std::string Port::getPortId() const {
    return getStringValue(lldpctl_k_port_id);
}

std::string Port::getPortDescription() const {
    return getStringValue(lldpctl_k_port_descr);
}

int Port::getPortIndex() const {
    return getIntValue(lldpctl_k_port_index);
}

int Port::getPortTTL() const {
    return getIntValue(lldpctl_k_port_ttl);
}

int Port::getPortAge() const {
    return getIntValue(lldpctl_k_port_age);
}

std::vector<std::unique_ptr<Neighbor>> Port::getNeighbors() const {
    std::vector<std::unique_ptr<Neighbor>> neighbors;
    
    if (!port_atom_) {
        return neighbors;
    }
    
    lldpctl_atom_t* neighbors_atom = lldpctl_atom_get(port_atom_, lldpctl_k_port_neighbors);
    if (!neighbors_atom) {
        return neighbors;
    }
    
    lldpctl_atom_t* neighbor_atom;
    lldpctl_atom_foreach(neighbors_atom, neighbor_atom) {
        neighbors.push_back(std::make_unique<Neighbor>(neighbor_atom));
    }
    
    lldpctl_atom_dec_ref(neighbors_atom);
    return neighbors;
}

std::unique_ptr<Chassis> Port::getChassis() const {
    if (!port_atom_) {
        return nullptr;
    }
    
    lldpctl_atom_t* chassis_atom = lldpctl_atom_get(port_atom_, lldpctl_k_port_chassis);
    if (!chassis_atom) {
        return nullptr;
    }
    
    return std::make_unique<Chassis>(chassis_atom);
}

bool Port::isValid() const {
    return port_atom_ != nullptr && !getPortName().empty();
}

std::string Port::getStringValue(lldpctl_key_t key) const {
    if (!port_atom_) {
        return "";
    }
    
    const char* value = lldpctl_atom_get_str(port_atom_, key);
    return value ? std::string(value) : "";
}

int Port::getIntValue(lldpctl_key_t key) const {
    if (!port_atom_) {
        return 0;
    }
    
    return lldpctl_atom_get_int(port_atom_, key);
}

// Helper function to parse OUI string (e.g., "00:1b:21") to binary bytes
std::vector<uint8_t> parseOUIString(const std::string& oui_str) {
    std::vector<uint8_t> bytes;
    std::stringstream ss(oui_str);
    std::string byte_str;
    
    while (std::getline(ss, byte_str, ':')) {
        if (byte_str.length() == 2) {
            uint8_t byte = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
            bytes.push_back(byte);
        }
    }
    
    return bytes;
}

lldpctl_atom_t* Port::getFreshPortAtom() const {
    if (!connection_) {
        return nullptr;
    }
    
    // Get a fresh port atom by iterating through interfaces
    lldpctl_atom_t* interfaces_atom = lldpctl_get_interfaces(connection_);
    if (!interfaces_atom) {
        return nullptr;
    }
    
    lldpctl_atom_t* interface_atom;
    lldpctl_atom_foreach(interfaces_atom, interface_atom) {
        lldpctl_atom_t* port_atom = lldpctl_get_port(interface_atom);
        if (port_atom) {
            // Found a port, return it (caller will need to decrement reference)
            lldpctl_atom_dec_ref(interfaces_atom);
            return port_atom;
        }
    }
    
    lldpctl_atom_dec_ref(interfaces_atom);
    return nullptr;
}

std::vector<std::unique_ptr<CustomTLV>> Port::getCustomTLVs() const {
    std::vector<std::unique_ptr<CustomTLV>> custom_tlvs;
    
    // If the port atom has been modified, get a fresh one for reading
    lldpctl_atom_t* port_atom_to_use = port_atom_;
    bool need_to_free_fresh_atom = false;
    
    if (atom_modified_ && connection_) {
        port_atom_to_use = getFreshPortAtom();
        if (port_atom_to_use) {
            need_to_free_fresh_atom = true;
        } else {
            return custom_tlvs;
        }
    }
    
    if (!port_atom_to_use) {
        return custom_tlvs;
    }
    
    try {
        lldpctl_atom_t* custom_tlvs_atom = lldpctl_atom_get(port_atom_to_use, lldpctl_k_custom_tlvs);
        if (!custom_tlvs_atom) {
            if (need_to_free_fresh_atom) {
                lldpctl_atom_dec_ref(port_atom_to_use);
            }
            return custom_tlvs;
        }
        
        lldpctl_atom_t* custom_tlv_atom;
        lldpctl_atom_foreach(custom_tlvs_atom, custom_tlv_atom) {
            custom_tlvs.push_back(std::make_unique<CustomTLV>(custom_tlv_atom));
        }
        
        // Note: lldpctl_atom_foreach handles reference counting internally
        // Do not manually decrement custom_tlvs_atom
        
        if (need_to_free_fresh_atom) {
            lldpctl_atom_dec_ref(port_atom_to_use);
        }
    } catch (const std::exception& e) {
        if (need_to_free_fresh_atom) {
            lldpctl_atom_dec_ref(port_atom_to_use);
        }
    } catch (...) {
        if (need_to_free_fresh_atom) {
            lldpctl_atom_dec_ref(port_atom_to_use);
        }
    }
    
    return custom_tlvs;
}

bool Port::addCustomTLV(const std::string& oui, int oui_subtype, 
                       const std::string& info_string, const std::string& operation) {
    if (!port_atom_) {
        return false;
    }
    
    try {
        // Get the custom TLVs list from the port
        lldpctl_atom_t* custom_tlvs_atom = lldpctl_atom_get(port_atom_, lldpctl_k_custom_tlvs);
        if (!custom_tlvs_atom) {
            return false;
        }
        
        // Create a new custom TLV atom
        lldpctl_atom_t* custom_tlv_atom = lldpctl_atom_create(custom_tlvs_atom);
        if (!custom_tlv_atom) {
            lldpctl_atom_dec_ref(custom_tlvs_atom);
            return false;
        }
        
        bool success = true;
        
        // Set OUI (convert from hex string to binary)
        std::vector<uint8_t> oui_bytes = parseOUIString(oui);
        lldpctl_atom_t* result = lldpctl_atom_set_buffer(custom_tlv_atom, lldpctl_k_custom_tlv_oui, 
                                                         oui_bytes.data(), oui_bytes.size());
        if (!result) {
            success = false;
        } else {
            lldpctl_atom_dec_ref(result);
        }
        
        // Set OUI subtype
        result = lldpctl_atom_set_int(custom_tlv_atom, lldpctl_k_custom_tlv_oui_subtype, oui_subtype);
        if (!result) {
            success = false;
        } else {
            lldpctl_atom_dec_ref(result);
        }
        
        // Set OUI info string
        result = lldpctl_atom_set_str(custom_tlv_atom, lldpctl_k_custom_tlv_oui_info_string, info_string.c_str());
        if (!result) {
            success = false;
        } else {
            lldpctl_atom_dec_ref(result);
        }
        
        // Set operation (default to "add" if not provided or invalid)
        std::string op = operation.empty() ? "add" : operation;
        if (op != "add" && op != "replace" && op != "remove") {
            op = "add"; // Default to add for any invalid operation
        }
        result = lldpctl_atom_set_str(custom_tlv_atom, lldpctl_k_custom_tlv_op, op.c_str());
        if (!result) {
            success = false;
        } else {
            lldpctl_atom_dec_ref(result);
        }
        
        // Assign the custom TLV to the port
        if (success) {
            result = lldpctl_atom_set(port_atom_, lldpctl_k_custom_tlv, custom_tlv_atom);
            if (!result) {
                success = false;
            } else {
                // Mark atom as modified to prevent destructor from trying to decrement invalid reference
                atom_modified_ = true;
            }
            // Clean up the custom TLV atom reference after assignment
            lldpctl_atom_dec_ref(custom_tlv_atom);
        } else {
            // Clean up the custom TLV atom reference since we didn't even try assignment
            lldpctl_atom_dec_ref(custom_tlv_atom);
        }
        
        // Don't decrement custom_tlvs_atom reference - it becomes invalid after assignment
        return success;
    } catch (const std::exception& e) {
        return false;
    }
}

bool Port::clearCustomTLVs() {
    if (!port_atom_) {
        return false;
    }
    
    try {
        // Clear custom TLVs directly on the port atom
        lldpctl_atom_t* result = lldpctl_atom_set(port_atom_, lldpctl_k_custom_tlvs_clear, nullptr);
        bool success = (result != nullptr);
        if (result) {
            lldpctl_atom_dec_ref(result);
            // Mark atom as modified to prevent destructor from trying to decrement invalid reference
            atom_modified_ = true;
        }
        return success;
    } catch (const std::exception& e) {
        return false;
    }
}


} // namespace netd::shared::lldp
