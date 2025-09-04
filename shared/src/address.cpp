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

#include <shared/include/address.hpp>
#include <libyang/tree_data.h>

namespace netd {

Address::Address(const std::string& type, const std::string& data) 
    : type_(type), data_(data) {
}

lyd_node* Address::toYang(ly_ctx* ctx) const {
    // Base Address class doesn't have specific YANG representation
    // This should be overridden by concrete classes
    return nullptr;
}

Address Address::fromYang(const ly_ctx* ctx, const lyd_node* node) {
    // Base Address class doesn't have specific YANG representation
    // This should be overridden by concrete classes
    return Address("", "");
}

// IPv4Address implementation
IPv4Address::IPv4Address(uint32_t addr, uint8_t prefix)
    : address_(addr), prefixLength_(prefix) {
}

lyd_node* IPv4Address::toYang(ly_ctx* ctx) const {
    // TODO: Implement YANG serialization for IPv4 addresses
    // This should create a YANG node representing the IPv4 address
    return nullptr;
}

IPv4Address IPv4Address::fromYang(const ly_ctx* ctx, const lyd_node* node) {
    // TODO: Implement YANG deserialization for IPv4 addresses
    // This should parse a YANG node to extract IPv4 address information
    return IPv4Address();
}

std::string IPv4Address::getString() const {
    struct in_addr addr;
    addr.s_addr = htonl(address_);
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr, str, INET_ADDRSTRLEN);
    return std::string(str) + "/" + std::to_string(prefixLength_);
}

bool IPv4Address::isValid() const {
    return address_ != 0 && prefixLength_ <= 32;
}

// IPv6Address implementation
IPv6Address::IPv6Address(const uint8_t addr[16], uint8_t prefix)
    : prefixLength_(prefix) {
    setAddress(addr);
}

lyd_node* IPv6Address::toYang(ly_ctx* ctx) const {
    // TODO: Implement YANG serialization for IPv6 addresses
    // This should create a YANG node representing the IPv6 address
    return nullptr;
}

IPv6Address IPv6Address::fromYang(const ly_ctx* ctx, const lyd_node* node) {
    // TODO: Implement YANG deserialization for IPv6 addresses
    // This should parse a YANG node to extract IPv6 address information
    return IPv6Address();
}

std::string IPv6Address::getString() const {
    char str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, address_, str, INET6_ADDRSTRLEN);
    return std::string(str) + "/" + std::to_string(prefixLength_);
}

bool IPv6Address::isValid() const {
    // Check if address is not all zeros
    for (int i = 0; i < 16; i++) {
        if (address_[i] != 0) {
            return prefixLength_ <= 128;
        }
    }
    return false;
}

void IPv6Address::setAddress(const uint8_t addr[16]) {
    std::memcpy(address_, addr, 16);
}

} // namespace netd
