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
#include <system/freebsd/lagg/lagg.h>

/**
 * Find lagg by name
 * @param state Server state
 * @param name LAGG name
 * @return lagg_t pointer or NULL if not found
 */
static lagg_t *lagg_find(netd_state_t *state, const char *name) {
  lagg_t *lagg;
  
  if (!state || !name) {
    return NULL;
  }
  
  TAILQ_FOREACH(lagg, &state->laggs, entries) {
    if (strcmp(lagg->name, name) == 0) {
      return lagg;
    }
  }
  
  return NULL;
}

/**
 * Create lagg struct
 * @param state Server state
 * @param name LAGG name
 * @return lagg_t pointer or NULL on failure
 */
static lagg_t *lagg_create(netd_state_t *state, const char *name) {
  lagg_t *lagg;
  
  if (!state || !name) {
    return NULL;
  }
  
  lagg = calloc(1, sizeof(lagg_t));
  if (!lagg) {
    debug_log(ERROR, "Failed to allocate memory for lagg %s", name);
    return NULL;
  }
  
  strlcpy(lagg->name, name, MAX_IFNAME_LEN);
  lagg->lagg_proto[0] = '\0';
  lagg->member_count = 0;
  
  TAILQ_INSERT_TAIL(&state->laggs, lagg, entries);
  
  debug_log(DEBUG, "Created lagg struct for %s", name);
  return lagg;
}

/**
 * Create a LAGG interface
 * @param state Server state
 * @param name LAGG interface name
 * @param lagg_proto LAGG protocol (default: "failover")
 * @return 0 on success, -1 on failure
 */
int lagg_interface_create(netd_state_t *state, const char *name,
                          const char *lagg_proto) {
  lagg_t *lagg;

  if (!state || !name) {
    debug_log(ERROR,
              "Invalid parameters for LAGG creation: state=%p, name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Creating LAGG interface '%s'", name);

  /* Create the LAGG interface using the general interface creation */
  int result = interface_create(state, name, IF_TYPE_LAGG);
  if (result < 0) {
    debug_log(ERROR, "Failed to create LAGG interface %s", name);
    return -1;
  }

  /* Create lagg-specific data structure */
  lagg = lagg_create(state, name);
  if (!lagg) {
    debug_log(ERROR, "Failed to create lagg struct for %s", name);
    interface_delete(state, name);
    return -1;
  }

  /* Set LAGG protocol */
  if (lagg_proto) {
    strlcpy(lagg->lagg_proto, lagg_proto, sizeof(lagg->lagg_proto));
  } else {
    strlcpy(lagg->lagg_proto, "failover", sizeof(lagg->lagg_proto));
  }

  /* Create LAGG in FreeBSD */
  if (freebsd_lagg_create(name, lagg->lagg_proto) < 0) {
    debug_log(ERROR,
              "Failed to create LAGG interface %s in FreeBSD", name);
    /* Clean up the interface and lagg struct from state */
    interface_delete(state, name);
    TAILQ_REMOVE(&state->laggs, lagg, entries);
    free(lagg);
    return -1;
  }

  debug_log(INFO, "Created LAGG interface %s with protocol %s", name,
            lagg->lagg_proto);
  return 0;
}

/**
 * Add member interface to LAGG
 * @param state Server state
 * @param lagg_name LAGG interface name
 * @param member_name Member interface name
 * @return 0 on success, -1 on failure
 */
int lagg_add_member(netd_state_t *state, const char *lagg_name,
                     const char *member_name) {
  interface_t *lagg_iface;
  lagg_t *lagg;

  if (!state || !lagg_name || !member_name) {
    debug_log(ERROR,
              "Invalid parameters for LAGG member addition: state=%p, "
              "lagg=%s, member=%s",
              state, lagg_name ? lagg_name : "NULL",
              member_name ? member_name : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Adding member %s to LAGG %s", member_name,
            lagg_name);

  /* Find LAGG interface */
  lagg_iface = interface_find(state, lagg_name);
  if (!lagg_iface) {
    debug_log(ERROR, "LAGG interface %s not found", lagg_name);
    return -1;
  }

  if (lagg_iface->type != IF_TYPE_LAGG) {
    debug_log(ERROR, "Interface %s is not a LAGG", lagg_name);
    return -1;
  }

  /* Find lagg struct */
  lagg = lagg_find(state, lagg_name);
  if (!lagg) {
    debug_log(ERROR, "Lagg struct for %s not found", lagg_name);
    return -1;
  }

  /* Add member to LAGG in FreeBSD */
  if (freebsd_lagg_add_member(lagg_name, member_name) < 0) {
    debug_log(ERROR,
              "Failed to add member %s to LAGG %s in FreeBSD", member_name,
              lagg_name);
    return -1;
  }

  /* Update LAGG members in lagg struct */
  if (lagg->member_count < MAX_LAGG_MEMBERS) {
    strlcpy(lagg->members[lagg->member_count], member_name, MAX_IFNAME_LEN);
    lagg->member_count++;
  } else {
    debug_log(ERROR, "LAGG %s has reached maximum member limit", lagg_name);
    return -1;
  }

  debug_log(INFO, "Added member %s to LAGG %s", member_name, lagg_name);
  return 0;
}

/**
 * Remove member interface from LAGG
 * @param state Server state
 * @param lagg_name LAGG interface name
 * @param member_name Member interface name
 * @return 0 on success, -1 on failure
 */
int lagg_remove_member(netd_state_t *state, const char *lagg_name,
                        const char *member_name) {
  interface_t *lagg_iface;
  lagg_t *lagg;
  int i, j;

  if (!state || !lagg_name || !member_name) {
    debug_log(ERROR,
              "Invalid parameters for LAGG member removal: state=%p, "
              "lagg=%s, member=%s",
              state, lagg_name ? lagg_name : "NULL",
              member_name ? member_name : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Removing member %s from LAGG %s", member_name,
            lagg_name);

  /* Find LAGG interface */
  lagg_iface = interface_find(state, lagg_name);
  if (!lagg_iface) {
    debug_log(ERROR, "LAGG interface %s not found", lagg_name);
    return -1;
  }

  if (lagg_iface->type != IF_TYPE_LAGG) {
    debug_log(ERROR, "Interface %s is not a LAGG", lagg_name);
    return -1;
  }

  /* Find lagg struct */
  lagg = lagg_find(state, lagg_name);
  if (!lagg) {
    debug_log(ERROR, "Lagg struct for %s not found", lagg_name);
    return -1;
  }

  /* Remove member from LAGG in FreeBSD */
  if (freebsd_lagg_remove_member(lagg_name, member_name) < 0) {
    debug_log(ERROR,
              "Failed to remove member %s from LAGG %s in FreeBSD",
              member_name, lagg_name);
    return -1;
  }

  /* Update LAGG members in lagg struct */
  for (i = 0; i < lagg->member_count; i++) {
    if (strcmp(lagg->members[i], member_name) == 0) {
      /* Remove member by shifting remaining members */
      for (j = i; j < lagg->member_count - 1; j++) {
        strlcpy(lagg->members[j], lagg->members[j + 1], MAX_IFNAME_LEN);
      }
      lagg->member_count--;
      break;
    }
  }

  debug_log(INFO, "Removed member %s from LAGG %s", member_name,
            lagg_name);
  return 0;
}

/**
 * Set LAGG protocol
 * @param state Server state
 * @param lagg_name LAGG interface name
 * @param protocol LAGG protocol string
 * @return 0 on success, -1 on failure
 */
int lagg_set_protocol(netd_state_t *state, const char *lagg_name,
                       const char *protocol) {
  interface_t *lagg_iface;
  lagg_t *lagg;

  if (!state || !lagg_name || !protocol) {
    debug_log(ERROR,
              "Invalid parameters for LAGG protocol setting: state=%p, "
              "lagg=%s, protocol=%s",
              state, lagg_name ? lagg_name : "NULL",
              protocol ? protocol : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Setting protocol '%s' for LAGG interface '%s'",
            protocol, lagg_name);

  /* Find LAGG interface */
  lagg_iface = interface_find(state, lagg_name);
  if (!lagg_iface) {
    debug_log(ERROR, "LAGG interface %s not found", lagg_name);
    return -1;
  }

  if (lagg_iface->type != IF_TYPE_LAGG) {
    debug_log(ERROR, "Interface %s is not a LAGG", lagg_name);
    return -1;
  }

  /* Find lagg struct */
  lagg = lagg_find(state, lagg_name);
  if (!lagg) {
    debug_log(ERROR, "Lagg struct for %s not found", lagg_name);
    return -1;
  }

  /* Set protocol in FreeBSD */
  if (freebsd_lagg_set_protocol(lagg_name, protocol) < 0) {
    debug_log(ERROR,
              "Failed to set protocol %s for LAGG %s in FreeBSD", protocol,
              lagg_name);
    return -1;
  }

  /* Update lagg struct */
  strlcpy(lagg->lagg_proto, protocol, sizeof(lagg->lagg_proto));

  debug_log(INFO, "Set protocol %s for LAGG interface %s", protocol,
            lagg_name);
  return 0;
} 