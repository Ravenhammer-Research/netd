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
#include <netinet6/in6_var.h> /* For IPv6 address macros */
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
 * Parse a FreeBSD route message and add it to the state
 * @param state Server state
 * @param rtm Route message header
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
static int parse_route_message(netd_state_t *state, struct rt_msghdr *rtm,
                               uint32_t fib) {
  char *cp;
  struct sockaddr *sp[RTAX_MAX];
  struct sockaddr_storage dest_addr, gw_addr, netmask_addr;
  char dest_str[INET6_ADDRSTRLEN];
  char gw_str[INET6_ADDRSTRLEN];
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
            /* Copy interface name with proper length limit */
            size_t copy_len = (sdl->sdl_nlen < sizeof(ifname) - 1) ? sdl->sdl_nlen : sizeof(ifname) - 1;
            memcpy(ifname, sdl->sdl_data, copy_len);
            ifname[copy_len] = '\0';
          } else if (sdl->sdl_index > 0) {
            /* If no interface name, use link#<index> format */
            snprintf(ifname, sizeof(ifname), "link#%d", sdl->sdl_index);
          }
        }
        break;
      }
      cp += sp[i]->sa_len;
    }
  }

  /* Format addresses */
  if (format_address(&dest_addr, dest_str, sizeof(dest_str)) < 0) {
    strlcpy(dest_str, "unknown", sizeof(dest_str));
  }

  if (gw_addr.ss_family != AF_UNSPEC) {
    if (format_address(&gw_addr, gw_str, sizeof(gw_str)) < 0) {
      strlcpy(gw_str, "unknown", sizeof(gw_str));
    }
  } else {
    strlcpy(gw_str, "direct", sizeof(gw_str));
  }

  /* Allocate new route */
  route = malloc(sizeof(*route));
  if (!route) {
    debug_log(ERROR, "Failed to allocate memory for route");
    return -1;
  }

  /* Initialize route */
  memset(route, 0, sizeof(*route));
  
  /* Copy addresses directly - no need to convert to string and back */
  memcpy(&route->destination, &dest_addr, sizeof(dest_addr));
  
  if (gw_addr.ss_family != AF_UNSPEC) {
    memcpy(&route->gateway, &gw_addr, sizeof(gw_addr));
  } else {
    /* For direct routes, set gateway to AF_UNSPEC */
    memset(&route->gateway, 0, sizeof(route->gateway));
    route->gateway.ss_family = AF_UNSPEC;
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
      /* For link-local addresses, use the interface name as scope */
      strlcpy(route->scope_interface, ifname, sizeof(route->scope_interface));
    } else {
      route->scope_interface[0] = '\0';
    }
  } else {
    route->scope_interface[0] = '\0';
  }
  
  /* Extract expiration time if available */
  route->expire = 0; /* TODO: Extract from rtm_rmx if available */

  /* Add to route list */
  TAILQ_INSERT_TAIL(&state->routes, route, entries);

  /* Format gateway string for debug output */
  if (gw_addr.ss_family == AF_UNSPEC) {
    strlcpy(gw_str, "direct", sizeof(gw_str));
  } else {
    if (format_address(&gw_addr, gw_str, sizeof(gw_str)) < 0) {
      strlcpy(gw_str, "unknown", sizeof(gw_str));
    }
  }
  
  debug_log(DEBUG2, "Added route: %s via %s on %s (FIB %u)", dest_str,
            gw_str, ifname, fib);

  return 0;
}

/**
 * List routes from the routing table
 * @param fib FIB number
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int freebsd_route_list(uint32_t fib, int family) {
  size_t needed;
  int mib[7];
  char *buf, *lim, *next;
  struct rt_msghdr *rtm;
  int retry_count = 0;
  const int max_retries = 3;

  /* Use sysctl to get all routing information for a specific FIB */
  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;           /* protocol */
  mib[3] = family;      /* address family */
  mib[4] = NET_RT_DUMP; /* get all routes */
  mib[5] = 0;           /* flags */
  mib[6] = fib;         /* FIB number */

retry:
  /* First, get the size needed */
  if (sysctl(mib, nitems(mib), NULL, &needed, NULL, 0) < 0) {
    debug_log(ERROR, "Failed to get route table size: %s",
              strerror(errno));
    return -1;
  }

  /* Allocate buffer */
  buf = malloc(needed);
  if (!buf) {
    debug_log(ERROR, "Failed to allocate route table buffer");
    return -1;
  }

  /* Get the actual route data */
  if (sysctl(mib, nitems(mib), buf, &needed, NULL, 0) < 0) {
    if (errno == ENOMEM && retry_count++ < max_retries) {
      debug_log(DEBUG, "Route table grew, retrying (attempt %d)",
                retry_count);
      free(buf);
      sleep(1);
      goto retry;
    }
    debug_log(ERROR, "Failed to get route table: %s", strerror(errno));
    free(buf);
    return -1;
  }

  /* Process all route messages */
  lim = buf + needed;
  for (next = buf; next < lim; next += rtm->rtm_msglen) {
    rtm = (struct rt_msghdr *)(void *)next;

    if (rtm->rtm_type == RTM_ADD || rtm->rtm_type == RTM_CHANGE ||
        rtm->rtm_type == RTM_GET) {
      /* Process route entry */
      debug_log(DEBUG, "Received route entry (type: %d)", rtm->rtm_type);

      /* Parse route information */
      char *cp = (char *)(rtm + 1);
      struct sockaddr *sa;
      struct sockaddr_storage dest_addr, gw_addr;
      char dest_str[INET6_ADDRSTRLEN];
      char gw_str[INET6_ADDRSTRLEN];
      char ifname[IFNAMSIZ];

      memset(&dest_addr, 0, sizeof(dest_addr));
      memset(&gw_addr, 0, sizeof(gw_addr));
      memset(ifname, 0, sizeof(ifname));

      /* Extract addresses from the message */
      for (int i = 0; i < RTAX_MAX; i++) {
        if (rtm->rtm_addrs & (1 << i)) {
          sa = (struct sockaddr *)cp;
          if (sa->sa_len == 0) {
            cp += sizeof(struct sockaddr);
            continue;
          }

          switch (i) {
          case RTAX_DST:
            memcpy(&dest_addr, sa, sa->sa_len);
            break;
          case RTAX_GATEWAY:
            memcpy(&gw_addr, sa, sa->sa_len);
            break;
          case RTAX_IFP:
            if (sa->sa_family == AF_LINK) {
              struct sockaddr_dl *sdl = (struct sockaddr_dl *)sa;
              if (sdl->sdl_nlen > 0) {
                strlcpy(ifname, sdl->sdl_data, sizeof(ifname));
              }
            }
            break;
          }
          cp += sa->sa_len;
        }
      }

      /* Format addresses for debug output */
      if (format_address(&dest_addr, dest_str, sizeof(dest_str)) < 0) {
        strlcpy(dest_str, "unknown", sizeof(dest_str));
      }

      if (gw_addr.ss_family != AF_UNSPEC) {
        if (format_address(&gw_addr, gw_str, sizeof(gw_str)) < 0) {
          strlcpy(gw_str, "unknown", sizeof(gw_str));
        }
      } else {
        strlcpy(gw_str, "direct", sizeof(gw_str));
      }

      debug_log(DEBUG, "Route: %s via %s on %s", dest_str, gw_str,
                ifname);
    }
  }

  debug_log(INFO, "Listed routes for FIB %u", fib);
  free(buf);
  return 0;
}

/**
 * Enumerate system routes and populate route list
 * @param state Netd state
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
int freebsd_route_enumerate_system(netd_state_t *state, uint32_t fib) {
  size_t needed;
  int mib[7];
  char *buf, *lim, *next;
  struct rt_msghdr *rtm;
  int retry_count = 0;
  const int max_retries = 3;

  if (!state) {
    return -1;
  }

  /* Use sysctl to get all routing information for a specific FIB */
  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;           /* protocol */
  mib[3] = 0;           /* address family (0 = all) */
  mib[4] = NET_RT_DUMP; /* get all routes */
  mib[5] = 0;           /* flags */
  mib[6] = fib;         /* FIB number */

retry:
  /* First, get the size needed */
  if (sysctl(mib, nitems(mib), NULL, &needed, NULL, 0) < 0) {
    debug_log(ERROR, "Failed to get route table size: %s",
              strerror(errno));
    return -1;
  }

  /* Allocate buffer */
  buf = malloc(needed);
  if (!buf) {
    debug_log(ERROR, "Failed to allocate route table buffer");
    return -1;
  }

  /* Get the actual route data */
  if (sysctl(mib, nitems(mib), buf, &needed, NULL, 0) < 0) {
    if (errno == ENOMEM && retry_count++ < max_retries) {
      debug_log(DEBUG, "Route table grew, retrying (attempt %d)",
                retry_count);
      free(buf);
      sleep(1);
      goto retry;
    }
    debug_log(ERROR, "Failed to get route table: %s", strerror(errno));
    free(buf);
    return -1;
  }

  /* Process all route messages */
  lim = buf + needed;
  for (next = buf; next < lim; next += rtm->rtm_msglen) {
    rtm = (struct rt_msghdr *)(void *)next;

    if (rtm->rtm_type == RTM_ADD || rtm->rtm_type == RTM_CHANGE ||
        rtm->rtm_type == RTM_GET) {
      /* Parse and add route to state */
      parse_route_message(state, rtm, fib);
    }
  }

  debug_log(DEBUG1, "Enumerated routes for FIB %u", fib);
  free(buf);
  return 0;
} 