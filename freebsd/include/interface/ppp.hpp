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

#ifndef NETD_FREEBSD_INTERFACE_PPP_HPP
#define NETD_FREEBSD_INTERFACE_PPP_HPP

#include <string>
#include <cstdint>
#include <memory>
#include <vector>

#include <shared/include/ethernet.hpp>
#include <shared/include/base/serialization.hpp>

namespace netd {
namespace freebsd {
namespace interface {

class PppInterface : public netd::Ethernet, 
                     public netd::base::Serialization<PppInterface> {
public:
    PppInterface();
    explicit PppInterface(const std::string& name);
    virtual ~PppInterface();

    // Interface name
    std::string getName() const { return name_; }

    // FreeBSD-specific operations
    bool createInterface();
    bool destroyInterface();
    bool loadFromSystem();
    bool applyToSystem();

    // PPP-specific configuration
    bool setPppUnit(int unit);
    int getPppUnit() const;
    bool setPppMode(const std::string& mode);
    std::string getPppMode() const;
    bool setPppProtocol(const std::string& protocol);
    std::string getPppProtocol() const;

    // Statistics and information
    std::string getType() const { return "ppp"; }

    // Serialization
    lyd_node* toYang() const override;
    static PppInterface fromYang(const lyd_node* node);

private:
    // Interface name
    std::string name_;
    
    // PPP-specific members
    int pppUnit_;
    std::string pppMode_;
    std::string pppProtocol_;
    
    // FreeBSD system interface
    int socket_;
    
    // Helper methods
    bool openSocket();
    void closeSocket();
    bool getPppInfo();
    bool setPppInfo() const;
};

} // namespace interface
} // namespace freebsd
} // namespace netd

#endif // NETD_FREEBSD_INTERFACE_PPP_HPP
