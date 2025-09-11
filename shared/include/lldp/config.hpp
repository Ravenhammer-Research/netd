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
#include <string>

namespace netd::shared::lldp {

class Config {
public:
    Config(lldpctl_conn_t* connection);
    ~Config();

    // System configuration
    bool setHostname(const std::string& hostname);
    bool setDescription(const std::string& description);
    bool setPlatform(const std::string& platform);
    
    // LLDP configuration
    bool setTxInterval(int interval);
    bool setTxHold(int hold);
    bool setReceiveOnly(bool receive_only);
    bool setPaused(bool paused);
    bool setFastStartEnabled(bool enabled);
    bool setFastStartInterval(int interval);
    
    // Interface configuration
    bool setInterfacePattern(const std::string& pattern);
    bool setManagementPattern(const std::string& pattern);
    bool setChassisIdPattern(const std::string& pattern);
    bool setChassisIdString(const std::string& cid_string);
    
    // Capability configuration
    bool setChassisCapAdvertise(bool advertise);
    bool setChassisMgmtAdvertise(bool advertise);
    bool setChassisCapOverride(bool override);
    
    // LLDP-MED configuration
    bool setLldpMedNoInventory(bool no_inventory);
    
    // Port configuration
    bool setLldpPortIdType(int type);
    bool setLldpAgentType(int type);
    bool setMaxNeighbors(int max_neighbors);
    
    bool isValid() const;

private:
    lldpctl_conn_t* connection_;
    bool setStringValue(lldpctl_key_t key, const std::string& value);
    bool setIntValue(lldpctl_key_t key, int value);
};

} // namespace netd::shared::lldp
