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
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * Delete an interface
 * @param state Server state
 * @param name Interface name
 * @return 0 on success, -1 on failure
 */
int interface_delete(netd_state_t *state, const char *name) {
  interface_t *iface;

  if (!state || !name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for interface deletion: state=%p, name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Deleting interface '%s'", name);

  /* Find interface */
  iface = interface_find(state, name);
  if (!iface) {
    debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Found interface %s in state, deleting from FreeBSD",
            name);

  /* Delete interface in FreeBSD */
  if (freebsd_interface_delete(name) < 0) {
    debug_log(DEBUG_ERROR, "Failed to delete interface %s in FreeBSD", name);
    return -1;
  }

  /* Remove from interface list */
  TAILQ_REMOVE(&state->interfaces, iface, entries);
  free(iface);

  debug_log(DEBUG_INFO, "Deleted interface %s", name);
  return 0;
}

/**
 * Remove interface from group
 * @param state Server state
 * @param name Interface name
 * @param group Group name
 * @return 0 on success, -1 on failure
 */
int interface_remove_group(netd_state_t *state, const char *name,
                           const char *group) {
  interface_t *iface;
  int i, j;

  if (!state || !name || !group) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for interface group removal: state=%p, "
              "name=%s, group=%s",
              state, name ? name : "NULL", group ? group : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Removing interface '%s' from group '%s'", name,
            group);

  /* Find interface */
  iface = interface_find(state, name);
  if (!iface) {
    debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
    return -1;
  }

  /* Find and remove group */
  for (i = 0; i < iface->group_count; i++) {
    if (strcmp(iface->groups[i], group) == 0) {
      debug_log(DEBUG_DEBUG, "Found group %s at index %d, removing", group, i);
      /* Shift remaining groups */
      for (j = i; j < iface->group_count - 1; j++) {
        strlcpy(iface->groups[j], iface->groups[j + 1], MAX_GROUP_NAME_LEN);
      }
      iface->group_count--;
      debug_log(DEBUG_INFO,
                "Removed interface %s from group %s (remaining groups: %d)",
                name, group, iface->group_count);
      return 0;
    }
  }

  debug_log(DEBUG_ERROR, "Interface %s not in group %s", name, group);
  return -1; /* Group not found */
}

/**
 * Delete interface address
 * @param state Server state
 * @param name Interface name
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int interface_delete_address(netd_state_t *state, const char *name,
                             int family) {
  interface_t *iface;

  if (!state || !name) {
    debug_log(
        DEBUG_ERROR,
        "Invalid parameters for interface address deletion: state=%p, name=%s",
        state, name ? name : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Deleting %s address from interface '%s'",
            family == AF_INET    ? "IPv4"
            : family == AF_INET6 ? "IPv6"
                                 : "unknown",
            name);

  /* Find interface */
  iface = interface_find(state, name);
  if (!iface) {
    debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
    return -1;
  }

  /* Delete address in FreeBSD */
  if (freebsd_interface_delete_address(name, family) < 0) {
    debug_log(DEBUG_ERROR,
              "Failed to delete address from interface %s in FreeBSD", name);
    return -1;
  }

  debug_log(DEBUG_INFO, "Deleted address from interface %s", name);
  return 0;
} 