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
#include <interface.h>
#include <system/freebsd/bridge/bridge.h>

/**
 * Find bridge by name
 * @param state Server state
 * @param name Bridge name
 * @return bridge_t pointer or NULL if not found
 */
static bridge_t *bridge_find(netd_state_t *state, const char *name) {
  bridge_t *bridge;
  
  if (!state || !name) {
    return NULL;
  }
  
  TAILQ_FOREACH(bridge, &state->bridges, entries) {
    if (strcmp(bridge->name, name) == 0) {
      return bridge;
    }
  }
  
  return NULL;
}

/**
 * Create bridge struct
 * @param state Server state
 * @param name Bridge name
 * @return bridge_t pointer or NULL on failure
 */
static bridge_t *bridge_create(netd_state_t *state, const char *name) {
  bridge_t *bridge;
  
  if (!state || !name) {
    return NULL;
  }
  
  bridge = calloc(1, sizeof(bridge_t));
  if (!bridge) {
    debug_log(ERROR, "Failed to allocate memory for bridge %s", name);
    return NULL;
  }
  
  strlcpy(bridge->name, name, MAX_IFNAME_LEN);
  bridge->member_count = 0;
  bridge->maxaddr = 1000;
  bridge->timeout = 1200;
  strlcpy(bridge->protocol, "stp", sizeof(bridge->protocol));
  
  TAILQ_INSERT_TAIL(&state->bridges, bridge, entries);
  
  debug_log(DEBUG, "Created bridge struct for %s", name);
  return bridge;
}

/**
 * Create a bridge interface
 * @param state Server state
 * @param name Bridge interface name
 * @return 0 on success, -1 on failure
 */
int bridge_interface_create(netd_state_t *state, const char *name) {
  if (!state || !name) {
    debug_log(ERROR,
              "Invalid parameters for bridge creation: state=%p, name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Creating bridge interface '%s'", name);

  /* Create the bridge interface using the general interface creation */
  int result = interface_create(state, name, IF_TYPE_BRIDGE);
  if (result < 0) {
    debug_log(ERROR, "Failed to create bridge interface %s", name);
    return -1;
  }

  /* Create bridge-specific data structure */
  bridge_t *bridge = bridge_create(state, name);
  if (!bridge) {
    debug_log(ERROR, "Failed to create bridge struct for %s", name);
    return -1;
  }

  /* Additional bridge-specific setup can go here */
  debug_log(INFO, "Created bridge interface %s", name);
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
  bridge_t *bridge;

  if (!state || !bridge_name || !member_name) {
    debug_log(ERROR,
              "Invalid parameters for bridge member addition: state=%p, "
              "bridge=%s, member=%s",
              state, bridge_name ? bridge_name : "NULL",
              member_name ? member_name : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Adding member %s to bridge %s", member_name,
            bridge_name);

  /* Find bridge interface */
  bridge_iface = interface_find(state, bridge_name);
  if (!bridge_iface) {
    debug_log(ERROR, "Bridge interface %s not found", bridge_name);
    return -1;
  }

  if (bridge_iface->type != IF_TYPE_BRIDGE) {
    debug_log(ERROR, "Interface %s is not a bridge", bridge_name);
    return -1;
  }

  /* Find or create bridge struct */
  bridge = bridge_find(state, bridge_name);
  if (!bridge) {
    bridge = bridge_create(state, bridge_name);
    if (!bridge) {
      debug_log(ERROR, "Failed to create bridge struct for %s", bridge_name);
      return -1;
    }
  }

  /* Add member to bridge in FreeBSD */
  if (freebsd_bridge_add_member(bridge_name, member_name) < 0) {
    debug_log(ERROR,
              "Failed to add member %s to bridge %s in FreeBSD", member_name,
              bridge_name);
    return -1;
  }

  /* Update bridge members in bridge struct */
  if (bridge->member_count < MAX_BRIDGE_MEMBERS) {
    strlcpy(bridge->members[bridge->member_count], 
            member_name, MAX_IFNAME_LEN);
    bridge->member_count++;
  }

  debug_log(INFO, "Added member %s to bridge %s", member_name,
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
  bridge_t *bridge;

  if (!state || !bridge_name || !member_name) {
    debug_log(ERROR,
              "Invalid parameters for bridge member removal: state=%p, "
              "bridge=%s, member=%s",
              state, bridge_name ? bridge_name : "NULL",
              member_name ? member_name : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Removing member %s from bridge %s", member_name,
            bridge_name);

  /* Find bridge interface */
  bridge_iface = interface_find(state, bridge_name);
  if (!bridge_iface) {
    debug_log(ERROR, "Bridge interface %s not found", bridge_name);
    return -1;
  }

  if (bridge_iface->type != IF_TYPE_BRIDGE) {
    debug_log(ERROR, "Interface %s is not a bridge", bridge_name);
    return -1;
  }

  /* Find bridge struct */
  bridge = bridge_find(state, bridge_name);
  if (!bridge) {
    debug_log(ERROR, "Bridge struct for %s not found", bridge_name);
    return -1;
  }

  /* Remove member from bridge in FreeBSD */
  if (freebsd_bridge_remove_member(bridge_name, member_name) < 0) {
    debug_log(ERROR,
              "Failed to remove member %s from bridge %s in FreeBSD",
              member_name, bridge_name);
    return -1;
  }

  /* Update bridge members in bridge struct */
  for (int i = 0; i < bridge->member_count; i++) {
    if (strcmp(bridge->members[i], member_name) == 0) {
      /* Remove member by shifting remaining members */
      for (int j = i; j < bridge->member_count - 1; j++) {
        strlcpy(bridge->members[j], 
                bridge->members[j + 1], MAX_IFNAME_LEN);
      }
      bridge->member_count--;
      break;
    }
  }

  debug_log(INFO, "Removed member %s from bridge %s", member_name,
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
  bridge_t *bridge;

  if (!state || !bridge_name) {
    debug_log(ERROR,
              "Invalid parameters for bridge STP setting: state=%p, bridge=%s",
              state, bridge_name ? bridge_name : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Setting STP mode %d for bridge %s", stp_mode,
            bridge_name);

  /* Find bridge interface */
  bridge_iface = interface_find(state, bridge_name);
  if (!bridge_iface) {
    debug_log(ERROR, "Bridge interface %s not found", bridge_name);
    return -1;
  }

  if (bridge_iface->type != IF_TYPE_BRIDGE) {
    debug_log(ERROR, "Interface %s is not a bridge", bridge_name);
    return -1;
  }

  /* Find bridge struct */
  bridge = bridge_find(state, bridge_name);
  if (!bridge) {
    debug_log(ERROR, "Bridge struct for %s not found", bridge_name);
    return -1;
  }

  /* Set STP mode in FreeBSD */
  if (freebsd_bridge_set_stp(bridge_name, stp_mode) < 0) {
    debug_log(ERROR,
              "Failed to set STP mode %d for bridge %s in FreeBSD", stp_mode,
              bridge_name);
    return -1;
  }

  debug_log(INFO, "Set STP mode %d for bridge %s", stp_mode,
            bridge_name);
  return 0;
} 