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

#include <shared/include/lldp/custom.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <sstream>
#include <iomanip>

namespace netd::shared::lldp {


CustomTLV::CustomTLV(lldpctl_atom_t* custom_tlv_atom)
    : custom_tlv_atom_(custom_tlv_atom) {
    if (custom_tlv_atom_) {
        lldpctl_atom_inc_ref(custom_tlv_atom_);
    }
}

CustomTLV::~CustomTLV() {
    if (custom_tlv_atom_) {
        lldpctl_atom_dec_ref(custom_tlv_atom_);
    }
}

std::string CustomTLV::getOUI() const {
    return getStringValue(lldpctl_k_custom_tlv_oui);
}

int CustomTLV::getOUISubtype() const {
    return getIntValue(lldpctl_k_custom_tlv_oui_subtype);
}

std::string CustomTLV::getOUIInfoString() const {
    return getStringValue(lldpctl_k_custom_tlv_oui_info_string);
}

std::string CustomTLV::getOperation() const {
    return getStringValue(lldpctl_k_custom_tlv_op);
}

bool CustomTLV::isValid() const {
    return custom_tlv_atom_ != nullptr && !getOUI().empty();
}

std::string CustomTLV::getStringValue(lldpctl_key_t key) const {
    if (!custom_tlv_atom_) {
        return "";
    }
    
    const char* value = lldpctl_atom_get_str(custom_tlv_atom_, key);
    return value ? std::string(value) : "";
}

int CustomTLV::getIntValue(lldpctl_key_t key) const {
    if (!custom_tlv_atom_) {
        return 0;
    }
    
    return lldpctl_atom_get_int(custom_tlv_atom_, key);
}

CustomTLVManager::CustomTLVManager(lldpctl_conn_t* connection)
    : connection_(connection) {
}

CustomTLVManager::~CustomTLVManager() {
}

std::vector<std::unique_ptr<CustomTLV>> CustomTLVManager::getCustomTLVs() const {
    std::vector<std::unique_ptr<CustomTLV>> custom_tlvs;
    
    if (!connection_) {
        return custom_tlvs;
    }
    
    lldpctl_atom_t* custom_tlvs_atom = getCustomTLVsAtom();
    if (!custom_tlvs_atom) {
        return custom_tlvs;
    }
    
    lldpctl_atom_t* custom_tlv_atom;
    lldpctl_atom_foreach(custom_tlvs_atom, custom_tlv_atom) {
        custom_tlvs.push_back(std::make_unique<CustomTLV>(custom_tlv_atom));
    }
    
    lldpctl_atom_dec_ref(custom_tlvs_atom);
    return custom_tlvs;
}

bool CustomTLVManager::addCustomTLV(const std::string& oui, int oui_subtype, 
                                   const std::string& info_string, const std::string& operation) {
    if (!connection_) {
        return false;
    }
    
    try {
        lldpctl_atom_t* custom_tlvs_atom = getCustomTLVsAtom();
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
        std::vector<uint8_t> oui_bytes;
        std::stringstream ss(oui);
        std::string byte_str;
        
        while (std::getline(ss, byte_str, ':')) {
            if (byte_str.length() == 2) {
                uint8_t byte = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
                oui_bytes.push_back(byte);
            }
        }
        
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
            // Get the default port (local port) for assignment
            lldpctl_atom_t* port = lldpctl_get_default_port(connection_);
            if (port) {
                result = lldpctl_atom_set(port, lldpctl_k_custom_tlv, custom_tlv_atom);
                if (!result) {
                    success = false;
                } else {
                    lldpctl_atom_dec_ref(result);
                }
                // Clean up the custom TLV atom reference after assignment
                lldpctl_atom_dec_ref(custom_tlv_atom);
                lldpctl_atom_dec_ref(port);
            } else {
                success = false;
                // Clean up the custom TLV atom reference since assignment failed
                lldpctl_atom_dec_ref(custom_tlv_atom);
            }
        } else {
            // Clean up the custom TLV atom reference since we didn't even try assignment
            lldpctl_atom_dec_ref(custom_tlv_atom);
        }
        
        // Clean up the custom TLVs atom reference
        lldpctl_atom_dec_ref(custom_tlvs_atom);
        return success;
    } catch (const std::exception& e) {
        return false;
    }
}

bool CustomTLVManager::clearCustomTLVs() {
    if (!connection_) {
        return false;
    }
    
    try {
        lldpctl_atom_t* custom_tlvs_atom = getCustomTLVsAtom();
        if (!custom_tlvs_atom) {
            return false;
        }
        
        lldpctl_atom_t* result = lldpctl_atom_set(custom_tlvs_atom, lldpctl_k_custom_tlvs_clear, nullptr);
        bool success = (result != nullptr);
        if (result) {
            lldpctl_atom_dec_ref(result);
        }
        
        lldpctl_atom_dec_ref(custom_tlvs_atom);
        return success;
    } catch (const std::exception& e) {
        return false;
    }
}


bool CustomTLVManager::isValid() const {
    return connection_ != nullptr;
}

lldpctl_atom_t* CustomTLVManager::getCustomTLVsAtom() const {
    if (!connection_) {
        return nullptr;
    }
    
    // Get the default port (local port) - this is what lldpd source uses for custom TLVs
    lldpctl_atom_t* port = lldpctl_get_default_port(connection_);
    if (!port) {
        return nullptr;
    }
    
    // Get the custom TLVs list from the port
    lldpctl_atom_t* custom_tlvs = lldpctl_atom_get(port, lldpctl_k_custom_tlvs);
    
    lldpctl_atom_dec_ref(port);
    return custom_tlvs;
}

} // namespace netd::shared::lldp
