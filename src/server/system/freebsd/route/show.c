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
#include <route.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet6/in6_var.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <net/if_types.h> // IFT_ETHER

/* Interface mapping structure */
struct ifmap_entry {
  char ifname[IFNAMSIZ];
  uint32_t mtu;
};

/* Mask to length table for IPv6 prefix calculation */
static const u_char masktolen[256] = {
  [0xff] = 8 + 1,
  [0xfe] = 7 + 1,
  [0xfc] = 6 + 1,
  [0xf8] = 5 + 1,
  [0xf0] = 4 + 1,
  [0xe0] = 3 + 1,
  [0xc0] = 2 + 1,
  [0x80] = 1 + 1,
  [0x00] = 0 + 1,
};

/* Global interface map */
static struct ifmap_entry *ifmap = NULL;
static size_t ifmap_size = 0;

/**
 * Prepare interface map for index to name mapping
 * @param pifmap_size Pointer to store the size of the interface map
 * @return Pointer to the interface map, or NULL on failure
 */
static struct ifmap_entry *prepare_ifmap(size_t *pifmap_size) {
  int ifindex = 0, size;
  struct ifaddrs *ifap, *ifa;
  struct sockaddr_dl *sdl;
  struct ifmap_entry *ifmap = NULL;
  int ifmap_size = 0;

  /* Retrieve interface list to create index to name mapping */
  if (getifaddrs(&ifap) != 0) {
    return NULL;
  }

  for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr->sa_family != AF_LINK) {
      continue;
    }

    sdl = (struct sockaddr_dl *)ifa->ifa_addr;
    ifindex = sdl->sdl_index;

    if (ifindex >= ifmap_size) {
      size = roundup2(ifindex + 1, 32) * sizeof(struct ifmap_entry);
      if ((ifmap = realloc(ifmap, size)) == NULL) {
        freeifaddrs(ifap);
        return NULL;
      }
      memset(&ifmap[ifmap_size], 0,
          size - ifmap_size * sizeof(struct ifmap_entry));
      ifmap_size = roundup2(ifindex + 1, 32);
    }

    if (*ifmap[ifindex].ifname != '\0') {
      continue;
    }

    strlcpy(ifmap[ifindex].ifname, ifa->ifa_name, IFNAMSIZ);
  }

  freeifaddrs(ifap);
  *pifmap_size = ifmap_size;
  return ifmap;
}

/**
 * Fill scope ID for IPv6 addresses
 * @param sa6 IPv6 socket address
 */
static void in6_fillscopeid(struct sockaddr_in6 *sa6) {
  if (IN6_IS_ADDR_LINKLOCAL(&sa6->sin6_addr) ||
      IN6_IS_ADDR_MC_NODELOCAL(&sa6->sin6_addr) ||
      IN6_IS_ADDR_MC_LINKLOCAL(&sa6->sin6_addr)) {
    if (sa6->sin6_scope_id == 0) {
      sa6->sin6_scope_id = ntohs(*(u_int16_t *)&sa6->sin6_addr.s6_addr[2]);
    }
    sa6->sin6_addr.s6_addr[2] = sa6->sin6_addr.s6_addr[3] = 0;
  }
}

/**
 * Calculate IPv6 prefix length from netmask
 * @param mask IPv6 netmask address
 * @return Prefix length
 */
static int get_ipv6_prefix_length(const struct sockaddr_in6 *mask) {
  u_char *p, *lim;
  u_char masklen = 0;
  bool illegal = false;

  if (!mask) {
    return 128; /* Default to /128 for host routes */
  }

  p = (u_char *)&mask->sin6_addr;
  for (lim = p + 16; p < lim; p++) {
    if (masktolen[*p] > 0) {
      /* -1 is required */
      masklen += (masktolen[*p] - 1);
    } else {
      illegal = true;
    }
  }

  if (illegal) {
    return -1; /* Invalid netmask */
  }

  return masklen;
}

/**
 * Get interface name from route message index
 * @param rtm_index Route message interface index
 * @param ifname Buffer to store interface name
 * @param ifname_len Length of the buffer
 */
static void get_interface_name(uint32_t rtm_index, char *ifname, size_t ifname_len) {
  if (rtm_index < ifmap_size && ifmap != NULL) {
    strlcpy(ifname, ifmap[rtm_index].ifname, ifname_len);
    if (*ifname == '\0') {
      strlcpy(ifname, "---", ifname_len);
    }
  } else {
    snprintf(ifname, ifname_len, "link#%d", rtm_index);
  }
}

/**
 * Parse a route message and add it to state
 * @param state Server state
 * @param rtm Route message header
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
static int parse_route_message(netd_state_t *state, struct rt_msghdr *rtm, uint32_t fib) {
  char *cp;
  struct sockaddr *sp[RTAX_MAX];
  struct sockaddr_storage dest_addr, gw_addr, netmask_addr;
  char ifname[IFNAMSIZ];
  netd_route_t *route;

  if (!state || !rtm) {
    return -1;
  }

  /* Initialize */
  memset(&dest_addr, 0, sizeof(dest_addr));
  memset(&gw_addr, 0, sizeof(gw_addr));
  memset(&netmask_addr, 0, sizeof(netmask_addr));
  memset(ifname, 0, sizeof(ifname));

  /* Parse addresses from the message */
  cp = (char *)(rtm + 1);
  for (int i = 0; i < RTAX_MAX; i++) {
    if (rtm->rtm_addrs & (1 << i)) {
      sp[i] = (struct sockaddr *)cp;
      if (sp[i]->sa_len == 0) {
        cp += sizeof(struct sockaddr);
        continue;
      }

      switch (i) {
      case RTAX_DST:
        memcpy(&dest_addr, sp[i], sp[i]->sa_len);
        break;
      case RTAX_GATEWAY:
        memcpy(&gw_addr, sp[i], sp[i]->sa_len);
        break;
      case RTAX_NETMASK:
        memcpy(&netmask_addr, sp[i], sp[i]->sa_len);
        break;
      case RTAX_IFP:
        /* Fallback to link address if interface map is not available */
        if (sp[i]->sa_family == AF_LINK) {
          struct sockaddr_dl *sdl = (struct sockaddr_dl *)sp[i];
          if (sdl->sdl_nlen > 0) {
            size_t copy_len = (sdl->sdl_nlen < sizeof(ifname) - 1) ? sdl->sdl_nlen : sizeof(ifname) - 1;
            memcpy(ifname, sdl->sdl_data, copy_len);
            ifname[copy_len] = '\0';
          } else if (sdl->sdl_index > 0) {
            snprintf(ifname, sizeof(ifname), "link#%d", sdl->sdl_index);
          }
        }
        break;
      }
      cp += sp[i]->sa_len;
    }
  }

  /* Get interface name from route message index (primary method) */
  if (ifmap != NULL) {
    get_interface_name(rtm->rtm_index, ifname, sizeof(ifname));
  }

  /* Allocate new route */
  route = malloc(sizeof(*route));
  if (!route) {
    return -1;
  }

  /* Initialize route */
  memset(route, 0, sizeof(*route));
  
  /* Copy addresses */
  memcpy(&route->destination, &dest_addr, sizeof(dest_addr));
  
  /* Copy gateway address */
  if (gw_addr.ss_family != AF_UNSPEC) {
    memcpy(&route->gateway, &gw_addr, sizeof(gw_addr));
  } else {
    /* No explicit gateway - for direct routes, use the interface as gateway */
    if (rtm->rtm_index > 0) {
      /* Create a link address for the interface */
      struct sockaddr_dl *sdl = (struct sockaddr_dl *)&route->gateway;
      memset(sdl, 0, sizeof(*sdl));
      sdl->sdl_len = sizeof(*sdl);
      sdl->sdl_family = AF_LINK;
      sdl->sdl_index = rtm->rtm_index;
      sdl->sdl_type = IFT_ETHER; /* Default type */
    } else {
      /* No interface index - leave as AF_UNSPEC */
      memset(&route->gateway, 0, sizeof(route->gateway));
      route->gateway.ss_family = AF_UNSPEC;
    }
  }
  
  /* Copy netmask address */
  if (netmask_addr.ss_family != AF_UNSPEC) {
    memcpy(&route->netmask, &netmask_addr, sizeof(netmask_addr));
  } else {
    memset(&route->netmask, 0, sizeof(route->netmask));
    route->netmask.ss_family = AF_UNSPEC;
  }
  
  strlcpy(route->interface, ifname, sizeof(route->interface));
  route->fib = fib;
  route->flags = rtm->rtm_flags;
  
  /* Extract prefix length from netmask with improved IPv6 handling */
  if (netmask_addr.ss_family == AF_INET6) {
    struct sockaddr_in6 *sin6_mask = (struct sockaddr_in6 *)&netmask_addr;
    route->prefix_length = get_ipv6_prefix_length(sin6_mask);
    if (route->prefix_length < 0) {
      /* Check if this is a host route */
      if (rtm->rtm_flags & RTF_HOST) {
        route->prefix_length = 128; /* Host route */
      } else {
        route->prefix_length = 0; /* Default for network route without valid netmask */
      }
    }
  } else if (netmask_addr.ss_family == AF_INET) {
    route->prefix_length = get_prefix_length(&netmask_addr);
  } else {
    /* No netmask - determine prefix length based on route type */
    if (rtm->rtm_flags & RTF_HOST) {
      route->prefix_length = (dest_addr.ss_family == AF_INET6) ? 128 : 32;
    } else {
      route->prefix_length = 0;
    }
  }
  
  /* Improved IPv6 scope interface handling */
  if (dest_addr.ss_family == AF_INET6) {
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&dest_addr;
    
    /* Fill scope ID for IPv6 addresses */
    in6_fillscopeid(sin6);
    
    if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr) ||
        IN6_IS_ADDR_MC_NODELOCAL(&sin6->sin6_addr) ||
        IN6_IS_ADDR_MC_LINKLOCAL(&sin6->sin6_addr)) {
      
      if (sin6->sin6_scope_id != 0) {
        /* Use the scope ID to get interface name */
        get_interface_name(sin6->sin6_scope_id, route->scope_interface, sizeof(route->scope_interface));
      } else {
        /* Fallback to route interface */
        strlcpy(route->scope_interface, ifname, sizeof(route->scope_interface));
      }
    } else {
      route->scope_interface[0] = '\0';
    }
  } else {
    route->scope_interface[0] = '\0';
  }
  
  route->expire = 0;

  /* Add to route list */
  TAILQ_INSERT_TAIL(&state->routes, route, entries);

  return 0;
}

/**
 * List routes from the routing table and populate state
 * @param state Server state
 * @param fib FIB number
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int freebsd_route_list(netd_state_t *state, uint32_t fib, int family) {
  size_t needed;
  int mib[7];
  char *buf, *lim, *next;
  struct rt_msghdr *rtm;

  if (!state) {
    return -1;
  }

  /* Prepare interface map for proper interface name resolution */
  if (ifmap == NULL) {
    ifmap = prepare_ifmap(&ifmap_size);
    if (ifmap == NULL) {
      /* Continue without interface map, will use fallback methods */
      ifmap_size = 0;
    }
  }

  /* Use sysctl to get routing information for a specific FIB */
  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;
  mib[3] = family;
  mib[4] = NET_RT_DUMP;
  mib[5] = 0;
  mib[6] = fib;

  /* Get the size needed */
  if (sysctl(mib, nitems(mib), NULL, &needed, NULL, 0) < 0) {
    return -1;
  }

  /* Allocate buffer */
  buf = malloc(needed);
  if (!buf) {
    return -1;
  }

  /* Get the actual route data */
  if (sysctl(mib, nitems(mib), buf, &needed, NULL, 0) < 0) {
    free(buf);
    return -1;
  }

  /* Process all route messages */
  lim = buf + needed;
  for (next = buf; next < lim; next += rtm->rtm_msglen) {
    rtm = (struct rt_msghdr *)(void *)next;

    if (rtm->rtm_type == RTM_ADD || rtm->rtm_type == RTM_CHANGE || rtm->rtm_type == RTM_GET) {
      parse_route_message(state, rtm, fib);
    }
  }

  free(buf);
  return 0;
}

/**
 * Get the number of FIBs configured in the system
 * @return Number of FIBs, or 1 if unable to determine
 */
uint32_t get_system_fib_count(void) {
  /* Only count FIBs that are actually in use (have routes or interfaces).
   * Start with 1 (default VRF) and check for active FIBs. */
  uint32_t fib_count = 1; /* Always count FIB 0 (default VRF) */
  int mib[6];
  size_t needed;
  
  /* Use the same sysctl approach as the working route enumeration code */
  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;           /* protocol */
  mib[3] = AF_INET;     /* address family (use IPv4 for FIB counting) */
  mib[4] = NET_RT_DUMP; /* get all routes */
  
  /* Check FIBs 1-255 for actual usage */
  for (uint32_t test_fib = 1; test_fib < 256; test_fib++) {
    mib[5] = test_fib;  /* FIB number */
    
    /* Try to get the size needed for this FIB */
    if (sysctl(mib, 6, NULL, &needed, NULL, 0) == 0 && needed > 0) {
      /* This FIB exists and has routes, count it */
      fib_count = test_fib + 1;
      debug_log(DEBUG2, "Found active FIB %u with %zu bytes of route data", test_fib, needed);
    } else if (errno == EINVAL || errno == ENOTSUP) {
      /* This FIB doesn't exist, stop checking */
      debug_log(ERROR, "FIB %u doesn't exist, stopping FIB enumeration", test_fib);
      break;
    } else if (errno == ENOENT) {
      /* This FIB exists but has no routes, don't count it */
      debug_log(DEBUG, "FIB %u exists but has no routes, skipping", test_fib);
      continue;
    } else {
      /* Some other error, assume this is the limit */
      debug_log(ERROR, "Error checking FIB %u: %s, stopping enumeration", test_fib, strerror(errno));
      break;
    }
  }
  
  debug_log(INFO, "System has %u active FIBs (0-%u)", fib_count, fib_count - 1);
  return fib_count;
}

/**
 * Validate FIB number
 * @param fib FIB number
 * @return true if valid, false otherwise
 */
bool is_valid_fib_number(uint32_t fib) {
  uint32_t max_fibs = get_system_fib_count();
  return (fib < max_fibs);
}

/**
 * Clean up interface map resources
 */
void freebsd_route_cleanup(void) {
  if (ifmap != NULL) {
    free(ifmap);
    ifmap = NULL;
    ifmap_size = 0;
  }
}
