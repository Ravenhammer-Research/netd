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

#ifndef NETD_VRF_HPP
#define NETD_VRF_HPP

#include <string>
#include <cstdint>
#include <shared/include/base/serialization.hpp>

namespace netd::shared {

    class VRF : public base::Serialization<VRF> {
public:
        VRF() = default;
        VRF(uint32_t id, const std::string& name, bool active = false);
        virtual ~VRF() = default;

        // Serialization methods
        lyd_node* toYang(ly_ctx* ctx) const override;
        static VRF fromYang(const ly_ctx* ctx, const lyd_node* node);

        // VRF properties
        uint32_t getId() const;
        std::string getName() const;
        bool isActive() const;

        // Setters
        void setId(uint32_t id);
        void setName(const std::string& name);
        void setActive(bool active);

private:
        uint32_t id_;
        std::string name_;
        bool active_;
    };

} // namespace netd::shared

#endif // NETD_VRF_HPP
