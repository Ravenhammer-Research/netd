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

#include "vlan.h"
#include <sys/types.h>
#include <net/if.h>
#include <net/if_vlan_var.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/sockio.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Create a VLAN interface
 * @param name VLAN interface name
 * @param parent_name Parent interface name
 * @param vlan_id VLAN ID
 * @return 0 on success, -1 on failure
 */
int freebsd_vlan_create(const char *name, const char *parent_name, int vlan_id) {
  int sock;
  struct ifreq ifr;
  struct vlanreq vreq;

  if (!name || !parent_name || vlan_id < 0 || vlan_id > 4095) {
    debug_log(DEBUG_ERROR, "Invalid parameters for VLAN creation");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Creating VLAN interface %s on parent %s with ID %d", 
            name, parent_name, vlan_id);

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for VLAN creation: %s", strerror(errno));
    return -1;
  }

  /* Set up VLAN request */
  memset(&vreq, 0, sizeof(vreq));
  strlcpy(vreq.vlr_parent, parent_name, sizeof(vreq.vlr_parent));
  vreq.vlr_tag = vlan_id;
  vreq.vlr_proto = ETHERTYPE_VLAN;

  /* Create VLAN interface */
  if (ioctl(sock, SIOCIFCREATE2, &vreq) < 0) {
    debug_log(DEBUG_ERROR, "Failed to create VLAN interface: %s", strerror(errno));
    close(sock);
    return -1;
  }

  close(sock);
  debug_log(DEBUG_INFO, "Created VLAN interface %s", name);
  return 0;
}

/**
 * Set VLAN priority (PCP)
 * @param name VLAN interface name
 * @param priority Priority value (0-7)
 * @return 0 on success, -1 on failure
 */
int freebsd_vlan_set_priority(const char *name, uint8_t priority) {
  int sock;
  struct ifreq ifr;
  struct vlanreq vreq;

  if (!name || priority > 7) {
    debug_log(DEBUG_ERROR, "Invalid parameters for VLAN priority setting");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Setting VLAN priority for %s to %d", name, priority);

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for VLAN priority: %s", strerror(errno));
    return -1;
  }

  /* Get current VLAN configuration */
  memset(&vreq, 0, sizeof(vreq));
  strlcpy(vreq.vlr_parent, name, sizeof(vreq.vlr_parent));

  if (ioctl(sock, SIOCGETVLAN, &vreq) < 0) {
    debug_log(DEBUG_ERROR, "Failed to get VLAN configuration: %s", strerror(errno));
    close(sock);
    return -1;
  }

  /* Set priority in the VLAN configuration */
  vreq.vlr_prio = priority;

  /* Apply the updated configuration */
  if (ioctl(sock, SIOCSETVLAN, &vreq) < 0) {
    debug_log(DEBUG_ERROR, "Failed to set VLAN priority: %s", strerror(errno));
    close(sock);
    return -1;
  }

  close(sock);
  debug_log(DEBUG_INFO, "Set VLAN priority for %s to %d", name, priority);
  return 0;
}

/**
 * Set VLAN protocol
 * @param name VLAN interface name
 * @param protocol Protocol string (e.g., "802.1q", "802.1ad")
 * @return 0 on success, -1 on failure
 */
int freebsd_vlan_set_protocol(const char *name, const char *protocol) {
  int sock;
  struct ifreq ifr;
  struct vlanreq vreq;
  uint16_t proto_value;

  if (!name || !protocol) {
    debug_log(DEBUG_ERROR, "Invalid parameters for VLAN protocol setting");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Setting VLAN protocol for %s to %s", name, protocol);

  /* Parse protocol string */
  if (strcmp(protocol, "802.1q") == 0) {
    proto_value = ETHERTYPE_VLAN;
  } else if (strcmp(protocol, "802.1ad") == 0) {
    proto_value = ETHERTYPE_VLAN; /* FreeBSD uses same ethertype for QinQ */
  } else {
    debug_log(DEBUG_ERROR, "Unsupported VLAN protocol: %s", protocol);
    return -1;
  }

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for VLAN protocol: %s", strerror(errno));
    return -1;
  }

  /* Get current VLAN configuration */
  memset(&vreq, 0, sizeof(vreq));
  strlcpy(vreq.vlr_parent, name, sizeof(vreq.vlr_parent));

  if (ioctl(sock, SIOCGETVLAN, &vreq) < 0) {
    debug_log(DEBUG_ERROR, "Failed to get VLAN configuration: %s", strerror(errno));
    close(sock);
    return -1;
  }

  /* Set protocol in the VLAN configuration */
  vreq.vlr_proto = proto_value;

  /* Apply the updated configuration */
  if (ioctl(sock, SIOCSETVLAN, &vreq) < 0) {
    debug_log(DEBUG_ERROR, "Failed to set VLAN protocol: %s", strerror(errno));
    close(sock);
    return -1;
  }

  close(sock);
  debug_log(DEBUG_INFO, "Set VLAN protocol for %s to %s", name, protocol);
  return 0;
} 