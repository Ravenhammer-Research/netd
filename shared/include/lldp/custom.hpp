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

namespace netd::shared::lldp {

class CustomTLV {
public:
    CustomTLV(lldpctl_atom_t* custom_tlv_atom);
    ~CustomTLV();

    std::string getOUI() const;
    int getOUISubtype() const;
    std::string getOUIInfoString() const;
    std::string getOperation() const;
    bool isValid() const;

private:
    lldpctl_atom_t* custom_tlv_atom_;
    std::string getStringValue(lldpctl_key_t key) const;
    int getIntValue(lldpctl_key_t key) const;
};

class CustomTLVManager {
public:
    CustomTLVManager(lldpctl_conn_t* connection);
    ~CustomTLVManager();

    std::vector<std::unique_ptr<CustomTLV>> getCustomTLVs() const;
    bool addCustomTLV(const std::string& oui, int oui_subtype, 
                     const std::string& info_string, const std::string& operation = "");
    bool clearCustomTLVs();
    bool isValid() const;

private:
    lldpctl_conn_t* connection_;
    lldpctl_atom_t* getCustomTLVsAtom() const;
};

} // namespace netd::shared::lldp
