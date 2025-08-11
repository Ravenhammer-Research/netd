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

#include <netd.h>
#include <interface/interface.h>
#include <vlan/vlan.h>
#include <80211/80211.h>
#include <bridge/bridge.h>
#include <lagg/lagg.h>
#include <gif/gif.h>
#include <epair/epair.h>
//#include <vxlan/vxlan.h>
//#include <gre/gre.h>
//#include <tun/tun.h>
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
    debug_log(ERROR, "Failed to get interface addresses: %s",
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
        debug_log(DEBUG, "Including virtual interface %s without address",
                  ifa->ifa_name);
      } else {
        debug_log(DEBUG, "Skipping interface %s without address",
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
              debug_log(DEBUG, "Processing bridge interface: %s", ifa->ifa_name);
    }

    /* Allocate new interface */
    iface = malloc(sizeof(*iface));
    if (!iface) {
      debug_log(ERROR, "Failed to allocate memory for interface %s",
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


    /* Get additional interface information using system.c functions */

    /* Get FIB */
    if (freebsd_interface_get_fib(ifa->ifa_name, &iface->fib) == 0) {
              debug_log(DEBUG, "Found FIB for %s: %u", ifa->ifa_name, iface->fib);
    } else {
      debug_log(DEBUG, "Failed to get FIB for %s", ifa->ifa_name);
    }

    /* Get MTU */
    if (freebsd_interface_get_mtu(ifa->ifa_name, &iface->mtu) == 0) {
              debug_log(DEBUG, "Found MTU for %s: %d", ifa->ifa_name, iface->mtu);
    } else {
      debug_log(DEBUG, "Failed to get MTU for %s", ifa->ifa_name);
    }

    /* Get group information */
    if (freebsd_interface_get_groups(ifa->ifa_name, iface->groups,
                                     MAX_GROUPS_PER_IF,
                                     &iface->group_count) == 0) {
              debug_log(DEBUG, "Found %d groups for %s", iface->group_count,
                 ifa->ifa_name);
    } else {
      debug_log(DEBUG, "Failed to get groups for %s", ifa->ifa_name);
    }

    /* Get bridge member information for bridge interfaces */
    if (type == IF_TYPE_BRIDGE) {
      debug_log(DEBUG, "Getting bridge members for %s", ifa->ifa_name);
      
      /* Create bridge struct and populate it */
      bridge_t *bridge = calloc(1, sizeof(bridge_t));
      if (bridge) {
        strlcpy(bridge->name, ifa->ifa_name, MAX_IFNAME_LEN);
        bridge->member_count = 0;
        bridge->maxaddr = 1000;
        bridge->timeout = 1200;
        strlcpy(bridge->protocol, "stp", sizeof(bridge->protocol));
        
        if (freebsd_get_bridge_members_array(ifa->ifa_name, bridge->members,
                                             MAX_BRIDGE_MEMBERS, &bridge->member_count) == 0) {
          debug_log(DEBUG, "Found %d bridge members for %s", bridge->member_count, ifa->ifa_name);
        } else {
          debug_log(DEBUG, "Failed to get bridge members for %s", ifa->ifa_name);
        }
        
        TAILQ_INSERT_TAIL(&state->bridges, bridge, entries);
      } else {
        debug_log(ERROR, "Failed to allocate bridge struct for %s", ifa->ifa_name);
      }
    }

    /* Get VLAN information for VLAN interfaces */
    if (type == IF_TYPE_VLAN || strchr(ifa->ifa_name, '.') != NULL) {
      debug_log(DEBUG, "Getting VLAN info for %s", ifa->ifa_name);
      
      /* Create VLAN struct and populate it */
      vlan_t *vlan = calloc(1, sizeof(vlan_t));
      if (vlan) {
        strlcpy(vlan->name, ifa->ifa_name, MAX_IFNAME_LEN);
        vlan->vlan_id = 0;
        vlan->vlan_pcp = 0;
        strlcpy(vlan->vlan_proto, "802.1Q", sizeof(vlan->vlan_proto));
        vlan->vlan_parent[0] = '\0';
        
        freebsd_vlan_show(ifa->ifa_name, &vlan->vlan_id, vlan->vlan_proto,
                          sizeof(vlan->vlan_proto), &vlan->vlan_pcp,
                          vlan->vlan_parent, sizeof(vlan->vlan_parent));
        
        debug_log(DEBUG2,
                  "Found VLAN info for %s: id=%d, proto=%s, pcp=%d, parent=%s",
                  ifa->ifa_name, vlan->vlan_id, vlan->vlan_proto,
                  vlan->vlan_pcp, vlan->vlan_parent);
        
        TAILQ_INSERT_TAIL(&state->vlans, vlan, entries);
      } else {
        debug_log(ERROR, "Failed to allocate VLAN struct for %s", ifa->ifa_name);
      }
    }

    /* Get epair information for epair interfaces */
    if (type == IF_TYPE_EPAIR) {
      debug_log(DEBUG, "Getting epair info for %s", ifa->ifa_name);
      
      /* Create epair struct and populate it */
      epair_t *epair = calloc(1, sizeof(epair_t));
      if (epair) {
        strlcpy(epair->name, ifa->ifa_name, MAX_IFNAME_LEN);
        epair->peer_name[0] = '\0';
        
        /* Get epair peer information */
        freebsd_epair_show(ifa->ifa_name, epair->peer_name, sizeof(epair->peer_name));
        
        debug_log(DEBUG2, "Found epair info for %s: peer=%s",
                  ifa->ifa_name, epair->peer_name);
        
        TAILQ_INSERT_TAIL(&state->epairs, epair, entries);
      } else {
        debug_log(ERROR, "Failed to allocate epair struct for %s", ifa->ifa_name);
      }
    }

    /* Get gif information for gif interfaces */
    if (type == IF_TYPE_GIF) {
      debug_log(DEBUG, "Getting gif info for %s", ifa->ifa_name);
      
      /* Create gif struct and populate it */
      gif_t *gif = calloc(1, sizeof(gif_t));
      if (gif) {
        strlcpy(gif->name, ifa->ifa_name, MAX_IFNAME_LEN);
        gif->tunnel_local[0] = '\0';
        gif->tunnel_remote[0] = '\0';
        
        /* Get gif tunnel information */
        freebsd_gif_show(ifa->ifa_name, gif->tunnel_local, sizeof(gif->tunnel_local),
                         gif->tunnel_remote, sizeof(gif->tunnel_remote));
        
        debug_log(DEBUG2, "Found gif info for %s: local=%s, remote=%s",
                  ifa->ifa_name, gif->tunnel_local, gif->tunnel_remote);
        
        TAILQ_INSERT_TAIL(&state->gifs, gif, entries);
      } else {
        debug_log(ERROR, "Failed to allocate gif struct for %s", ifa->ifa_name);
      }
    }

    /* Get lagg information for lagg interfaces */
    if (type == IF_TYPE_LAGG) {
      debug_log(DEBUG, "Getting lagg info for %s", ifa->ifa_name);
      
      /* Create lagg struct and populate it */
      lagg_t *lagg = calloc(1, sizeof(lagg_t));
      if (lagg) {
        strlcpy(lagg->name, ifa->ifa_name, MAX_IFNAME_LEN);
        lagg->lagg_proto[0] = '\0';
        lagg->member_count = 0;
        
        /* Get lagg information */
        freebsd_lagg_show(ifa->ifa_name, lagg->lagg_proto, sizeof(lagg->lagg_proto),
                          lagg->members, MAX_LAGG_MEMBERS, &lagg->member_count);
        
        debug_log(DEBUG2, "Found lagg info for %s: proto=%s, members=%d",
                  ifa->ifa_name, lagg->lagg_proto, lagg->member_count);
        
        TAILQ_INSERT_TAIL(&state->laggs, lagg, entries);
      } else {
        debug_log(ERROR, "Failed to allocate lagg struct for %s", ifa->ifa_name);
      }
    }

    /* Get WiFi information for wireless interfaces */
    if (type == IF_TYPE_WIRELESS) {
      debug_log(DEBUG, "Getting wifi info for %s", ifa->ifa_name);
      
      /* Create wifi struct and populate it */
      wifi_t *wifi = calloc(1, sizeof(wifi_t));
      if (wifi) {
        strlcpy(wifi->name, ifa->ifa_name, MAX_IFNAME_LEN);
        wifi->parent[0] = '\0';
        wifi->regdomain[0] = '\0';
        wifi->country[0] = '\0';
        wifi->authmode[0] = '\0';
        wifi->privacy[0] = '\0';
        wifi->features[0] = '\0';
        wifi->txpower = 0;
        wifi->bmiss = 0;
        wifi->scanvalid = 0;
        wifi->bintval = 0;
        
        /* Get wifi information */
        freebsd_wireless_show(
            ifa->ifa_name, wifi->regdomain, sizeof(wifi->regdomain),
            wifi->country, sizeof(wifi->country),
            wifi->authmode, sizeof(wifi->authmode),
            wifi->privacy, sizeof(wifi->privacy),
            &wifi->txpower, &wifi->bmiss, &wifi->scanvalid,
            wifi->features, sizeof(wifi->features),
            &wifi->bintval, wifi->parent, sizeof(wifi->parent));
        
        debug_log(DEBUG2,
                  "Found WiFi info for %s: regdomain=%s, country=%s, "
                  "authmode=%s, privacy=%s, txpower=%d, bmiss=%d, scanvalid=%d, "
                  "features=%s, bintval=%d, parent=%s",
                  ifa->ifa_name, wifi->regdomain, wifi->country,
                  wifi->authmode, wifi->privacy, wifi->txpower,
                  wifi->bmiss, wifi->scanvalid, wifi->features,
                  wifi->bintval, wifi->parent);
        
        TAILQ_INSERT_TAIL(&state->wifis, wifi, entries);
      } else {
        debug_log(ERROR, "Failed to allocate wifi struct for %s", ifa->ifa_name);
      }
    }

    /* Get all addresses */
    struct ifaddrs *ifa_addr;

    for (ifa_addr = ifap; ifa_addr; ifa_addr = ifa_addr->ifa_next) {
      if (strcmp(ifa_addr->ifa_name, ifa->ifa_name) == 0 &&
          ifa_addr->ifa_addr) {
        if (ifa_addr->ifa_addr->sa_family == AF_INET) {
          struct sockaddr_in *sin = (struct sockaddr_in *)ifa_addr->ifa_addr;
          char addr_str[64];
          inet_ntop(AF_INET, &sin->sin_addr, addr_str, sizeof(addr_str));

          /* Get prefix length from netmask */
          int prefix_length = 32; /* Default for host routes */
          if (ifa_addr->ifa_netmask) {
            struct sockaddr_in *mask = (struct sockaddr_in *)ifa_addr->ifa_netmask;
            prefix_length = get_prefix_length((struct sockaddr_storage *)mask);
          }

          if (iface->address_count < 11) {
            /* Add IPv4 address to array */
            strncpy(iface->addresses[iface->address_count].addr, addr_str,
                    sizeof(iface->addresses[0].addr) - 1);
            iface->addresses[iface->address_count].addr[sizeof(iface->addresses[0].addr) - 1] = '\0';
            iface->addresses[iface->address_count].prefixlen = prefix_length;
            iface->address_count++;
          }
        } else if (ifa_addr->ifa_addr->sa_family == AF_INET6) {
          struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ifa_addr->ifa_addr;
          char addr_str[64];
          inet_ntop(AF_INET6, &sin6->sin6_addr, addr_str, sizeof(addr_str));

          /* Get prefix length from netmask */
          int prefix_length = 128; /* Default for host routes */
          if (ifa_addr->ifa_netmask) {
            struct sockaddr_in6 *mask = (struct sockaddr_in6 *)ifa_addr->ifa_netmask;
            prefix_length = get_prefix_length((struct sockaddr_storage *)mask);
          }

          if (iface->address_count6 < 11) {
            /* Add IPv6 address to array */
            strncpy(iface->addresses6[iface->address_count6].addr, addr_str,
                    sizeof(iface->addresses6[0].addr) - 1);
            iface->addresses6[iface->address_count6].addr[sizeof(iface->addresses6[0].addr) - 1] = '\0';
            iface->addresses6[iface->address_count6].prefixlen = prefix_length;
            iface->address_count6++;
          }
        }
      }
    }

    /* Add to interface list */
    TAILQ_INSERT_TAIL(&state->interfaces, iface, entries);





    last_name = strdup(ifa->ifa_name);
  }

  freeifaddrs(ifap);
  if (last_name) {
    free(last_name);
  }

  return 0;
}

/**
 * Check if an interface type represents a FreeBSD hardware interface
 * @param type Interface type to check
 * @return true if it's a hardware interface, false otherwise
 */
bool freebsd_is_hardware_interface(interface_type_t type) {
  switch (type) {
  case IF_TYPE_ETHERNET:
  case IF_TYPE_WIRELESS:
  case IF_TYPE_LOOPBACK:
    return true;
  case IF_TYPE_EPAIR:
  case IF_TYPE_GIF:
  case IF_TYPE_GRE:
  case IF_TYPE_LAGG:
  case IF_TYPE_OVPN:
  case IF_TYPE_TUN:
  case IF_TYPE_TAP:
  case IF_TYPE_VLAN:
  case IF_TYPE_VXLAN:
  case IF_TYPE_BRIDGE:
  case IF_TYPE_UNKNOWN:
  default:
    return false;
  }
} 