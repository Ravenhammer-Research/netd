/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "netd.h"
#include <sys/types.h>
#include <fcntl.h>
#include <sys/un.h>
#include <net/if.h>
#include <net/if_dl.h> // For struct sockaddr_dl
#include <net/if_bridgevar.h>
#include <net/if_vlan_var.h>
#include <net80211/ieee80211_ioctl.h>
#include <sys/sockio.h> // For SIOCSIFFIB
#include <net/if_mib.h> // For struct ifmibdata
#include <ifaddrs.h> // For getifaddrs, freeifaddrs, struct ifaddrs
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/queue.h>
#include <sys/sysctl.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if_types.h>
#include <net/if_var.h>
#include <time.h>
#include <sys/module.h>
#include <sys/linker.h>

/**
 * Enumerate system interfaces and populate interface list
 * @param interfaces Interface list to populate
 * @return 0 on success, -1 on failure
 */
int freebsd_enumerate_interfaces(netd_state_t *state) {
  struct ifaddrs *ifap, *ifa;
  interface_t *iface;
  char *last_name = NULL;

  if (!state) {
    return -1;
  }

  /* Get interface addresses */
  if (getifaddrs(&ifap) != 0) {
    debug_log(DEBUG_ERROR, "Failed to get interface addresses: %s",
              strerror(errno));
    return -1;
  }

  /* Iterate through interfaces */
  for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
    /* Skip if we've already processed this interface name */
    if (last_name && strcmp(last_name, ifa->ifa_name) == 0) {
      continue;
    }

    /* Skip interfaces without addresses, except for certain virtual interface
     * types */
    if (!ifa->ifa_addr) {
      /* For virtual interfaces like tap, tun, bridge, etc., we want to include
       * them even without addresses */
      if (strncmp(ifa->ifa_name, "tap", 3) == 0 ||
          strncmp(ifa->ifa_name, "tun", 3) == 0 ||
          strncmp(ifa->ifa_name, "bridge", 6) == 0 ||
          strncmp(ifa->ifa_name, "vlan", 4) == 0 ||
          strncmp(ifa->ifa_name, "vxlan", 5) == 0 ||
          strncmp(ifa->ifa_name, "gif", 3) == 0 ||
          strncmp(ifa->ifa_name, "gre", 3) == 0 ||
          strncmp(ifa->ifa_name, "lagg", 4) == 0 ||
          strncmp(ifa->ifa_name, "epair", 5) == 0) {
        /* Continue processing these virtual interfaces even without addresses
         */
        debug_log(DEBUG_DEBUG, "Including virtual interface %s without address",
                  ifa->ifa_name);
      } else {
        debug_log(DEBUG_DEBUG, "Skipping interface %s without address",
                  ifa->ifa_name);
        continue;
      }
    }

    /* Skip loopback interface if it's not the main loopback */
    if (strcmp(ifa->ifa_name, "lo0") != 0 &&
        strncmp(ifa->ifa_name, "lo", 2) == 0) {
      continue;
    }

    /* Determine interface type based on name */
    interface_type_t type = IF_TYPE_UNKNOWN;
    if (strncmp(ifa->ifa_name, "em", 2) == 0 ||
        strncmp(ifa->ifa_name, "igb", 3) == 0 ||
        strncmp(ifa->ifa_name, "ix", 2) == 0 ||
        strncmp(ifa->ifa_name, "bge", 3) == 0 ||
        strncmp(ifa->ifa_name, "fxp", 3) == 0 ||
        strncmp(ifa->ifa_name, "re", 2) == 0 ||
        strncmp(ifa->ifa_name, "rl", 2) == 0 ||
        strncmp(ifa->ifa_name, "sk", 2) == 0 ||
        strncmp(ifa->ifa_name, "ti", 2) == 0 ||
        strncmp(ifa->ifa_name, "tx", 2) == 0 ||
        strncmp(ifa->ifa_name, "vr", 2) == 0 ||
        strncmp(ifa->ifa_name, "xl", 2) == 0) {
      type = IF_TYPE_ETHERNET;
    } else if (strncmp(ifa->ifa_name, "wlan", 4) == 0 ||
               strncmp(ifa->ifa_name, "ath", 3) == 0 ||
               strncmp(ifa->ifa_name, "iwn", 3) == 0 ||
               strncmp(ifa->ifa_name, "iwm", 3) == 0 ||
               strncmp(ifa->ifa_name, "iwl", 3) == 0 ||
               strncmp(ifa->ifa_name, "bwi", 3) == 0 ||
               strncmp(ifa->ifa_name, "rum", 3) == 0 ||
               strncmp(ifa->ifa_name, "run", 3) == 0 ||
               strncmp(ifa->ifa_name, "ural", 4) == 0 ||
               strncmp(ifa->ifa_name, "urtw", 4) == 0 ||
               strncmp(ifa->ifa_name, "zyd", 3) == 0) {
      type = IF_TYPE_WIRELESS;
    } else if (strncmp(ifa->ifa_name, "epair", 5) == 0) {
      type = IF_TYPE_EPAIR;
    } else if (strncmp(ifa->ifa_name, "gif", 3) == 0) {
      type = IF_TYPE_GIF;
    } else if (strncmp(ifa->ifa_name, "gre", 3) == 0) {
      type = IF_TYPE_GRE;
    } else if (strncmp(ifa->ifa_name, "lagg", 4) == 0) {
      type = IF_TYPE_LAGG;
    } else if (strcmp(ifa->ifa_name, "lo0") == 0) {
      type = IF_TYPE_LOOPBACK;
    } else if (strncmp(ifa->ifa_name, "ovpn", 4) == 0) {
      type = IF_TYPE_OVPN;
    } else if (strncmp(ifa->ifa_name, "tun", 3) == 0) {
      type = IF_TYPE_TUN;
    } else if (strncmp(ifa->ifa_name, "tap", 3) == 0) {
      type = IF_TYPE_TAP;
    } else if (strncmp(ifa->ifa_name, "vlan", 4) == 0) {
      type = IF_TYPE_VLAN;
    } else if (strncmp(ifa->ifa_name, "vxlan", 5) == 0) {
      type = IF_TYPE_VXLAN;
    } else if (strncmp(ifa->ifa_name, "bridge", 6) == 0) {
      type = IF_TYPE_BRIDGE;
      debug_log(DEBUG_TRACE, "Processing bridge interface: %s", ifa->ifa_name);
    }

    /* Allocate new interface */
    iface = malloc(sizeof(*iface));
    if (!iface) {
      debug_log(DEBUG_ERROR, "Failed to allocate memory for interface %s",
                ifa->ifa_name);
      continue;
    }

    /* Initialize interface */
    memset(iface, 0, sizeof(*iface));
    strlcpy(iface->name, ifa->ifa_name, sizeof(iface->name));
    iface->type = type;
    iface->fib = 0; /* Default FIB */
    iface->group_count = 0;
    iface->enabled = (ifa->ifa_flags & IFF_UP) != 0;
    iface->flags = ifa->ifa_flags;
    iface->mtu = 0;                  /* Will be set below */
    iface->bridge_members[0] = '\0'; /* Initialize bridge members to empty */

    /* Get additional interface information using system.c functions */

    /* Get FIB */
    if (freebsd_interface_get_fib(ifa->ifa_name, &iface->fib) == 0) {
      debug_log(DEBUG_TRACE, "Found FIB for %s: %u", ifa->ifa_name, iface->fib);
    } else {
      debug_log(DEBUG_DEBUG, "Failed to get FIB for %s", ifa->ifa_name);
    }

    /* Get MTU */
    if (freebsd_interface_get_mtu(ifa->ifa_name, &iface->mtu) == 0) {
      debug_log(DEBUG_TRACE, "Found MTU for %s: %d", ifa->ifa_name, iface->mtu);
    } else {
      debug_log(DEBUG_DEBUG, "Failed to get MTU for %s", ifa->ifa_name);
    }

    /* Get group information */
    if (freebsd_interface_get_groups(ifa->ifa_name, iface->groups,
                                     MAX_GROUPS_PER_IF,
                                     &iface->group_count) == 0) {
      debug_log(DEBUG_TRACE, "Found %d groups for %s", iface->group_count,
                ifa->ifa_name);
    } else {
      debug_log(DEBUG_DEBUG, "Failed to get groups for %s", ifa->ifa_name);
    }

    /* Get bridge member information for bridge interfaces */
    if (type == IF_TYPE_BRIDGE) {
      freebsd_get_bridge_members(ifa->ifa_name, iface->bridge_members,
                                 sizeof(iface->bridge_members));
    }

    /* Get VLAN information for VLAN interfaces */
    if (type == IF_TYPE_VLAN || strchr(ifa->ifa_name, '.') != NULL) {
      freebsd_get_vlan_info(ifa->ifa_name, &iface->vlan_id, iface->vlan_proto,
                            sizeof(iface->vlan_proto), &iface->vlan_pcp,
                            iface->vlan_parent, sizeof(iface->vlan_parent));
      debug_log(DEBUG_TRACE,
                "Found VLAN info for %s: id=%d, proto=%s, pcp=%d, parent=%s",
                ifa->ifa_name, iface->vlan_id, iface->vlan_proto,
                iface->vlan_pcp, iface->vlan_parent);
    }

    /* Get WiFi information for wireless interfaces */
    if (type == IF_TYPE_WIRELESS) {
      freebsd_get_wifi_info(
          ifa->ifa_name, iface->wifi_regdomain, sizeof(iface->wifi_regdomain),
          iface->wifi_country, sizeof(iface->wifi_country),
          iface->wifi_authmode, sizeof(iface->wifi_authmode),
          iface->wifi_privacy, sizeof(iface->wifi_privacy),
          &iface->wifi_txpower, &iface->wifi_bmiss, &iface->wifi_scanvalid,
          iface->wifi_features, sizeof(iface->wifi_features),
          &iface->wifi_bintval, iface->wifi_parent, sizeof(iface->wifi_parent));
      debug_log(DEBUG_TRACE,
                "Found WiFi info for %s: regdomain=%s, country=%s, "
                "authmode=%s, privacy=%s, txpower=%d, bmiss=%d, scanvalid=%d, "
                "features=%s, bintval=%d, parent=%s",
                ifa->ifa_name, iface->wifi_regdomain, iface->wifi_country,
                iface->wifi_authmode, iface->wifi_privacy, iface->wifi_txpower,
                iface->wifi_bmiss, iface->wifi_scanvalid, iface->wifi_features,
                iface->wifi_bintval, iface->wifi_parent);
    }

    /* Get all addresses */
    struct ifaddrs *ifa_addr;
    bool primary_ipv4_found = false;
    bool primary_ipv6_found = false;

    for (ifa_addr = ifap; ifa_addr; ifa_addr = ifa_addr->ifa_next) {
      if (strcmp(ifa_addr->ifa_name, ifa->ifa_name) == 0 &&
          ifa_addr->ifa_addr) {
        if (ifa_addr->ifa_addr->sa_family == AF_INET) {
          struct sockaddr_in *sin = (struct sockaddr_in *)ifa_addr->ifa_addr;
          char addr_str[64];
          inet_ntop(AF_INET, &sin->sin_addr, addr_str, sizeof(addr_str));

          if (!primary_ipv4_found) {
            /* First IPv4 address is primary */
            strncpy(iface->primary_address, addr_str,
                    sizeof(iface->primary_address) - 1);
            iface->primary_address[sizeof(iface->primary_address) - 1] = '\0';
            primary_ipv4_found = true;
            debug_log(DEBUG_TRACE, "Found primary IPv4 address for %s: %s",
                      ifa->ifa_name, iface->primary_address);
          } else if (iface->alias_count < 10) {
            /* Additional IPv4 addresses are aliases */
            strncpy(iface->alias_addresses[iface->alias_count], addr_str,
                    sizeof(iface->alias_addresses[0]) - 1);
            iface->alias_addresses[iface->alias_count]
                                  [sizeof(iface->alias_addresses[0]) - 1] =
                '\0';
            iface->alias_count++;
            debug_log(DEBUG_TRACE, "Found IPv4 alias for %s: %s", ifa->ifa_name,
                      addr_str);
          }
        } else if (ifa_addr->ifa_addr->sa_family == AF_INET6) {
          struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ifa_addr->ifa_addr;
          char addr_str[64];
          inet_ntop(AF_INET6, &sin6->sin6_addr, addr_str, sizeof(addr_str));

          if (!primary_ipv6_found) {
            /* First IPv6 address is primary */
            strncpy(iface->primary_address6, addr_str,
                    sizeof(iface->primary_address6) - 1);
            iface->primary_address6[sizeof(iface->primary_address6) - 1] = '\0';
            primary_ipv6_found = true;
            debug_log(DEBUG_TRACE, "Found primary IPv6 address for %s: %s",
                      ifa->ifa_name, iface->primary_address6);
          } else if (iface->alias_count6 < 10) {
            /* Additional IPv6 addresses are aliases */
            strncpy(iface->alias_addresses6[iface->alias_count6], addr_str,
                    sizeof(iface->alias_addresses6[0]) - 1);
            iface->alias_addresses6[iface->alias_count6]
                                   [sizeof(iface->alias_addresses6[0]) - 1] =
                '\0';
            iface->alias_count6++;
            debug_log(DEBUG_TRACE, "Found IPv6 alias for %s: %s", ifa->ifa_name,
                      addr_str);
          }
        }
      }
    }

    /* Add to interface list */
    TAILQ_INSERT_TAIL(&state->interfaces, iface, entries);

    if (strncmp(ifa->ifa_name, "bridge", 6) == 0) {
      debug_log(DEBUG_TRACE, "Added bridge interface to list: %s",
                ifa->ifa_name);
    }

    debug_log(DEBUG_TRACE,
              "Added system interface %s (type: %s, enabled: %s, fib: %u, mtu: "
              "%d, addr: %s, addr6: %s, groups: %d)",
              ifa->ifa_name, interface_type_to_string(type),
              iface->enabled ? "yes" : "no", iface->fib, iface->mtu,
              iface->primary_address[0] ? iface->primary_address : "none",
              iface->primary_address6[0] ? iface->primary_address6 : "none",
              iface->group_count);

    last_name = strdup(ifa->ifa_name);
  }

  freeifaddrs(ifap);
  if (last_name) {
    free(last_name);
  }

  return 0;
}

/**
 * Check if an interface name represents a FreeBSD hardware interface
 * @param name Interface name to check
 * @return true if it's a hardware interface, false otherwise
 */
bool freebsd_is_hardware_interface(const char *name) {
  if (!name) {
    return false;
  }

  /* Check for FreeBSD hardware interface names */
  if (strncmp(name, "em", 2) == 0 || strncmp(name, "igb", 3) == 0 ||
      strncmp(name, "ix", 2) == 0 || strncmp(name, "bge", 3) == 0 ||
      strncmp(name, "fxp", 3) == 0 || strncmp(name, "re", 2) == 0 ||
      strncmp(name, "rl", 2) == 0 || strncmp(name, "sk", 2) == 0 ||
      strncmp(name, "ti", 2) == 0 || strncmp(name, "tx", 2) == 0 ||
      strncmp(name, "vr", 2) == 0 || strncmp(name, "xl", 2) == 0 ||
      strncmp(name, "wlan", 4) == 0 || strncmp(name, "ath", 3) == 0 ||
      strncmp(name, "iwn", 3) == 0 || strncmp(name, "iwm", 3) == 0 ||
      strncmp(name, "iwl", 3) == 0 || strncmp(name, "bwi", 3) == 0 ||
      strncmp(name, "rum", 3) == 0 || strncmp(name, "run", 3) == 0 ||
      strncmp(name, "ural", 4) == 0 || strncmp(name, "urtw", 4) == 0 ||
      strncmp(name, "zyd", 3) == 0 || strcmp(name, "lo0") == 0) {
    return true;
  }

  return false;
} 