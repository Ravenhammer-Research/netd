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
    memset(&route->gateway, 0, sizeof(route->gateway));
    route->gateway.ss_family = AF_UNSPEC;
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
  
  /* Extract prefix length from netmask */
  if (netmask_addr.ss_family != AF_UNSPEC) {
    route->prefix_length = get_prefix_length(&netmask_addr);
  } else {
    route->prefix_length = 0;
  }
  
  /* Extract scope interface for IPv6 link-local addresses */
  if (dest_addr.ss_family == AF_INET6) {
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&dest_addr;
    if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr)) {
      /* For IPv6 link-local addresses, extract scope from the address */
      char addr_str[INET6_ADDRSTRLEN];
      if (inet_ntop(AF_INET6, &sin6->sin6_addr, addr_str, sizeof(addr_str))) {
        char *scope_ptr = strchr(addr_str, '%');
        if (scope_ptr) {
          scope_ptr++; /* Skip the '%' */
          strlcpy(route->scope_interface, scope_ptr, sizeof(route->scope_interface));
        } else {
          strlcpy(route->scope_interface, ifname, sizeof(route->scope_interface));
        }
      } else {
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
  /* Try to determine the number of FIBs by attempting to enumerate routes
   * for different FIB numbers. Start with 1 and increment until we fail. */
  uint32_t fib_count = 1;
  int mib[6];
  size_t needed;
  
  /* Use the same sysctl approach as the working route enumeration code */
  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;           /* protocol */
  mib[3] = AF_INET;     /* address family (use IPv4 for FIB counting) */
  mib[4] = NET_RT_DUMP; /* get all routes */
  
  /* Try FIB numbers starting from 0 */
  for (uint32_t test_fib = 0; test_fib < 256; test_fib++) { /* Reasonable upper limit */
    mib[5] = test_fib;  /* FIB number */
    
    /* Try to get the size needed for this FIB */
    if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0) {
      if (errno == EINVAL || errno == ENOTSUP) {
        /* This FIB doesn't exist, we've found the limit */
        fib_count = test_fib;
        break;
      } else if (errno == ENOENT) {
        /* This FIB exists but has no routes, continue */
        fib_count = test_fib + 1; /* Count it even if empty */
      } else {
        /* Some other error, assume this is the limit */
        fib_count = test_fib;
        break;
      }
    } else {
      /* This FIB exists and has routes */
      fib_count = test_fib + 1;
    }
  }
  
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
