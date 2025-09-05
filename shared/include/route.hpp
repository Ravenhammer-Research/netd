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

#ifndef NETD_ROUTE_HPP
#define NETD_ROUTE_HPP

#include <string>
#include <memory>
#include <cstdint>
#include <shared/include/address.hpp>
#include <shared/include/base/serialization.hpp>

namespace netd::shared {

    class Route : public base::Serialization<Route> {
public:
        Route() = default;
        Route(std::shared_ptr<Address> destination, std::shared_ptr<Address> gateway, 
              const std::string& interface, uint32_t vrf);
        virtual ~Route() = default;

        // Serialization methods
        lyd_node* toYang(ly_ctx* ctx) const override;
        static Route fromYang(const ly_ctx* ctx, const lyd_node* node);

        // Route properties
        std::shared_ptr<Address> getDestination() const;
        std::shared_ptr<Address> getGateway() const;
        std::string getInterface() const;
        uint32_t getVRF() const;

        // Setters
        void setDestination(std::shared_ptr<Address> destination);
        void setGateway(std::shared_ptr<Address> gateway);
        void setInterface(const std::string& interface);
        void setVRF(uint32_t vrf);

private:
        std::shared_ptr<Address> destination_;
        std::shared_ptr<Address> gateway_;
        std::string interface_;
        uint32_t vrf_;
    };

} // namespace netd::shared

#endif // NETD_ROUTE_HPP
