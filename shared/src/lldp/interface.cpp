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

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <shared/include/address.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/lldp/interface.hpp>

namespace netd::shared::lldp {

  Interface::Interface(lldpctl_conn_t *connection) : connection_(connection) {}

  Interface::~Interface() {}

  std::vector<std::string> Interface::getLLDPInterfaces() const {
    std::vector<std::string> interfaces;

    if (!connection_) {
      throw netd::shared::LLDPError("LLDP connection not available");
    }

    lldpctl_atom_t *interfaces_atom = lldpctl_get_interfaces(connection_);
    if (!interfaces_atom) {
      lldpctl_error_t error = lldpctl_last_error(connection_);
      if (error == LLDPCTL_ERR_CANNOT_CONNECT) {
        throw netd::shared::LLDPError(
            "Cannot connect to LLDP daemon - daemon not available");
      } else {
        throw netd::shared::LLDPError(
            "Failed to get interfaces from LLDP daemon");
      }
    }

    lldpctl_atom_t *interface_atom;
    lldpctl_atom_foreach(interfaces_atom, interface_atom) {
      const char *interface_name =
          lldpctl_atom_get_str(interface_atom, lldpctl_k_interface_name);
      if (interface_name) {
        interfaces.push_back(std::string(interface_name));
      }
    }

    lldpctl_atom_dec_ref(interfaces_atom);
    return interfaces;
  }

  std::map<std::string, std::unique_ptr<netd::shared::Address>>
  Interface::getLinkLocalAddresses() const {
    std::map<std::string, std::unique_ptr<netd::shared::Address>>
        link_local_addrs;

    struct ifaddrs *ifaddrs_ptr, *ifa;
    if (getifaddrs(&ifaddrs_ptr) == -1) {
      return link_local_addrs;
    }

    for (ifa = ifaddrs_ptr; ifa != nullptr; ifa = ifa->ifa_next) {
      if (ifa->ifa_addr == nullptr) {
        continue;
      }

      std::string interface_name(ifa->ifa_name);

      // Check for IPv6 link-local addresses (fe80::/10)
      if (ifa->ifa_addr->sa_family == AF_INET6) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)ifa->ifa_addr;
        if (IN6_IS_ADDR_LINKLOCAL(&addr6->sin6_addr)) {
          // Create IPv6Address object with link-local prefix (64 bits)
          auto ipv6_addr = std::make_unique<netd::shared::IPv6Address>(
              addr6->sin6_addr.s6_addr, 64);
          link_local_addrs[interface_name] = std::move(ipv6_addr);
        }
      }
      // Check for IPv4 link-local addresses (169.254.0.0/16)
      else if (ifa->ifa_addr->sa_family == AF_INET) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)ifa->ifa_addr;
        uint32_t addr = ntohl(addr4->sin_addr.s_addr);
        // Check if it's in 169.254.0.0/16 range
        if ((addr & 0xFFFF0000) == 0xA9FE0000) {
          // Create IPv4Address object with /16 prefix for link-local
          auto ipv4_addr =
              std::make_unique<netd::shared::IPv4Address>(addr, 16);
          link_local_addrs[interface_name] = std::move(ipv4_addr);
        }
      }
    }

    freeifaddrs(ifaddrs_ptr);
    return link_local_addrs;
  }

} // namespace netd::shared::lldp
