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

#include <arpa/inet.h>
#include <netinet/in.h>
#include <shared/include/lldp/chassis.hpp>

namespace netd::shared::lldp {

  Chassis::Chassis(lldpctl_atom_t *chassis_atom) : chassis_atom_(chassis_atom) {
    if (chassis_atom_) {
      lldpctl_atom_inc_ref(chassis_atom_);
    }
  }

  Chassis::~Chassis() {
    if (chassis_atom_) {
      lldpctl_atom_dec_ref(chassis_atom_);
    }
  }

  std::string Chassis::getChassisId() const {
    return getStringValue(lldpctl_k_chassis_id);
  }

  std::string Chassis::getChassisName() const {
    return getStringValue(lldpctl_k_chassis_name);
  }

  std::string Chassis::getChassisDescription() const {
    return getStringValue(lldpctl_k_chassis_descr);
  }

  int Chassis::getCapabilitiesAvailable() const {
    return getIntValue(lldpctl_k_chassis_cap_available);
  }

  int Chassis::getCapabilitiesEnabled() const {
    return getIntValue(lldpctl_k_chassis_cap_enabled);
  }

  std::vector<std::unique_ptr<netd::shared::Address>>
  Chassis::getManagementAddresses() const {
    std::vector<std::unique_ptr<netd::shared::Address>> addresses;

    if (!chassis_atom_) {
      return addresses;
    }

    lldpctl_atom_t *mgmt_addrs =
        lldpctl_atom_get(chassis_atom_, lldpctl_k_chassis_mgmt);
    if (!mgmt_addrs) {
      return addresses;
    }

    lldpctl_atom_t *mgmt_addr;
    lldpctl_atom_foreach(mgmt_addrs, mgmt_addr) {
      const char *addr_data =
          lldpctl_atom_get_str(mgmt_addr, lldpctl_k_mgmt_ip);

      if (addr_data) {
        struct in_addr addr4;
        if (inet_pton(AF_INET, addr_data, &addr4) == 1) {
          auto ipv4_addr = std::make_unique<netd::shared::IPv4Address>(
              ntohl(addr4.s_addr), 32);
          addresses.push_back(std::move(ipv4_addr));
        } else {
          struct in6_addr addr6;
          if (inet_pton(AF_INET6, addr_data, &addr6) == 1) {
            auto ipv6_addr =
                std::make_unique<netd::shared::IPv6Address>(addr6.s6_addr, 128);
            addresses.push_back(std::move(ipv6_addr));
          }
        }
      }
    }

    lldpctl_atom_dec_ref(mgmt_addrs);
    return addresses;
  }

  bool Chassis::isValid() const {
    return chassis_atom_ != nullptr && !getChassisId().empty();
  }

  std::string Chassis::getStringValue(lldpctl_key_t key) const {
    if (!chassis_atom_) {
      return "";
    }

    const char *value = lldpctl_atom_get_str(chassis_atom_, key);
    return value ? std::string(value) : "";
  }

  int Chassis::getIntValue(lldpctl_key_t key) const {
    if (!chassis_atom_) {
      return 0;
    }

    return lldpctl_atom_get_int(chassis_atom_, key);
  }

} // namespace netd::shared::lldp
