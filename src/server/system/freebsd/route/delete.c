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