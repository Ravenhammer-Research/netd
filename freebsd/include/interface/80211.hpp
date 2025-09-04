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

#ifndef NETD_FREEBSD_INTERFACE_80211_HPP
#define NETD_FREEBSD_INTERFACE_80211_HPP

#include <string>
#include <cstdint>
#include <memory>
#include <vector>

#include <shared/include/interface/80211.hpp>
#include <shared/include/base/serialization.hpp>

namespace netd {
namespace freebsd {
namespace interface {

class WifiInterface : public netd::WirelessInterface {
public:
    WifiInterface();
    explicit WifiInterface(const std::string& name);
    virtual ~WifiInterface();

    // Interface name
    std::string getName() const { return name_; }

    // FreeBSD-specific operations
    bool createInterface();
    bool destroyInterface();
    bool loadFromSystem();
    bool applyToSystem();

    // WiFi-specific configuration
    bool setSsid(const std::string& ssid);
    std::string getSsid() const;
    bool setChannel(uint8_t channel);
    uint8_t getChannel() const;
    bool setMode(const std::string& mode);
    std::string getMode() const;
    bool setSecurity(const std::string& security);
    std::string getSecurity() const;

    // Statistics and information
    std::string getType() const { return "80211"; }

    // Conversion to shared interface for serialization
    operator netd::WirelessInterface() const;

private:
    // Interface name
    std::string name_;
    
    // WiFi-specific members
    std::string ssid_;
    uint8_t channel_;
    std::string mode_;
    std::string security_;
    
    // FreeBSD system interface
    int socket_;
    
    // Helper methods
    bool openSocket();
    void closeSocket();
    bool getWifiInfo();
    bool setWifiInfo() const;
};

} // namespace interface
} // namespace freebsd
} // namespace netd

#endif // NETD_FREEBSD_INTERFACE_80211_HPP
