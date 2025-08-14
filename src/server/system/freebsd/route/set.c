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
#include <system/freebsd/freebsd.h>
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
  uint8_t *dest_num = parse_address_freebsd(destination, &dest_addr);
  if (!dest_num) {
    debug_log(ERROR, "Failed to parse destination address %s",
              destination);
    return -1;
  }

  uint8_t *gw_num = NULL;
  if (gateway) {
    gw_num = parse_address_freebsd(gateway, &gw_addr);
    if (!gw_num) {
      debug_log(ERROR, "Failed to parse gateway address %s", gateway);
      free(dest_num);
      return -1;
    }
  }

  /* Create PF_ROUTE socket */
  sock = socket(PF_ROUTE, SOCK_RAW, 0);
  if (sock < 0) {
    debug_log(ERROR, "Failed to create PF_ROUTE socket: %s",
              strerror(errno));
    free(dest_num);
    if (gw_num) {
      free(gw_num);
    }
    return -1;
  }

  /* Set FIB for the socket - only for non-default FIB */
  if (fib > 0) {
    if (setsockopt(sock, SOL_SOCKET, SO_SETFIB, &fib, sizeof(fib)) < 0) {
      debug_log(ERROR, "Failed to set FIB %u for socket: %s", fib,
                strerror(errno));
      close(sock);
      return -1;
    }
    debug_log(DEBUG, "Set FIB %u for route socket", fib);
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
    debug_log(ERROR, "Failed to allocate route message buffer");
    free(dest_num);
    if (gw_num) {
      free(gw_num);
    }
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
  debug_log(DEBUG,
            "Sending route message: type=%d, flags=0x%x, addrs=0x%x, len=%d",
            rtm->rtm_type, rtm->rtm_flags, rtm->rtm_addrs, len);
  if (write(sock, rtm, len) < 0) {
    debug_log(ERROR, "Failed to add route: %s (errno=%d)",
              strerror(errno), errno);
    free(rtm);
    free(dest_num);
    if (gw_num) {
      free(gw_num);
    }
    close(sock);
    return -1;
  }

  debug_log(INFO, "Added route to %s via %s (FIB %u)", destination,
            gateway ? gateway : "direct", fib);
  free(rtm);
  free(dest_num);
  if (gw_num) {
    free(gw_num);
  }
  close(sock);
  return 0;
} 