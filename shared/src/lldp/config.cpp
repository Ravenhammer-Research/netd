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

#include <shared/include/lldp/config.hpp>

namespace netd::shared::lldp {

Config::Config(lldpctl_conn_t* connection)
    : connection_(connection) {
}

Config::~Config() {
}

bool Config::setHostname(const std::string& hostname) {
    return setStringValue(lldpctl_k_config_hostname, hostname);
}

bool Config::setDescription(const std::string& description) {
    return setStringValue(lldpctl_k_config_description, description);
}

bool Config::setPlatform(const std::string& platform) {
    return setStringValue(lldpctl_k_config_platform, platform);
}

bool Config::setTxInterval(int interval) {
    return setIntValue(lldpctl_k_config_tx_interval, interval);
}

bool Config::setTxHold(int hold) {
    return setIntValue(lldpctl_k_config_tx_hold, hold);
}

bool Config::setReceiveOnly(bool receive_only) {
    return setIntValue(lldpctl_k_config_receiveonly, receive_only ? 1 : 0);
}

bool Config::setPaused(bool paused) {
    return setIntValue(lldpctl_k_config_paused, paused ? 1 : 0);
}

bool Config::setFastStartEnabled(bool enabled) {
    return setIntValue(lldpctl_k_config_fast_start_enabled, enabled ? 1 : 0);
}

bool Config::setFastStartInterval(int interval) {
    return setIntValue(lldpctl_k_config_fast_start_interval, interval);
}

bool Config::setInterfacePattern(const std::string& pattern) {
    return setStringValue(lldpctl_k_config_iface_pattern, pattern);
}

bool Config::setManagementPattern(const std::string& pattern) {
    return setStringValue(lldpctl_k_config_mgmt_pattern, pattern);
}

bool Config::setChassisIdPattern(const std::string& pattern) {
    return setStringValue(lldpctl_k_config_cid_pattern, pattern);
}

bool Config::setChassisIdString(const std::string& cid_string) {
    return setStringValue(lldpctl_k_config_cid_string, cid_string);
}

bool Config::setChassisCapAdvertise(bool advertise) {
    return setIntValue(lldpctl_k_config_chassis_cap_advertise, advertise ? 1 : 0);
}

bool Config::setChassisMgmtAdvertise(bool advertise) {
    return setIntValue(lldpctl_k_config_chassis_mgmt_advertise, advertise ? 1 : 0);
}

bool Config::setChassisCapOverride(bool override) {
    return setIntValue(lldpctl_k_config_chassis_cap_override, override ? 1 : 0);
}

bool Config::setLldpMedNoInventory(bool no_inventory) {
    return setIntValue(lldpctl_k_config_lldpmed_noinventory, no_inventory ? 1 : 0);
}

bool Config::setLldpPortIdType(int type) {
    return setIntValue(lldpctl_k_config_lldp_portid_type, type);
}

bool Config::setLldpAgentType(int type) {
    return setIntValue(lldpctl_k_config_lldp_agent_type, type);
}

bool Config::setMaxNeighbors(int max_neighbors) {
    return setIntValue(lldpctl_k_config_max_neighbors, max_neighbors);
}

bool Config::isValid() const {
    return connection_ != nullptr;
}

bool Config::setStringValue(lldpctl_key_t key, const std::string& value) {
    if (!connection_) {
        return false;
    }
    
    try {
        lldpctl_atom_t* config_atom = lldpctl_get_configuration(connection_);
        if (!config_atom) {
            return false;
        }
        
        lldpctl_atom_t* result = lldpctl_atom_set_str(config_atom, key, value.c_str());
        bool success = (result != nullptr);
        if (result) {
            lldpctl_atom_dec_ref(result);
        }
        lldpctl_atom_dec_ref(config_atom);
        return success;
    } catch (...) {
        return false;
    }
}

bool Config::setIntValue(lldpctl_key_t key, int value) {
    if (!connection_) {
        return false;
    }
    
    try {
        lldpctl_atom_t* config_atom = lldpctl_get_configuration(connection_);
        if (!config_atom) {
            return false;
        }
        
        lldpctl_atom_t* result = lldpctl_atom_set_int(config_atom, key, value);
        bool success = (result != nullptr);
        if (result) {
            lldpctl_atom_dec_ref(result);
        }
        lldpctl_atom_dec_ref(config_atom);
        return success;
    } catch (...) {
        return false;
    }
}

} // namespace netd::shared::lldp
