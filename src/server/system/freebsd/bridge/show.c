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
#include <types.h>
#include <bridge.h>
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
#include <bridge.h>




/**
 * Get bridge members and populate netd_bridge_members_t structure
 * @param ifname Bridge interface name
 * @param members netd_bridge_members_t structure to populate
 * @return 0 on success, -1 on failure
 */
int freebsd_get_bridge_members(const char *ifname, netd_bridge_members_t *members) {
  int sock;
  struct ifdrv ifd;
  struct ifbifconf bifc;
  struct ifbreq *breq;
  char *buf = NULL;
  int buflen;

  if (!ifname || !members) {
    return -1;
  }

  /* Initialize the TAILQ structure */
  netd_bridge_members_init(members);

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(ERROR, "Failed to create socket for bridge members: %s", strerror(errno));
    return -1;
  }

  /* Set up ifdrv structure for bridge ioctl */
  memset(&ifd, 0, sizeof(ifd));
  strlcpy(ifd.ifd_name, ifname, sizeof(ifd.ifd_name));
  ifd.ifd_cmd = BRDGGIFS;
  ifd.ifd_len = sizeof(struct ifbifconf);
  ifd.ifd_data = &bifc;

  /* First call to get the required buffer size */
  memset(&bifc, 0, sizeof(bifc));
  bifc.ifbic_len = 0;
  bifc.ifbic_req = NULL;

  if (ioctl(sock, SIOCGDRVSPEC, &ifd) < 0) {
    debug_log(ERROR, "Failed to get bridge members size for %s: %s", ifname, strerror(errno));
    close(sock);
    return -1;
  }

  if (bifc.ifbic_len == 0) {
    debug_log(INFO, "No bridge members found for %s", ifname);
    close(sock);
    return 0;
  }

  /* Allocate buffer for bridge member data */
  buflen = bifc.ifbic_len;
  buf = malloc(buflen);
  if (!buf) {
    debug_log(ERROR, "Failed to allocate memory for bridge members");
    close(sock);
    return -1;
  }

  /* Second call to get the actual bridge member data */
  bifc.ifbic_req = (struct ifbreq *)buf;
  if (ioctl(sock, SIOCGDRVSPEC, &ifd) < 0) {
    debug_log(ERROR, "Failed to get bridge members for %s: %s", ifname, strerror(errno));
    free(buf);
    close(sock);
    return -1;
  }

  /* Parse bridge members from the ifbreq structures */
  breq = (struct ifbreq *)buf;
  int count = bifc.ifbic_len / sizeof(struct ifbreq);
  
  for (int i = 0; i < count; i++) {
    /* Skip the bridge interface itself and empty entries */
    if (strlen(breq[i].ifbr_ifsname) > 0 && 
        strcmp(breq[i].ifbr_ifsname, ifname) != 0) {
      
      /* For now, we'll pass NULL for the interface pointer since we don't have
         the actual interface lookup yet. This should be updated when interface
         lookup is implemented. The netd_bridge_members_add function will create
         the member structure internally. */
      if (!netd_bridge_members_add(members, NULL)) {
        debug_log(ERROR, "Failed to add bridge member to list");
        continue;
      }
      
      debug_log(DEBUG2, "Found bridge member: %s", breq[i].ifbr_ifsname);
    }
  }

  free(buf);
  close(sock);

  debug_log(DEBUG2, "Found %d bridge members for %s", members->count, ifname);
  return 0;
}

/**
 * Get complete bridge data including interface base and members
 * @param ifname Bridge interface name
 * @param bridge netd_bridge_t structure to populate
 * @return 0 on success, -1 on failure
 */
int freebsd_get_bridge_data(const char *ifname, netd_bridge_t *bridge) {
    if (!ifname || !bridge) {
        return -1;
    }
    
    /* Initialize the bridge structure */
    memset(bridge, 0, sizeof(netd_bridge_t));
    
    /* Set the interface name */
    strlcpy(bridge->base.name, ifname, sizeof(bridge->base.name));
    
    /* Set the interface type */
    bridge->base.type = NETD_IF_TYPE_BRIDGE;
    
    /* Initialize the bridge members TAILQ */
    netd_bridge_members_init(&bridge->members);
    
    /* Get bridge members */
    if (freebsd_get_bridge_members(ifname, &bridge->members) != 0) {
        debug_log(ERROR, "Failed to get bridge members for %s", ifname);
        return -1;
    }
    
    /* For now, set default values for bridge-specific fields */
    bridge->timeout = 120;  /* Default timeout */
    bridge->protocol = NETD_BRIDGE_PROTO_STP;  /* Default to STP */
    
    /* TODO: Get actual bridge configuration from FreeBSD system:
     * - MTU from interface
     * - Bridge timeout from sysctl or ioctl
     * - Bridge protocol from sysctl or ioctl
     * - Interface flags from getifaddrs or ioctl
     * - Addresses from getifaddrs
     * - Groups from interface groups
     */
    
    debug_log(DEBUG, "Populated bridge data for %s with %d members", ifname, bridge->members.count);
    return 0;
}

 