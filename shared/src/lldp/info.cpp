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

#include <shared/include/lldp/info.hpp>
#include <algorithm>

namespace netd::shared::lldp {

ServiceInfo::ServiceInfo(const ServiceInfo& other)
    : service_name_(other.service_name_)
    , service_type_(other.service_type_)
    , hostname_(other.hostname_)
    , service_data_(other.service_data_)
    , interface_name_(other.interface_name_)
    , additional_info_(other.additional_info_)
    , last_seen_(other.last_seen_) {
    // Deep copy addresses
    for (const auto& addr : other.addresses_) {
        if (addr) {
            // Create a copy of the address based on its type
            if (addr->getFamily() == netd::shared::Address::Family::IPv4) {
                auto* ipv4 = dynamic_cast<const netd::shared::IPv4Address*>(addr.get());
                if (ipv4) {
                    addresses_.push_back(std::make_unique<netd::shared::IPv4Address>(
                        ipv4->getAddress(), ipv4->getPrefixLength()));
                }
            } else if (addr->getFamily() == netd::shared::Address::Family::IPv6) {
                auto* ipv6 = dynamic_cast<const netd::shared::IPv6Address*>(addr.get());
                if (ipv6) {
                    addresses_.push_back(std::make_unique<netd::shared::IPv6Address>(
                        ipv6->getAddress(), ipv6->getPrefixLength()));
                }
            }
        }
    }
}

ServiceInfo& ServiceInfo::operator=(const ServiceInfo& other) {
    if (this != &other) {
        service_name_ = other.service_name_;
        service_type_ = other.service_type_;
        hostname_ = other.hostname_;
        service_data_ = other.service_data_;
        interface_name_ = other.interface_name_;
        additional_info_ = other.additional_info_;
        last_seen_ = other.last_seen_;
        
        // Clear existing addresses
        addresses_.clear();
        
        // Deep copy addresses
        for (const auto& addr : other.addresses_) {
            if (addr) {
                // Create a copy of the address based on its type
                if (addr->getFamily() == netd::shared::Address::Family::IPv4) {
                    auto* ipv4 = dynamic_cast<const netd::shared::IPv4Address*>(addr.get());
                    if (ipv4) {
                        addresses_.push_back(std::make_unique<netd::shared::IPv4Address>(
                            ipv4->getAddress(), ipv4->getPrefixLength()));
                    }
                } else if (addr->getFamily() == netd::shared::Address::Family::IPv6) {
                    auto* ipv6 = dynamic_cast<const netd::shared::IPv6Address*>(addr.get());
                    if (ipv6) {
                        addresses_.push_back(std::make_unique<netd::shared::IPv6Address>(
                            ipv6->getAddress(), ipv6->getPrefixLength()));
                    }
                }
            }
        }
    }
    return *this;
}

void ServiceInfo::addAddress(std::unique_ptr<netd::shared::Address> address) {
    if (address) {
        addresses_.push_back(std::move(address));
    }
}

std::vector<std::string> ServiceInfo::getAddressStrings() const {
    std::vector<std::string> address_strings;
    for (const auto& addr : addresses_) {
        if (addr) {
            address_strings.push_back(addr->getString());
        }
    }
    return address_strings;
}

bool ServiceInfo::isValid() const {
    return !service_name_.empty() && !hostname_.empty();
}

} // namespace netd::shared::lldp
