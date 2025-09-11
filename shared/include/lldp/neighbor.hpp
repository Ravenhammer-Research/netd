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
#include <vector>
#include <memory>
#include <chrono>
#include <shared/include/address.hpp>

namespace netd::shared::lldp {

class Neighbor {
public:
    Neighbor(lldpctl_atom_t* neighbor_atom);
    ~Neighbor();

    std::string getChassisId() const;
    std::string getPortId() const;
    std::string getSystemName() const;
    std::string getSystemDescription() const;
    std::string getPortDescription() const;
    std::chrono::seconds getTTL() const;
    std::chrono::system_clock::time_point getLastUpdate() const;
    std::vector<std::unique_ptr<netd::shared::Address>> getManagementAddresses() const;
    bool isValid() const;

private:
    lldpctl_atom_t* neighbor_atom_;
    std::string getStringValue(lldpctl_key_t key) const;
    std::chrono::seconds getSecondsValue(lldpctl_key_t key) const;
};

} // namespace netd::shared::lldp
