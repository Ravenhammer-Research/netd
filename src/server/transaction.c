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
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

/**
 * Begin a transaction
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int transaction_begin(netd_state_t *state) {
  if (!state) {
    debug_log(DEBUG_ERROR, "Invalid state parameter for transaction begin");
    return -1;
  }

  if (state->transaction_active) {
    debug_log(DEBUG_WARN,
              "Transaction already active, cannot begin new transaction");
    return -1;
  }

  state->transaction_active = true;
  debug_log(DEBUG_INFO, "Transaction begun successfully");
  return 0;
}

/**
 * Commit a transaction
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int transaction_commit(netd_state_t *state) {
  pending_change_t *change, *temp;
  int ret = 0;
  int success_count = 0;
  int failure_count = 0;

  if (!state) {
    debug_log(DEBUG_ERROR, "Invalid state parameter for transaction commit");
    return -1;
  }

  if (!state->transaction_active) {
    debug_log(DEBUG_WARN, "No transaction active, cannot commit");
    return -1;
  }

  /* Count pending changes */
  int change_count = 0;
  TAILQ_FOREACH(change, &state->pending_changes, entries) { change_count++; }

  debug_log(DEBUG_INFO, "Committing transaction with %d pending changes",
            change_count);

  if (change_count == 0) {
    debug_log(DEBUG_DEBUG,
              "No changes to commit, transaction completed successfully");
    state->transaction_active = false;
    return 0;
  }

  /* Apply all pending changes */
  int change_index = 0;
  TAILQ_FOREACH_SAFE(change, &state->pending_changes, entries, temp) {
    change_index++;
    int change_ret = 0;
    debug_log(DEBUG_DEBUG, "Applying change %d/%d (type: %d)", change_index,
              change_count, change->type);

    switch (change->type) {
    case CHANGE_VRF_CREATE:
      debug_log(DEBUG_DEBUG, "Change %d: Creating VRF %s with FIB %u",
                change_index, change->data.vrf.name, change->data.vrf.fib);
      change_ret =
          vrf_create(state, change->data.vrf.name, change->data.vrf.fib);
      break;
    case CHANGE_VRF_DELETE:
      debug_log(DEBUG_DEBUG, "Change %d: Deleting VRF %s", change_index,
                change->data.vrf.name);
      change_ret = vrf_delete(state, change->data.vrf.name);
      break;
    case CHANGE_INTERFACE_CREATE:
      debug_log(DEBUG_DEBUG, "Change %d: Creating interface %s of type %d",
                change_index, change->data.interface.name,
                change->data.interface.type);
      change_ret = interface_create(state, change->data.interface.name,
                                    change->data.interface.type);
      break;
    case CHANGE_INTERFACE_DELETE:
      debug_log(DEBUG_DEBUG, "Change %d: Deleting interface %s", change_index,
                change->data.interface.name);
      change_ret = interface_delete(state, change->data.interface.name);
      break;
    case CHANGE_INTERFACE_SET_FIB:
      debug_log(DEBUG_DEBUG, "Change %d: Setting FIB %u for interface %s",
                change_index, change->data.interface.fib,
                change->data.interface.name);
      change_ret = interface_set_fib(state, change->data.interface.name,
                                     change->data.interface.fib);
      break;
    case CHANGE_INTERFACE_SET_ADDRESS:
      debug_log(DEBUG_DEBUG, "Change %d: Setting address %s for interface %s",
                change_index, change->data.interface.address,
                change->data.interface.name);
      change_ret = interface_set_address(state, change->data.interface.name,
                                         change->data.interface.address,
                                         change->data.interface.family);
      break;
    case CHANGE_INTERFACE_DELETE_ADDRESS:
      debug_log(DEBUG_DEBUG, "Change %d: Deleting address from interface %s",
                change_index, change->data.interface.name);
      change_ret = interface_delete_address(state, change->data.interface.name,
                                            change->data.interface.family);
      break;
    case CHANGE_INTERFACE_SET_MTU:
      debug_log(DEBUG_DEBUG, "Change %d: Setting MTU %d for interface %s",
                change_index, change->data.interface.mtu,
                change->data.interface.name);
      change_ret = interface_set_mtu(state, change->data.interface.name,
                                     change->data.interface.mtu);
      break;
    case CHANGE_INTERFACE_ADD_GROUP:
      debug_log(DEBUG_DEBUG, "Change %d: Adding interface %s to group %s",
                change_index, change->data.interface.name,
                change->data.interface.group);
      change_ret = interface_add_group(state, change->data.interface.name,
                                       change->data.interface.group);
      break;
    case CHANGE_INTERFACE_REMOVE_GROUP:
      debug_log(DEBUG_DEBUG, "Change %d: Removing interface %s from group %s",
                change_index, change->data.interface.name,
                change->data.interface.group);
      change_ret = interface_remove_group(state, change->data.interface.name,
                                          change->data.interface.group);
      break;
    case CHANGE_ROUTE_ADD:
      debug_log(DEBUG_DEBUG, "Change %d: Adding route to %s via %s",
                change_index, change->data.route.destination,
                change->data.route.gateway);
      change_ret =
          route_add(state, change->data.route.fib,
                    change->data.route.destination, change->data.route.gateway,
                    change->data.route.interface, change->data.route.flags);
      break;
    case CHANGE_ROUTE_DELETE:
      debug_log(DEBUG_DEBUG, "Change %d: Deleting route to %s", change_index,
                change->data.route.destination);
      change_ret = route_delete(state, change->data.route.fib,
                                change->data.route.destination);
      break;
    default:
      debug_log(DEBUG_ERROR, "Change %d: Unknown change type %d", change_index,
                change->type);
      change_ret = -1;
      break;
    }

    if (change_ret < 0) {
      debug_log(DEBUG_ERROR, "Change %d/%d failed (type: %d)", change_index,
                change_count, change->type);
      failure_count++;
      ret = -1;
    } else {
      debug_log(DEBUG_DEBUG, "Change %d/%d completed successfully",
                change_index, change_count);
      success_count++;
    }

    /* Remove change from pending list */
    TAILQ_REMOVE(&state->pending_changes, change, entries);
    free(change);
  }

  state->transaction_active = false;

  if (ret == 0) {
    debug_log(DEBUG_INFO,
              "Transaction committed successfully: %d changes applied",
              success_count);
  } else {
    debug_log(DEBUG_ERROR,
              "Transaction commit failed: %d successful, %d failed",
              success_count, failure_count);
  }

  return ret;
}

/**
 * Rollback a transaction
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int transaction_rollback(netd_state_t *state) {
  pending_change_t *change, *temp;

  if (!state) {
    debug_log(DEBUG_ERROR, "Invalid state parameter for transaction rollback");
    return -1;
  }

  if (!state->transaction_active) {
    debug_log(DEBUG_WARN, "No transaction active, cannot rollback");
    return -1;
  }

  /* Count pending changes */
  int change_count = 0;
  TAILQ_FOREACH(change, &state->pending_changes, entries) { change_count++; }

  debug_log(DEBUG_INFO, "Rolling back transaction with %d pending changes",
            change_count);

  if (change_count == 0) {
    debug_log(DEBUG_DEBUG, "No changes to rollback, transaction completed");
    state->transaction_active = false;
    return 0;
  }

  /* Remove all pending changes */
  int removed_count = 0;
  TAILQ_FOREACH_SAFE(change, &state->pending_changes, entries, temp) {
    TAILQ_REMOVE(&state->pending_changes, change, entries);
    free(change);
    removed_count++;
  }

  state->transaction_active = false;
  debug_log(DEBUG_INFO,
            "Transaction rolled back successfully: %d changes discarded",
            removed_count);
  return 0;
}

/**
 * Add a pending VRF creation change
 * @param state Server state
 * @param name VRF name
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
int add_pending_vrf_create(netd_state_t *state, const char *name,
                           uint32_t fib) {
  pending_change_t *change;

  if (!state || !name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for pending VRF creation: state=%p, name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  if (!state->transaction_active) {
    debug_log(DEBUG_ERROR, "No active transaction for pending VRF creation");
    return -1;
  }

  change = malloc(sizeof(*change));
  if (!change) {
    debug_log(DEBUG_ERROR,
              "Failed to allocate memory for pending VRF creation");
    return -1;
  }

  change->type = CHANGE_VRF_CREATE;
  strlcpy(change->data.vrf.name, name, sizeof(change->data.vrf.name));
  change->data.vrf.fib = fib;

  TAILQ_INSERT_TAIL(&state->pending_changes, change, entries);
  debug_log(DEBUG_DEBUG, "Added pending VRF creation: %s (FIB %u)", name, fib);
  return 0;
}

/**
 * Add a pending VRF deletion change
 * @param state Server state
 * @param name VRF name
 * @return 0 on success, -1 on failure
 */
int add_pending_vrf_delete(netd_state_t *state, const char *name) {
  pending_change_t *change;

  if (!state || !name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for pending VRF deletion: state=%p, name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  if (!state->transaction_active) {
    debug_log(DEBUG_ERROR, "No active transaction for pending VRF deletion");
    return -1;
  }

  change = malloc(sizeof(*change));
  if (!change) {
    debug_log(DEBUG_ERROR,
              "Failed to allocate memory for pending VRF deletion");
    return -1;
  }

  change->type = CHANGE_VRF_DELETE;
  strlcpy(change->data.vrf.name, name, sizeof(change->data.vrf.name));

  TAILQ_INSERT_TAIL(&state->pending_changes, change, entries);
  debug_log(DEBUG_DEBUG, "Added pending VRF deletion: %s", name);
  return 0;
}

/**
 * Add a pending interface creation change
 * @param state Server state
 * @param name Interface name
 * @param type Interface type
 * @return 0 on success, -1 on failure
 */
int add_pending_interface_create(netd_state_t *state, const char *name,
                                 interface_type_t type) {
  pending_change_t *change;

  if (!state || !name) {
    debug_log(
        DEBUG_ERROR,
        "Invalid parameters for pending interface creation: state=%p, name=%s",
        state, name ? name : "NULL");
    return -1;
  }

  if (!state->transaction_active) {
    debug_log(DEBUG_ERROR,
              "No active transaction for pending interface creation");
    return -1;
  }

  change = malloc(sizeof(*change));
  if (!change) {
    debug_log(DEBUG_ERROR,
              "Failed to allocate memory for pending interface creation");
    return -1;
  }

  change->type = CHANGE_INTERFACE_CREATE;
  strlcpy(change->data.interface.name, name,
          sizeof(change->data.interface.name));
  change->data.interface.type = type;

  TAILQ_INSERT_TAIL(&state->pending_changes, change, entries);
  debug_log(DEBUG_DEBUG, "Added pending interface creation: %s (%s)", name,
            interface_type_to_string(type));
  return 0;
}

/**
 * Add a pending interface deletion change
 * @param state Server state
 * @param name Interface name
 * @return 0 on success, -1 on failure
 */
int add_pending_interface_delete(netd_state_t *state, const char *name) {
  pending_change_t *change;

  if (!state || !name) {
    debug_log(
        DEBUG_ERROR,
        "Invalid parameters for pending interface deletion: state=%p, name=%s",
        state, name ? name : "NULL");
    return -1;
  }

  if (!state->transaction_active) {
    debug_log(DEBUG_ERROR,
              "No active transaction for pending interface deletion");
    return -1;
  }

  change = malloc(sizeof(*change));
  if (!change) {
    debug_log(DEBUG_ERROR,
              "Failed to allocate memory for pending interface deletion");
    return -1;
  }

  change->type = CHANGE_INTERFACE_DELETE;
  strlcpy(change->data.interface.name, name,
          sizeof(change->data.interface.name));

  TAILQ_INSERT_TAIL(&state->pending_changes, change, entries);
  debug_log(DEBUG_DEBUG, "Added pending interface deletion: %s", name);
  return 0;
}

/**
 * Add a pending interface FIB change
 * @param state Server state
 * @param name Interface name
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
int add_pending_interface_set_fib(netd_state_t *state, const char *name,
                                  uint32_t fib) {
  pending_change_t *change;

  if (!state || !name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for pending interface FIB change: state=%p, "
              "name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  if (!state->transaction_active) {
    debug_log(DEBUG_ERROR,
              "No active transaction for pending interface FIB change");
    return -1;
  }

  change = malloc(sizeof(*change));
  if (!change) {
    debug_log(DEBUG_ERROR,
              "Failed to allocate memory for pending interface FIB change");
    return -1;
  }

  change->type = CHANGE_INTERFACE_SET_FIB;
  strlcpy(change->data.interface.name, name,
          sizeof(change->data.interface.name));
  change->data.interface.fib = fib;

  TAILQ_INSERT_TAIL(&state->pending_changes, change, entries);
  debug_log(DEBUG_DEBUG, "Added pending interface FIB change: %s -> FIB %u",
            name, fib);
  return 0;
}

/**
 * Add a pending route addition change
 * @param state Server state
 * @param fib FIB number
 * @param destination Destination address
 * @param gateway Gateway address
 * @param interface Interface name
 * @param flags Route flags
 * @return 0 on success, -1 on failure
 */
int add_pending_route_add(netd_state_t *state, uint32_t fib,
                          const char *destination, const char *gateway,
                          const char *interface, int flags) {
  pending_change_t *change;

  if (!state || !destination) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for pending route addition: state=%p, "
              "destination=%s",
              state, destination ? destination : "NULL");
    return -1;
  }

  if (!state->transaction_active) {
    debug_log(DEBUG_ERROR, "No active transaction for pending route addition");
    return -1;
  }

  change = malloc(sizeof(*change));
  if (!change) {
    debug_log(DEBUG_ERROR,
              "Failed to allocate memory for pending route addition");
    return -1;
  }

  change->type = CHANGE_ROUTE_ADD;
  change->data.route.fib = fib;
  strlcpy(change->data.route.destination, destination,
          sizeof(change->data.route.destination));
  if (gateway) {
    strlcpy(change->data.route.gateway, gateway,
            sizeof(change->data.route.gateway));
  } else {
    change->data.route.gateway[0] = '\0';
  }
  if (interface) {
    strlcpy(change->data.route.interface, interface,
            sizeof(change->data.route.interface));
  } else {
    change->data.route.interface[0] = '\0';
  }
  change->data.route.flags = flags;

  TAILQ_INSERT_TAIL(&state->pending_changes, change, entries);
  debug_log(DEBUG_DEBUG, "Added pending route addition: %s via %s (FIB %u)",
            destination, gateway ? gateway : "direct", fib);
  return 0;
}

/**
 * Add a pending route deletion change
 * @param state Server state
 * @param fib FIB number
 * @param destination Destination address
 * @return 0 on success, -1 on failure
 */
int add_pending_route_delete(netd_state_t *state, uint32_t fib,
                             const char *destination) {
  pending_change_t *change;

  if (!state || !destination) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for pending route deletion: state=%p, "
              "destination=%s",
              state, destination ? destination : "NULL");
    return -1;
  }

  if (!state->transaction_active) {
    debug_log(DEBUG_ERROR, "No active transaction for pending route deletion");
    return -1;
  }

  change = malloc(sizeof(*change));
  if (!change) {
    debug_log(DEBUG_ERROR,
              "Failed to allocate memory for pending route deletion");
    return -1;
  }

  change->type = CHANGE_ROUTE_DELETE;
  change->data.route.fib = fib;
  strlcpy(change->data.route.destination, destination,
          sizeof(change->data.route.destination));
  change->data.route.gateway[0] = '\0';
  change->data.route.interface[0] = '\0';
  change->data.route.flags = 0;

  TAILQ_INSERT_TAIL(&state->pending_changes, change, entries);
  debug_log(DEBUG_DEBUG, "Added pending route deletion: %s (FIB %u)",
            destination, fib);
  return 0;
}