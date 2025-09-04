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

#ifndef NETD_ADDRESS_HPP
#define NETD_ADDRESS_HPP

#include <string>
#include <memory>
#include <cstdint>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <shared/include/base/serialization.hpp>

namespace netd {

class Address : public base::Serialization<Address> {
public:
    enum class Family {
        IPv4,
        IPv6
    };

    Address() = default;
    explicit Address(const std::string& type, const std::string& data);
    virtual ~Address() = default;

    // Implement Serialization methods
    lyd_node* toYang() const override;
    static Address fromYang(const lyd_node* node);

    // Address properties
    virtual Family getFamily() const { return Family::IPv4; }
    virtual std::string getString() const { return ""; }
    virtual uint8_t getPrefixLength() const { return 0; }
    virtual bool isValid() const { return false; }

    // Getters for member variables
    std::string getType() const { return type_; }
    std::string getData() const { return data_; }

private:
    std::string type_;
    std::string data_;
};

// Concrete IPv4 Address class
class IPv4Address : public Address {
public:
    IPv4Address() = default;
    IPv4Address(uint32_t addr, uint8_t prefix = 32);
    virtual ~IPv4Address() = default;

    // Implement Address methods
    lyd_node* toYang() const override;
    static IPv4Address fromYang(const lyd_node* node);

    Family getFamily() const override { return Family::IPv4; }
    std::string getString() const override;
    uint8_t getPrefixLength() const override { return prefixLength_; }
    bool isValid() const override;

    // IPv4-specific methods
    uint32_t getAddress() const { return address_; }
    void setAddress(uint32_t addr) { address_ = addr; }
    void setPrefixLength(uint8_t prefix) { prefixLength_ = prefix; }

private:
    uint32_t address_{0};
    uint8_t prefixLength_{32};
};

// Concrete IPv6 Address class
class IPv6Address : public Address {
public:
    IPv6Address() = default;
    IPv6Address(const uint8_t addr[16], uint8_t prefix = 128);
    virtual ~IPv6Address() = default;

    // Implement Address methods
    lyd_node* toYang() const override;
    static IPv6Address fromYang(const lyd_node* node);

    Family getFamily() const override { return Family::IPv6; }
    std::string getString() const override;
    uint8_t getPrefixLength() const override { return prefixLength_; }
    bool isValid() const override;

    // IPv6-specific methods
    const uint8_t* getAddress() const { return address_; }
    void setAddress(const uint8_t addr[16]);
    void setPrefixLength(uint8_t prefix) { prefixLength_ = prefix; }

private:
    uint8_t address_[16]{0};
    uint8_t prefixLength_{128};
};

} // namespace netd

#endif // NETD_ADDRESS_HPP
