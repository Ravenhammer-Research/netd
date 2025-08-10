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
#include <system/freebsd/lagg/lagg.h>

/**
 * Create a LAGG interface
 * @param state Server state
 * @param name LAGG interface name
 * @param lagg_proto LAGG protocol (default: "failover")
 * @return 0 on success, -1 on failure
 */
int lagg_interface_create(netd_state_t *state, const char *name,
                          const char *lagg_proto) {
  interface_t *lagg_iface;

  if (!state || !name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for LAGG creation: state=%p, name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Creating LAGG interface '%s'", name);

  /* Create the LAGG interface using the general interface creation */
  int result = interface_create(state, name, IF_TYPE_LAGG);
  if (result < 0) {
    debug_log(DEBUG_ERROR, "Failed to create LAGG interface %s", name);
    return -1;
  }

  /* Find the created interface and set LAGG-specific properties */
  lagg_iface = interface_find(state, name);
  if (!lagg_iface) {
    debug_log(DEBUG_ERROR, "Failed to find created LAGG interface %s", name);
    return -1;
  }

  /* Set LAGG protocol */
  if (lagg_proto) {
    strlcpy(lagg_iface->lagg_proto, lagg_proto, sizeof(lagg_iface->lagg_proto));
  } else {
    strlcpy(lagg_iface->lagg_proto, "failover", sizeof(lagg_iface->lagg_proto));
  }

  /* Create LAGG in FreeBSD */
  if (freebsd_lagg_create(name, lagg_iface->lagg_proto) < 0) {
    debug_log(DEBUG_ERROR,
              "Failed to create LAGG interface %s in FreeBSD", name);
    /* Clean up the interface from state */
    interface_delete(state, name);
    return -1;
  }

  debug_log(DEBUG_INFO, "Created LAGG interface %s with protocol %s", name,
            lagg_iface->lagg_proto);
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

  if (!state || !lagg_name || !member_name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for LAGG member addition: state=%p, "
              "lagg=%s, member=%s",
              state, lagg_name ? lagg_name : "NULL",
              member_name ? member_name : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Adding member %s to LAGG %s", member_name,
            lagg_name);

  /* Find LAGG interface */
  lagg_iface = interface_find(state, lagg_name);
  if (!lagg_iface) {
    debug_log(DEBUG_ERROR, "LAGG interface %s not found", lagg_name);
    return -1;
  }

  if (lagg_iface->type != IF_TYPE_LAGG) {
    debug_log(DEBUG_ERROR, "Interface %s is not a LAGG", lagg_name);
    return -1;
  }

  /* Add member to LAGG in FreeBSD */
  if (freebsd_lagg_add_member(lagg_name, member_name) < 0) {
    debug_log(DEBUG_ERROR,
              "Failed to add member %s to LAGG %s in FreeBSD", member_name,
              lagg_name);
    return -1;
  }

  /* Update LAGG members in state */
  if (strlen(lagg_iface->lagg_members) == 0) {
    strlcpy(lagg_iface->lagg_members, member_name,
            sizeof(lagg_iface->lagg_members));
  } else {
    /* Append to existing members */
    char temp[sizeof(lagg_iface->lagg_members)];
    strlcpy(temp, lagg_iface->lagg_members, sizeof(temp));
    strlcat(temp, ",", sizeof(temp));
    strlcat(temp, member_name, sizeof(temp));
    strlcpy(lagg_iface->lagg_members, temp,
            sizeof(lagg_iface->lagg_members));
  }

  debug_log(DEBUG_INFO, "Added member %s to LAGG %s", member_name, lagg_name);
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

  if (!state || !lagg_name || !member_name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for LAGG member removal: state=%p, "
              "lagg=%s, member=%s",
              state, lagg_name ? lagg_name : "NULL",
              member_name ? member_name : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Removing member %s from LAGG %s", member_name,
            lagg_name);

  /* Find LAGG interface */
  lagg_iface = interface_find(state, lagg_name);
  if (!lagg_iface) {
    debug_log(DEBUG_ERROR, "LAGG interface %s not found", lagg_name);
    return -1;
  }

  if (lagg_iface->type != IF_TYPE_LAGG) {
    debug_log(DEBUG_ERROR, "Interface %s is not a LAGG", lagg_name);
    return -1;
  }

  /* Remove member from LAGG in FreeBSD */
  if (freebsd_lagg_remove_member(lagg_name, member_name) < 0) {
    debug_log(DEBUG_ERROR,
              "Failed to remove member %s from LAGG %s in FreeBSD",
              member_name, lagg_name);
    return -1;
  }

  /* Update LAGG members in state */
  if (strstr(lagg_iface->lagg_members, member_name)) {
    /* Simple removal - in a real implementation you'd want more sophisticated
     * parsing */
    char *pos = strstr(lagg_iface->lagg_members, member_name);
    if (pos) {
      /* Remove the member from the string */
      memmove(pos, pos + strlen(member_name), strlen(pos + strlen(member_name)) + 1);
      /* Clean up any leading/trailing commas */
      if (lagg_iface->lagg_members[0] == ',') {
        memmove(lagg_iface->lagg_members,
                lagg_iface->lagg_members + 1,
                strlen(lagg_iface->lagg_members));
      }
      char *last_comma = strrchr(lagg_iface->lagg_members, ',');
      if (last_comma && *(last_comma + 1) == '\0') {
        *last_comma = '\0';
      }
    }
  }

  debug_log(DEBUG_INFO, "Removed member %s from LAGG %s", member_name,
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

  if (!state || !lagg_name || !protocol) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for LAGG protocol setting: state=%p, "
              "lagg=%s, protocol=%s",
              state, lagg_name ? lagg_name : "NULL",
              protocol ? protocol : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Setting protocol '%s' for LAGG interface '%s'",
            protocol, lagg_name);

  /* Find LAGG interface */
  lagg_iface = interface_find(state, lagg_name);
  if (!lagg_iface) {
    debug_log(DEBUG_ERROR, "LAGG interface %s not found", lagg_name);
    return -1;
  }

  if (lagg_iface->type != IF_TYPE_LAGG) {
    debug_log(DEBUG_ERROR, "Interface %s is not a LAGG", lagg_name);
    return -1;
  }

  /* Set protocol in FreeBSD */
  if (freebsd_lagg_set_protocol(lagg_name, protocol) < 0) {
    debug_log(DEBUG_ERROR,
              "Failed to set protocol %s for LAGG %s in FreeBSD", protocol,
              lagg_name);
    return -1;
  }

  /* Update interface state */
  strlcpy(lagg_iface->lagg_proto, protocol, sizeof(lagg_iface->lagg_proto));

  debug_log(DEBUG_INFO, "Set protocol %s for LAGG interface %s", protocol,
            lagg_name);
  return 0;
} 