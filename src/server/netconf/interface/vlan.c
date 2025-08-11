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
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system/freebsd/vlan/vlan.h>

/**
 * Create a VLAN interface
 * @param state Server state
 * @param name VLAN interface name
 * @param parent_name Parent interface name
 * @param vlan_id VLAN ID
 * @param vlan_proto VLAN protocol (default: 802.1Q)
 * @return 0 on success, -1 on failure
 */
int vlan_interface_create(netd_state_t *state, const char *name,
                          const char *parent_name, uint16_t vlan_id,
                          const char *vlan_proto) {
  interface_t *vlan_iface;

  if (!state || !name || !parent_name) {
    debug_log(ERROR,
              "Invalid parameters for VLAN creation: state=%p, name=%s, "
              "parent=%s",
              state, name ? name : "NULL", parent_name ? parent_name : "NULL");
    return -1;
  }

  if (vlan_id < 1 || vlan_id > 4094) {
    debug_log(ERROR, "Invalid VLAN ID %d (must be 1-4094)", vlan_id);
    return -1;
  }

  debug_log(DEBUG, "Creating VLAN interface '%s' on parent '%s' with ID %d",
            name, parent_name, vlan_id);

  /* Create the VLAN interface using the general interface creation */
  int result = interface_create(state, name, IF_TYPE_VLAN);
  if (result < 0) {
    debug_log(ERROR, "Failed to create VLAN interface %s", name);
    return -1;
  }

  /* Find the created interface and set VLAN-specific properties */
  vlan_iface = interface_find(state, name);
  if (!vlan_iface) {
    debug_log(ERROR, "Failed to find created VLAN interface %s", name);
    return -1;
  }

  /* Set VLAN properties */
  vlan_iface->vlan_id = vlan_id;
  strlcpy(vlan_iface->vlan_parent, parent_name, sizeof(vlan_iface->vlan_parent));
  
  if (vlan_proto) {
    strlcpy(vlan_iface->vlan_proto, vlan_proto, sizeof(vlan_iface->vlan_proto));
  } else {
    strlcpy(vlan_iface->vlan_proto, "802.1Q", sizeof(vlan_iface->vlan_proto));
  }

  /* Create VLAN in FreeBSD */
  if (freebsd_vlan_create(name, parent_name, vlan_id) < 0) {
    debug_log(ERROR,
              "Failed to create VLAN %s on parent %s in FreeBSD", name,
              parent_name);
    /* Clean up the interface from state */
    interface_delete(state, name);
    return -1;
  }

  debug_log(INFO, "Created VLAN interface %s on parent %s with ID %d",
            name, parent_name, vlan_id);
  return 0;
}

/**
 * Set VLAN priority (PCP - Priority Code Point)
 * @param state Server state
 * @param name VLAN interface name
 * @param priority Priority value (0-7)
 * @return 0 on success, -1 on failure
 */
int vlan_set_priority(netd_state_t *state, const char *name, uint8_t priority) {
  interface_t *vlan_iface;

  if (!state || !name) {
    debug_log(ERROR,
              "Invalid parameters for VLAN priority setting: state=%p, name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  if (priority > 7) {
    debug_log(ERROR, "Invalid VLAN priority %d (must be 0-7)", priority);
    return -1;
  }

  debug_log(DEBUG, "Setting priority %d for VLAN interface '%s'", priority,
            name);

  /* Find VLAN interface */
  vlan_iface = interface_find(state, name);
  if (!vlan_iface) {
    debug_log(ERROR, "VLAN interface %s not found", name);
    return -1;
  }

  if (vlan_iface->type != IF_TYPE_VLAN) {
    debug_log(ERROR, "Interface %s is not a VLAN", name);
    return -1;
  }

  /* Set priority in FreeBSD */
  if (freebsd_vlan_set_priority(name, priority) < 0) {
    debug_log(ERROR,
              "Failed to set priority %d for VLAN %s in FreeBSD", priority,
              name);
    return -1;
  }

  /* Update interface state */
  vlan_iface->vlan_pcp = priority;

  debug_log(INFO, "Set priority %d for VLAN interface %s", priority,
            name);
  return 0;
}

/**
 * Set VLAN protocol
 * @param state Server state
 * @param name VLAN interface name
 * @param protocol VLAN protocol string
 * @return 0 on success, -1 on failure
 */
int vlan_set_protocol(netd_state_t *state, const char *name,
                       const char *protocol) {
  interface_t *vlan_iface;

  if (!state || !name || !protocol) {
    debug_log(ERROR,
              "Invalid parameters for VLAN protocol setting: state=%p, "
              "name=%s, protocol=%s",
              state, name ? name : "NULL", protocol ? protocol : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Setting protocol '%s' for VLAN interface '%s'",
            protocol, name);

  /* Find VLAN interface */
  vlan_iface = interface_find(state, name);
  if (!vlan_iface) {
    debug_log(ERROR, "VLAN interface %s not found", name);
    return -1;
  }

  if (vlan_iface->type != IF_TYPE_VLAN) {
    debug_log(ERROR, "Interface %s is not a VLAN", name);
    return -1;
  }

  /* Set protocol in FreeBSD */
  if (freebsd_vlan_set_protocol(name, protocol) < 0) {
    debug_log(ERROR,
              "Failed to set protocol %s for VLAN %s in FreeBSD", protocol,
              name);
    return -1;
  }

  /* Update interface state */
  strlcpy(vlan_iface->vlan_proto, protocol, sizeof(vlan_iface->vlan_proto));

  debug_log(INFO, "Set protocol %s for VLAN interface %s", protocol,
            name);
  return 0;
}

/**
 * Get VLAN information
 * @param state Server state
 * @param name VLAN interface name
 * @param vlan_id Output parameter for VLAN ID
 * @param parent_name Output parameter for parent interface name
 * @param protocol Output parameter for VLAN protocol
 * @param priority Output parameter for VLAN priority
 * @return 0 on success, -1 on failure
 */
int vlan_get_info(netd_state_t *state, const char *name, uint16_t *vlan_id,
                   char *parent_name, size_t parent_len, char *protocol,
                   size_t protocol_len, uint8_t *priority) {
  interface_t *vlan_iface;

  if (!state || !name) {
    debug_log(ERROR,
              "Invalid parameters for VLAN info retrieval: state=%p, name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  /* Find VLAN interface */
  vlan_iface = interface_find(state, name);
  if (!vlan_iface) {
    debug_log(ERROR, "VLAN interface %s not found", name);
    return -1;
  }

  if (vlan_iface->type != IF_TYPE_VLAN) {
    debug_log(ERROR, "Interface %s is not a VLAN", name);
    return -1;
  }

  /* Return VLAN information */
  if (vlan_id) {
    *vlan_id = vlan_iface->vlan_id;
  }
  if (parent_name && parent_len > 0) {
    strlcpy(parent_name, vlan_iface->vlan_parent, parent_len);
  }
  if (protocol && protocol_len > 0) {
    strlcpy(protocol, vlan_iface->vlan_proto, protocol_len);
  }
  if (priority) {
    *priority = vlan_iface->vlan_pcp;
  }

  debug_log(DEBUG, "Retrieved VLAN info for %s: id=%d, parent=%s, "
            "protocol=%s, priority=%d",
            name, vlan_iface->vlan_id, vlan_iface->vlan_parent,
            vlan_iface->vlan_proto, vlan_iface->vlan_pcp);
  return 0;
} 