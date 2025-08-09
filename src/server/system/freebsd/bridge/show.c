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
#include "bridge.h"
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
 * Get bridge member information for a bridge interface
 * @param ifname Interface name
 * @param members Buffer to store member information
 * @param members_size Size of the members buffer
 * @return 0 on success, -1 on failure
 */
int freebsd_get_bridge_members(const char *ifname, char *members,
                               size_t members_size) {
  int sock;
  struct ifdrv ifd;
  struct ifbifconf bifc;
  struct ifbreq *breq;
  char *buf = NULL;
  char members_list[256] = "";
  int member_count = 0;
  int buflen;

  if (!ifname || !members) {
    return -1;
  }

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for bridge members: %s", strerror(errno));
    strlcpy(members, "none", members_size);
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
    debug_log(DEBUG_ERROR, "Failed to get bridge members size for %s: %s", ifname, strerror(errno));
    close(sock);
    strlcpy(members, "none", members_size);
    return -1;
  }

  if (bifc.ifbic_len == 0) {
    debug_log(DEBUG_INFO, "No bridge members found for %s", ifname);
    close(sock);
    strlcpy(members, "none", members_size);
    return 0;
  }

  /* Allocate buffer for bridge member data */
  buflen = bifc.ifbic_len;
  buf = malloc(buflen);
  if (!buf) {
    debug_log(DEBUG_ERROR, "Failed to allocate memory for bridge members");
    close(sock);
    strlcpy(members, "none", members_size);
    return -1;
  }

  /* Second call to get the actual bridge member data */
  bifc.ifbic_req = (struct ifbreq *)buf;
  if (ioctl(sock, SIOCGDRVSPEC, &ifd) < 0) {
    debug_log(DEBUG_ERROR, "Failed to get bridge members for %s: %s", ifname, strerror(errno));
    free(buf);
    close(sock);
    strlcpy(members, "none", members_size);
    return -1;
  }

  /* Parse bridge members from the ifbreq structures */
  breq = (struct ifbreq *)buf;
  int count = bifc.ifbic_len / sizeof(struct ifbreq);
  
  for (int i = 0; i < count && member_count < 10; i++) {
    /* Skip the bridge interface itself and empty entries */
    if (strlen(breq[i].ifbr_ifsname) > 0 && 
        strcmp(breq[i].ifbr_ifsname, ifname) != 0) {
      
      if (member_count > 0) {
        strlcat(members_list, ",", sizeof(members_list));
      }
      strlcat(members_list, breq[i].ifbr_ifsname, sizeof(members_list));
      member_count++;
      debug_log(DEBUG_TRACE, "Found bridge member: %s", breq[i].ifbr_ifsname);
    }
  }

  free(buf);
  close(sock);

  if (strlen(members_list) > 0) {
    strlcpy(members, members_list, members_size);
    debug_log(DEBUG_TRACE, "Found bridge members for %s: '%s'", ifname, members);
  } else {
    strlcpy(members, "none", members_size);
    debug_log(DEBUG_INFO, "No bridge members found for %s", ifname);
  }

  return 0;
}

/**
 * Get bridge member information for a bridge interface
 * @param ifname Interface name
 * @param members Buffer to store member information
 * @param members_size Size of the members buffer
 * @return 0 on success, -1 on failure
 */
int freebsd_get_bridge_members(const char *ifname, char *members,
                               size_t members_size) {
  int sock;
  struct ifdrv ifd;
  struct ifbifconf bifc;
  struct ifbreq *breq;
  char *buf = NULL;
  char members_list[256] = "";
  int member_count = 0;
  int buflen;

  if (!ifname || !members) {
    return -1;
  }

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for bridge members: %s", strerror(errno));
    strlcpy(members, "none", members_size);
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
    debug_log(DEBUG_ERROR, "Failed to get bridge members size for %s: %s", ifname, strerror(errno));
    close(sock);
    strlcpy(members, "none", members_size);
    return -1;
  }

  if (bifc.ifbic_len == 0) {
    debug_log(DEBUG_INFO, "No bridge members found for %s", ifname);
    close(sock);
    strlcpy(members, "none", members_size);
    return 0;
  }

  /* Allocate buffer for bridge member data */
  buflen = bifc.ifbic_len;
  buf = malloc(buflen);
  if (!buf) {
    debug_log(DEBUG_ERROR, "Failed to allocate memory for bridge members");
    close(sock);
    strlcpy(members, "none", members_size);
    return -1;
  }

  /* Second call to get the actual bridge member data */
  bifc.ifbic_req = (struct ifbreq *)buf;
  if (ioctl(sock, SIOCGDRVSPEC, &ifd) < 0) {
    debug_log(DEBUG_ERROR, "Failed to get bridge members for %s: %s", ifname, strerror(errno));
    free(buf);
    close(sock);
    strlcpy(members, "none", members_size);
    return -1;
  }

  /* Parse bridge members from the ifbreq structures */
  breq = (struct ifbreq *)buf;
  int count = bifc.ifbic_len / sizeof(struct ifbreq);
  
  for (int i = 0; i < count && member_count < 10; i++) {
    /* Skip the bridge interface itself and empty entries */
    if (strlen(breq[i].ifbr_ifsname) > 0 && 
        strcmp(breq[i].ifbr_ifsname, ifname) != 0) {
      
      if (member_count > 0) {
        strlcat(members_list, ",", sizeof(members_list));
      }
      strlcat(members_list, breq[i].ifbr_ifsname, sizeof(members_list));
      member_count++;
      debug_log(DEBUG_TRACE, "Found bridge member: %s", breq[i].ifbr_ifsname);
    }
  }

  free(buf);
  close(sock);

  if (strlen(members_list) > 0) {
    strlcpy(members, members_list, members_size);
    debug_log(DEBUG_TRACE, "Found bridge members for %s: '%s'", ifname, members);
  } else {
    strlcpy(members, "none", members_size);
    debug_log(DEBUG_INFO, "No bridge members found for %s", ifname);
  }

  return 0;
} 