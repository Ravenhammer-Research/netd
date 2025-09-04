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

#ifndef NETD_FREEBSD_VRF_HPP
#define NETD_FREEBSD_VRF_HPP

#include <shared/include/vrf.hpp>
#include <string>
#include <vector>
#include <memory>

namespace netd {
namespace freebsd {

class VRF : public netd::VRFAbstract {
public:
    VRF();
    explicit VRF(uint32_t fibId);
    explicit VRF(const std::string& name, uint32_t fibId);
    virtual ~VRF();

    // VRF management
    bool create();
    bool destroy();
    bool activate();
    bool deactivate();
    
    // FreeBSD-specific operations
    bool loadFromSystem();
    bool applyToSystem() const;
    
    // FIB table management
    bool setFibTable(uint32_t tableNumber);
    uint32_t getFibTable() const;
    bool addRoute(const std::string& destination, const std::string& gateway, const std::string& interface = "");
    bool removeRoute(const std::string& destination, const std::string& gateway = "");
    std::vector<std::string> getRoutes() const;

    // Implement abstract methods from shared VRF
    uint32_t getId() const override;
    std::string getName() const override;
    bool isActive() const override;
    
    // Conversion to shared VRF for serialization
    operator const netd::VRF&() const;

private:
    std::string name_;
    uint32_t fibTable_;
    bool active_;
    
    // Helper methods
    bool createFibTable();
    bool destroyFibTable();
    bool setFibTableActive(bool active);
    int getFibCount() const;
    bool isFibValid(uint32_t fibId) const;
};

} // namespace freebsd
} // namespace netd

#endif // NETD_FREEBSD_VRF_HPP