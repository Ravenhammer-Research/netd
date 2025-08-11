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

#include <sys/types.h>
#include <net/if.h>
#include <net/if_vlan_var.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/sockio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netd.h>

/**
 * Show VLAN interface information
 * @param name VLAN interface name
 * @param vlan_id VLAN ID (output)
 * @param vlan_proto VLAN protocol (output)
 * @param proto_size Size of vlan_proto buffer
 * @param vlan_pcp VLAN Priority Code Point (output)
 * @param vlan_parent Parent interface name (output)
 * @param parent_size Size of vlan_parent buffer
 * @return 0 on success, -1 on failure
 */
int freebsd_vlan_show(const char *name, int *vlan_id, char *vlan_proto,
                      size_t proto_size, int *vlan_pcp, char *vlan_parent,
                      size_t parent_size) {
  int sock;
  struct ifreq ifr;
  struct vlanreq vreq;
  int found = 0;

  if (!name || !vlan_id || !vlan_proto || !vlan_pcp || !vlan_parent) {
    return -1;
  }

  /* Initialize output parameters */
  *vlan_id = -1;
  *vlan_pcp = 0;
  vlan_proto[0] = '\0';
  vlan_parent[0] = '\0';

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(ERROR, "Failed to create socket for vlan info: %s", strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Check if interface exists */
  if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
    debug_log(ERROR, "Interface %s does not exist", name);
    close(sock);
    return -1;
  }

  /* Get VLAN information using VLAN ioctls */
  memset(&vreq, 0, sizeof(vreq));
  strlcpy(vreq.vlr_parent, name, sizeof(vreq.vlr_parent));

  /* Get VLAN configuration */
  if (ioctl(sock, SIOCGETVLAN, &vreq) == 0) {
    *vlan_id = vreq.vlr_tag;
    *vlan_pcp = 0; /* PCP is not available in vlanreq structure */
    
    /* Set protocol based on vreq.vlr_proto */
    switch (vreq.vlr_proto) {
      case ETHERTYPE_VLAN:
        strlcpy(vlan_proto, "802.1q", proto_size);
        break;
      default:
        snprintf(vlan_proto, proto_size, "0x%04x", vreq.vlr_proto);
        break;
    }
    
    strlcpy(vlan_parent, vreq.vlr_parent, parent_size);
    found = 1;
  } else {
    /* Fallback: try to infer VLAN info from interface name */
    char *dot = strchr(name, '.');
    if (dot) {
      /* This is a VLAN interface like em0.18 */
      strlcpy(vlan_parent, name, dot - name + 1);
      *vlan_id = atoi(dot + 1);
      strlcpy(vlan_proto, "802.1q", proto_size);
      found = 1;
    } else if (strncmp(name, "vlan", 4) == 0) {
      /* This is a VLAN interface like vlan28 */
      *vlan_id = atoi(name + 4);
      strlcpy(vlan_proto, "802.1q", proto_size);
      found = 1;
    }
  }

  close(sock);
  return found ? 0 : -1;
} 