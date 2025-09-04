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

#ifndef NETD_FREEBSD_INTERFACE_LAGG_HPP
#define NETD_FREEBSD_INTERFACE_LAGG_HPP

#include <string>
#include <cstdint>
#include <memory>
#include <vector>

#include <shared/include/interface/lagg.hpp>
#include <shared/include/base/serialization.hpp>

namespace netd {
namespace freebsd {
namespace interface {

class LagInterface : public netd::LagInterface {
public:
    LagInterface();
    explicit LagInterface(const std::string& name);
    virtual ~LagInterface();

    // Interface name
    std::string getName() const { return name_; }

    // FreeBSD-specific operations
    bool createInterface();
    bool destroyInterface();
    bool loadFromSystem();
    bool applyToSystem();

    // LAGG-specific configuration
    bool setLaggProtocol(const std::string& protocol);
    std::string getLaggProtocol() const;
    bool addLaggPort(const std::string& portName);
    bool removeLaggPort(const std::string& portName);
    std::vector<std::string> getLaggPorts() const;

    // Statistics and information
    std::string getType() const { return "lagg"; }

    // Conversion to shared interface for serialization
    operator netd::LagInterface() const;

private:
    // Interface name
    std::string name_;
    
    // LAGG-specific members
    std::string laggProtocol_;
    std::vector<std::string> laggPorts_;
    
    // FreeBSD system interface
    int socket_;
    
    // Helper methods
    bool openSocket();
    void closeSocket();
    bool getLaggInfo();
    bool setLaggInfo() const;
};

} // namespace interface
} // namespace freebsd
} // namespace netd

#endif // NETD_FREEBSD_INTERFACE_LAGG_HPP
