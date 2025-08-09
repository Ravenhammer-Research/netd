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
 * Add a route to the routing table
 * @param fib FIB number
 * @param destination Destination address
 * @param gateway Gateway address
 * @param interface Interface name
 * @param flags Route flags
 * @return 0 on success, -1 on failure
 */
int freebsd_route_add(uint32_t fib, const char *destination,
                      const char *gateway, const char *interface, int flags) {
  int sock;
  struct rt_msghdr *rtm;
  char *cp;
  int len;
  struct sockaddr_storage dest_addr, gw_addr;

  if (!destination) {
    return -1;
  }

  /* Parse addresses */
  if (parse_address(destination, &dest_addr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to parse destination address %s",
              destination);
    return -1;
  }

  if (gateway && parse_address(gateway, &gw_addr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to parse gateway address %s", gateway);
    return -1;
  }

  /* Create PF_ROUTE socket */
  sock = socket(PF_ROUTE, SOCK_RAW, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create PF_ROUTE socket: %s",
              strerror(errno));
    return -1;
  }

  /* Set FIB for the socket - only for non-default FIB */
  if (fib > 0) {
    if (setsockopt(sock, SOL_SOCKET, SO_SETFIB, &fib, sizeof(fib)) < 0) {
      debug_log(DEBUG_ERROR, "Failed to set FIB %u for socket: %s", fib,
                strerror(errno));
      close(sock);
      return -1;
    }
    debug_log(DEBUG_DEBUG, "Set FIB %u for route socket", fib);
  }

  /* Calculate message length */
  len = sizeof(struct rt_msghdr) + SA_SIZE((struct sockaddr *)&dest_addr);
  if (gateway) {
    len += SA_SIZE((struct sockaddr *)&gw_addr);
  }
  if (interface) {
    len += sizeof(struct sockaddr_dl);
  }

  /* Allocate message buffer */
  rtm = malloc(len);
  if (!rtm) {
    debug_log(DEBUG_ERROR, "Failed to allocate route message buffer");
    close(sock);
    return -1;
  }

  /* Set up route message */
  memset(rtm, 0, len);
  rtm->rtm_type = RTM_ADD;
  rtm->rtm_flags = flags | RTF_UP | RTF_STATIC;
  if (gateway) {
    rtm->rtm_flags |= RTF_GATEWAY;
  }
  rtm->rtm_version = RTM_VERSION;
  rtm->rtm_seq = 1;
  rtm->rtm_addrs = RTA_DST;
  rtm->rtm_fmask = 0;
  rtm->rtm_pid = getpid();
  rtm->rtm_msglen = len;

  /* Set destination address */
  cp = (char *)(rtm + 1);
  memmove(cp, (char *)&dest_addr, SA_SIZE((struct sockaddr *)&dest_addr));
  cp += SA_SIZE((struct sockaddr *)&dest_addr);

  /* Set gateway address if provided */
  if (gateway) {
    rtm->rtm_addrs |= RTA_GATEWAY;
    memmove(cp, (char *)&gw_addr, SA_SIZE((struct sockaddr *)&gw_addr));
    cp += SA_SIZE((struct sockaddr *)&gw_addr);
  }

  /* Set interface if provided */
  if (interface) {
    struct sockaddr_dl *sdl = (struct sockaddr_dl *)cp;
    rtm->rtm_addrs |= RTA_IFP;
    sdl->sdl_len = sizeof(struct sockaddr_dl);
    sdl->sdl_family = AF_LINK;
    sdl->sdl_index = if_nametoindex(interface);
    strlcpy(sdl->sdl_data, interface, sizeof(sdl->sdl_data));
  }

  /* Send route message */
  debug_log(DEBUG_DEBUG,
            "Sending route message: type=%d, flags=0x%x, addrs=0x%x, len=%d",
            rtm->rtm_type, rtm->rtm_flags, rtm->rtm_addrs, len);
  if (write(sock, rtm, len) < 0) {
    debug_log(DEBUG_ERROR, "Failed to add route: %s (errno=%d)",
              strerror(errno), errno);
    free(rtm);
    close(sock);
    return -1;
  }

  debug_log(DEBUG_INFO, "Added route to %s via %s (FIB %u)", destination,
            gateway ? gateway : "direct", fib);
  free(rtm);
  close(sock);
  return 0;
}

/**
 * Delete a route from the routing table
 * @param fib FIB number
 * @param destination Destination address
 * @return 0 on success, -1 on failure
 */
int freebsd_route_delete(uint32_t fib, const char *destination) {
  int sock;
  struct rt_msghdr *rtm;
  char *cp;
  int len;
  struct sockaddr_storage dest_addr;

  if (!destination) {
    return -1;
  }

  /* Parse destination address */
  if (parse_address(destination, &dest_addr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to parse destination address %s",
              destination);
    return -1;
  }

  /* Create PF_ROUTE socket */
  sock = socket(PF_ROUTE, SOCK_RAW, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create PF_ROUTE socket: %s",
              strerror(errno));
    return -1;
  }

  /* Calculate message length */
  len = sizeof(struct rt_msghdr) + sizeof(struct sockaddr_storage);

  /* Allocate message buffer */
  rtm = malloc(len);
  if (!rtm) {
    debug_log(DEBUG_ERROR, "Failed to allocate route message buffer");
    close(sock);
    return -1;
  }

  /* Set up route message */
  memset(rtm, 0, len);
  rtm->rtm_type = RTM_DELETE;
  rtm->rtm_flags = 0;
  rtm->rtm_version = RTM_VERSION;
  rtm->rtm_seq = 1;
  rtm->rtm_addrs = RTA_DST;
  rtm->rtm_fmask = 0;
  rtm->rtm_pid = getpid();
  rtm->rtm_msglen = len;

  /* Set destination address */
  cp = (char *)(rtm + 1);
  memcpy(cp, &dest_addr, sizeof(struct sockaddr_storage));

  /* Send route message */
  if (write(sock, rtm, len) < 0) {
    debug_log(DEBUG_ERROR, "Failed to delete route: %s", strerror(errno));
    free(rtm);
    close(sock);
    return -1;
  }

  debug_log(DEBUG_INFO, "Deleted route to %s (FIB %u)", destination, fib);
  free(rtm);
  close(sock);
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
  int mib[6];
  char *buf, *lim, *next;
  struct rt_msghdr *rtm;
  int retry_count = 0;
  const int max_retries = 3;

  /* Use sysctl to get all routing information */
  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;           /* protocol */
  mib[3] = family;      /* address family */
  mib[4] = NET_RT_DUMP; /* get all routes */
  mib[5] = fib;         /* FIB number */

retry:
  /* First, get the size needed */
  if (sysctl(mib, nitems(mib), NULL, &needed, NULL, 0) < 0) {
    debug_log(DEBUG_ERROR, "Failed to get route table size: %s",
              strerror(errno));
    return -1;
  }

  /* Allocate buffer */
  buf = malloc(needed);
  if (!buf) {
    debug_log(DEBUG_ERROR, "Failed to allocate route table buffer");
    return -1;
  }

  /* Get the actual route data */
  if (sysctl(mib, nitems(mib), buf, &needed, NULL, 0) < 0) {
    if (errno == ENOMEM && retry_count++ < max_retries) {
      debug_log(DEBUG_DEBUG, "Route table grew, retrying (attempt %d)",
                retry_count);
      free(buf);
      sleep(1);
      goto retry;
    }
    debug_log(DEBUG_ERROR, "Failed to get route table: %s", strerror(errno));
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
      debug_log(DEBUG_DEBUG, "Received route entry (type: %d)", rtm->rtm_type);

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

      debug_log(DEBUG_DEBUG, "Route: %s via %s on %s", dest_str, gw_str,
                ifname);
    }
  }

  debug_log(DEBUG_INFO, "Listed routes for FIB %u", fib);
  free(buf);
  return 0;
}

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
  struct sockaddr_storage dest_addr, gw_addr;
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
      case RTAX_IFP:
        if (sp[i]->sa_family == AF_LINK) {
          struct sockaddr_dl *sdl = (struct sockaddr_dl *)sp[i];
          if (sdl->sdl_nlen > 0) {
            strlcpy(ifname, sdl->sdl_data, sizeof(ifname));
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
    debug_log(DEBUG_ERROR, "Failed to allocate memory for route");
    return -1;
  }

  /* Initialize route */
  memset(route, 0, sizeof(*route));
  
  /* Convert addresses to sockaddr_storage format */
  if (format_address(&dest_addr, dest_str, sizeof(dest_str)) >= 0) {
    if (parse_address(dest_str, &route->destination) < 0) {
      debug_log(DEBUG_ERROR, "Failed to parse destination address for route");
      free(route);
      return -1;
    }
  }
  
  if (gw_addr.ss_family != AF_UNSPEC) {
    if (format_address(&gw_addr, gw_str, sizeof(gw_str)) >= 0) {
      if (parse_address(gw_str, &route->gateway) < 0) {
        debug_log(DEBUG_ERROR, "Failed to parse gateway address for route");
        free(route);
        return -1;
      }
    }
  }
  
  strlcpy(route->interface, ifname, sizeof(route->interface));
  route->fib = fib;
  route->flags = rtm->rtm_flags;

  /* Add to route list */
  TAILQ_INSERT_TAIL(&state->routes, route, entries);

  debug_log(DEBUG_DEBUG, "Added route: %s via %s on %s (FIB %u)", dest_str,
            gw_str, ifname, fib);

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
  int mib[6];
  char *buf, *lim, *next;
  struct rt_msghdr *rtm;
  int retry_count = 0;
  const int max_retries = 3;

  if (!state) {
    return -1;
  }

  /* Use sysctl to get all routing information */
  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;           /* protocol */
  mib[3] = 0;           /* address family (0 = all) */
  mib[4] = NET_RT_DUMP; /* get all routes */
  mib[5] = fib;         /* FIB number */

retry:
  /* First, get the size needed */
  if (sysctl(mib, nitems(mib), NULL, &needed, NULL, 0) < 0) {
    debug_log(DEBUG_ERROR, "Failed to get route table size: %s",
              strerror(errno));
    return -1;
  }

  /* Allocate buffer */
  buf = malloc(needed);
  if (!buf) {
    debug_log(DEBUG_ERROR, "Failed to allocate route table buffer");
    return -1;
  }

  /* Get the actual route data */
  if (sysctl(mib, nitems(mib), buf, &needed, NULL, 0) < 0) {
    if (errno == ENOMEM && retry_count++ < max_retries) {
      debug_log(DEBUG_DEBUG, "Route table grew, retrying (attempt %d)",
                retry_count);
      free(buf);
      sleep(1);
      goto retry;
    }
    debug_log(DEBUG_ERROR, "Failed to get route table: %s", strerror(errno));
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

  debug_log(DEBUG_INFO, "Enumerated routes for FIB %u", fib);
  free(buf);
  return 0;
} 