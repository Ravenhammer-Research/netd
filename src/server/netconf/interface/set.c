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
 * Set interface FIB assignment
 * @param state Server state
 * @param name Interface name
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
int interface_set_fib(netd_state_t *state, const char *name, uint32_t fib) {
  interface_t *iface;

  if (!state || !name || !is_valid_fib_number(fib)) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for interface FIB assignment: state=%p, "
              "name=%s, fib=%u",
              state, name ? name : "NULL", fib);
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Setting FIB %u for interface '%s'", fib, name);

  /* Find interface */
  iface = interface_find(state, name);
  if (!iface) {
    debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Found interface %s in state, setting FIB in FreeBSD",
            name);

  /* Set FIB in FreeBSD */
  if (freebsd_interface_set_fib(name, fib) < 0) {
    debug_log(DEBUG_ERROR, "Failed to set FIB %u for interface %s in FreeBSD",
              fib, name);
    return -1;
  }

  /* Update interface state */
  iface->fib = fib;

  debug_log(DEBUG_INFO, "Set FIB %u for interface %s", fib, name);
  return 0;
}

/**
 * Add interface to group
 * @param state Server state
 * @param name Interface name
 * @param group Group name
 * @return 0 on success, -1 on failure
 */
int interface_add_group(netd_state_t *state, const char *name,
                        const char *group) {
  interface_t *iface;
  int i;

  if (!state || !name || !group) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for interface group addition: state=%p, "
              "name=%s, group=%s",
              state, name ? name : "NULL", group ? group : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Adding interface '%s' to group '%s'", name, group);

  /* Find interface */
  iface = interface_find(state, name);
  if (!iface) {
    debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
    return -1;
  }

  /* Check if already in group */
  for (i = 0; i < iface->group_count; i++) {
    if (strcmp(iface->groups[i], group) == 0) {
      debug_log(DEBUG_DEBUG, "Interface %s already in group %s", name, group);
      return 0; /* Already in group */
    }
  }

  /* Check if group limit reached */
  if (iface->group_count >= MAX_GROUPS_PER_IF) {
    debug_log(DEBUG_ERROR,
              "Interface %s has reached maximum number of groups (%d)", name,
              MAX_GROUPS_PER_IF);
    return -1;
  }

  /* Add to group */
  strlcpy(iface->groups[iface->group_count], group, MAX_GROUP_NAME_LEN);
  iface->group_count++;

  debug_log(DEBUG_INFO, "Added interface %s to group %s (total groups: %d)",
            name, group, iface->group_count);
  return 0;
}

/**
 * Set interface address
 * @param state Server state
 * @param name Interface name
 * @param address Address string
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int interface_set_address(netd_state_t *state, const char *name,
                          const char *address, int family) {
  interface_t *iface;

  if (!state || !name || !address) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for interface address assignment: state=%p, "
              "name=%s, address=%s",
              state, name ? name : "NULL", address ? address : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Setting %s address '%s' for interface '%s'",
            family == AF_INET    ? "IPv4"
            : family == AF_INET6 ? "IPv6"
                                 : "unknown",
            address, name);

  /* Find interface */
  iface = interface_find(state, name);
  if (!iface) {
    debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
    return -1;
  }

  /* Set address in FreeBSD */
  if (freebsd_interface_set_address(name, address, family) < 0) {
    debug_log(DEBUG_ERROR,
              "Failed to set address %s for interface %s in FreeBSD", address,
              name);
    return -1;
  }

  debug_log(DEBUG_INFO, "Set address %s for interface %s", address, name);
  return 0;
}

/**
 * Set interface MTU
 * @param state Server state
 * @param name Interface name
 * @param mtu MTU value
 * @return 0 on success, -1 on failure
 */
int interface_set_mtu(netd_state_t *state, const char *name, int mtu) {
  interface_t *iface;

  if (!state || !name || mtu <= 0) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for interface MTU setting: state=%p, "
              "name=%s, mtu=%d",
              state, name ? name : "NULL", mtu);
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Setting MTU %d for interface '%s'", mtu, name);

  /* Find interface */
  iface = interface_find(state, name);
  if (!iface) {
    debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
    return -1;
  }

  /* Set MTU in FreeBSD */
  if (freebsd_interface_set_mtu(name, mtu) < 0) {
    debug_log(DEBUG_ERROR, "Failed to set MTU %d for interface %s in FreeBSD",
              mtu, name);
    return -1;
  }

  /* Update interface state */
  iface->mtu = mtu;

  debug_log(DEBUG_INFO, "Set MTU %d for interface %s", mtu, name);
  return 0;
}

/**
 * Create a new interface or find existing one
 * @param state Server state
 * @param name Interface name
 * @param type Interface type
 * @return 0 on success, -1 on failure
 */
int interface_create(netd_state_t *state, const char *name,
                     interface_type_t type) {
  interface_t *iface;

  if (!state || !name || !is_valid_interface_name(name)) {
    debug_log(
        DEBUG_ERROR,
        "Invalid parameters for interface creation: state=%p, name=%s, type=%d",
        state, name ? name : "NULL", type);
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Creating interface '%s' of type '%s'", name,
            interface_type_to_string(type));

  /* Check if interface already exists in our list */
  iface = interface_find(state, name);
  if (iface) {
    debug_log(DEBUG_DEBUG,
              "Interface %s already exists in state, updating configuration",
              name);
    return 0; /* Interface exists, just return success */
  }

  /* Check if this is a hardware interface that already exists in the system */
  bool is_hardware = freebsd_is_hardware_interface(type);
  debug_log(DEBUG_DEBUG, "Interface %s (type: %s) is %s", name,
            interface_type_to_string(type),
            is_hardware ? "hardware" : "virtual");

  /* Allocate new interface */
  iface = malloc(sizeof(*iface));
  if (!iface) {
    debug_log(DEBUG_ERROR, "Failed to allocate memory for interface %s", name);
    return -1;
  }

  /* Initialize interface */
  memset(iface, 0, sizeof(*iface));
  strlcpy(iface->name, name, sizeof(iface->name));
  iface->type = type;
  iface->fib = 0; /* Default FIB */
  iface->group_count = 0;
  iface->enabled = true;

  /* Only create interface in FreeBSD if it's not a hardware interface */
  if (!is_hardware) {
    /* Check if interface already exists in system */
    if (freebsd_interface_exists(name)) {
      debug_log(DEBUG_DEBUG,
                "Interface %s already exists in system, adding to state", name);
    } else {
      debug_log(DEBUG_DEBUG, "Creating virtual interface %s in FreeBSD", name);
      int result = freebsd_interface_create(name, type);
      if (result < 0) {
        debug_log(DEBUG_ERROR, "Failed to create interface %s in FreeBSD",
                  name);
        free(iface);
        return -1;
      }
      debug_log(DEBUG_INFO, "Created virtual interface %s of type %s", name,
                interface_type_to_string(type));
    }
  } else {
    debug_log(DEBUG_INFO,
              "Adding existing hardware interface %s of type %s to state", name,
              interface_type_to_string(type));
  }

  /* Add to interface list */
  TAILQ_INSERT_TAIL(&state->interfaces, iface, entries);
  debug_log(DEBUG_DEBUG, "Added interface %s to state list", name);

  return 0;
}

/**
 * Add an interface to the system
 * @param state Server state
 * @param name Interface name
 * @return 0 on success, -1 on failure
 */
int interface_add(netd_state_t *state, const char *name) {
  if (!state || !name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for interface add: state=%p, name=%s", state,
              name ? name : "NULL");
    return -1;
  }

  /* Check if interface already exists */
  if (interface_find(state, name)) {
    debug_log(DEBUG_WARN, "Interface %s already exists in state", name);
    return 0; /* Already exists, consider it a success */
  }

  /* For now, just add to state without creating in system */
  /* This is a placeholder - in a real implementation, you'd create the
   * interface */
  debug_log(DEBUG_INFO,
            "Adding interface %s to state (placeholder implementation)", name);
  return 0;
}

/**
 * Modify an interface property
 * @param state Server state
 * @param name Interface name
 * @param property Property to modify
 * @param value New value
 * @return 0 on success, -1 on failure
 */
int interface_modify(netd_state_t *state, const char *name,
                     const char *property, const char *value) {
  if (!state || !name || !property || !value) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for interface modify: state=%p, name=%s, "
              "property=%s, value=%s",
              state, name ? name : "NULL", property ? property : "NULL",
              value ? value : "NULL");
    return -1;
  }

  /* Find the interface */
  interface_t *iface = interface_find(state, name);
  if (!iface) {
    debug_log(DEBUG_ERROR, "Interface %s not found", name);
    return -1;
  }

  /* Handle different properties */
  if (strcmp(property, "vrf") == 0) {
    /* Set VRF/FIB for interface */
    char *endptr;
    uint32_t fib = strtoul(value, &endptr, 10);
    if (*endptr != '\0') {
      debug_log(DEBUG_ERROR, "Invalid VRF/FIB number: %s", value);
      return -1;
    }
    return interface_set_fib(state, name, fib);
  } else if (strcmp(property, "peer") == 0) {
    /* Handle peer property for epair interfaces */
    debug_log(DEBUG_INFO, "Setting peer %s for interface %s", value, name);
    /* This would typically involve setting up the epair peer relationship */
    return 0;
  } else {
    debug_log(DEBUG_WARN, "Unsupported interface property: %s", property);
    return -1;
  }
} 