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
#include <system/freebsd/bridge/bridge.h>

/**
 * Create a bridge interface
 * @param state Server state
 * @param name Bridge interface name
 * @return 0 on success, -1 on failure
 */
int bridge_interface_create(netd_state_t *state, const char *name) {
  if (!state || !name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for bridge creation: state=%p, name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Creating bridge interface '%s'", name);

  /* Create the bridge interface using the general interface creation */
  int result = interface_create(state, name, IF_TYPE_BRIDGE);
  if (result < 0) {
    debug_log(DEBUG_ERROR, "Failed to create bridge interface %s", name);
    return -1;
  }

  /* Additional bridge-specific setup can go here */
  debug_log(DEBUG_INFO, "Created bridge interface %s", name);
  return 0;
}

/**
 * Add member interface to bridge
 * @param state Server state
 * @param bridge_name Bridge interface name
 * @param member_name Member interface name
 * @return 0 on success, -1 on failure
 */
int bridge_add_member(netd_state_t *state, const char *bridge_name,
                      const char *member_name) {
  interface_t *bridge_iface;

  if (!state || !bridge_name || !member_name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for bridge member addition: state=%p, "
              "bridge=%s, member=%s",
              state, bridge_name ? bridge_name : "NULL",
              member_name ? member_name : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Adding member %s to bridge %s", member_name,
            bridge_name);

  /* Find bridge interface */
  bridge_iface = interface_find(state, bridge_name);
  if (!bridge_iface) {
    debug_log(DEBUG_ERROR, "Bridge interface %s not found", bridge_name);
    return -1;
  }

  if (bridge_iface->type != IF_TYPE_BRIDGE) {
    debug_log(DEBUG_ERROR, "Interface %s is not a bridge", bridge_name);
    return -1;
  }

  /* Add member to bridge in FreeBSD */
  if (freebsd_bridge_add_member(bridge_name, member_name) < 0) {
    debug_log(DEBUG_ERROR,
              "Failed to add member %s to bridge %s in FreeBSD", member_name,
              bridge_name);
    return -1;
  }

  /* Update bridge members in state */
  if (strlen(bridge_iface->bridge_members) == 0) {
    strlcpy(bridge_iface->bridge_members, member_name,
            sizeof(bridge_iface->bridge_members));
  } else {
    /* Append to existing members */
    char temp[sizeof(bridge_iface->bridge_members)];
    strlcpy(temp, bridge_iface->bridge_members, sizeof(temp));
    strlcat(temp, ",", sizeof(temp));
    strlcat(temp, member_name, sizeof(temp));
    strlcpy(bridge_iface->bridge_members, temp,
            sizeof(bridge_iface->bridge_members));
  }

  debug_log(DEBUG_INFO, "Added member %s to bridge %s", member_name,
            bridge_name);
  return 0;
}

/**
 * Remove member interface from bridge
 * @param state Server state
 * @param bridge_name Bridge interface name
 * @param member_name Member interface name
 * @return 0 on success, -1 on failure
 */
int bridge_remove_member(netd_state_t *state, const char *bridge_name,
                         const char *member_name) {
  interface_t *bridge_iface;

  if (!state || !bridge_name || !member_name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for bridge member removal: state=%p, "
              "bridge=%s, member=%s",
              state, bridge_name ? bridge_name : "NULL",
              member_name ? member_name : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Removing member %s from bridge %s", member_name,
            bridge_name);

  /* Find bridge interface */
  bridge_iface = interface_find(state, bridge_name);
  if (!bridge_iface) {
    debug_log(DEBUG_ERROR, "Bridge interface %s not found", bridge_name);
    return -1;
  }

  if (bridge_iface->type != IF_TYPE_BRIDGE) {
    debug_log(DEBUG_ERROR, "Interface %s is not a bridge", bridge_name);
    return -1;
  }

  /* Remove member from bridge in FreeBSD */
  if (freebsd_bridge_remove_member(bridge_name, member_name) < 0) {
    debug_log(DEBUG_ERROR,
              "Failed to remove member %s from bridge %s in FreeBSD",
              member_name, bridge_name);
    return -1;
  }

  /* Update bridge members in state */
  if (strstr(bridge_iface->bridge_members, member_name)) {
    /* Simple removal - in a real implementation you'd want more sophisticated
     * parsing */
    char *pos = strstr(bridge_iface->bridge_members, member_name);
    if (pos) {
      /* Remove the member from the string */
      memmove(pos, pos + strlen(member_name), strlen(pos + strlen(member_name)) + 1);
      /* Clean up any leading/trailing commas */
      if (bridge_iface->bridge_members[0] == ',') {
        memmove(bridge_iface->bridge_members,
                bridge_iface->bridge_members + 1,
                strlen(bridge_iface->bridge_members));
      }
      char *last_comma = strrchr(bridge_iface->bridge_members, ',');
      if (last_comma && *(last_comma + 1) == '\0') {
        *last_comma = '\0';
      }
    }
  }

  debug_log(DEBUG_INFO, "Removed member %s from bridge %s", member_name,
            bridge_name);
  return 0;
}

/**
 * Set bridge STP (Spanning Tree Protocol) mode
 * @param state Server state
 * @param bridge_name Bridge interface name
 * @param stp_mode STP mode (0=disabled, 1=enabled)
 * @return 0 on success, -1 on failure
 */
int bridge_set_stp(netd_state_t *state, const char *bridge_name, int stp_mode) {
  interface_t *bridge_iface;

  if (!state || !bridge_name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for bridge STP setting: state=%p, bridge=%s",
              state, bridge_name ? bridge_name : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Setting STP mode %d for bridge %s", stp_mode,
            bridge_name);

  /* Find bridge interface */
  bridge_iface = interface_find(state, bridge_name);
  if (!bridge_iface) {
    debug_log(DEBUG_ERROR, "Bridge interface %s not found", bridge_name);
    return -1;
  }

  if (bridge_iface->type != IF_TYPE_BRIDGE) {
    debug_log(DEBUG_ERROR, "Interface %s is not a bridge", bridge_name);
    return -1;
  }

  /* Set STP mode in FreeBSD */
  if (freebsd_bridge_set_stp(bridge_name, stp_mode) < 0) {
    debug_log(DEBUG_ERROR,
              "Failed to set STP mode %d for bridge %s in FreeBSD", stp_mode,
              bridge_name);
    return -1;
  }

  debug_log(DEBUG_INFO, "Set STP mode %d for bridge %s", stp_mode,
            bridge_name);
  return 0;
} 