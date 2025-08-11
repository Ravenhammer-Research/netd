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
#include <system/freebsd/epair/epair.h>

/**
 * Create an epair interface
 * @param state Server state
 * @param name Epair interface name
 * @return 0 on success, -1 on failure
 */
int epair_interface_create(netd_state_t *state, const char *name) {
  if (!state || !name) {
    debug_log(ERROR,
              "Invalid parameters for epair creation: state=%p, name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Creating epair interface '%s'", name);

  /* Create the epair interface using the general interface creation */
  int result = interface_create(state, name, IF_TYPE_EPAIR);
  if (result < 0) {
    debug_log(ERROR, "Failed to create epair interface %s", name);
    return -1;
  }

  /* Create epair in FreeBSD */
  if (freebsd_epair_create(name) < 0) {
    debug_log(ERROR,
              "Failed to create epair interface %s in FreeBSD", name);
    /* Clean up the interface from state */
    interface_delete(state, name);
    return -1;
  }

  debug_log(INFO, "Created epair interface %s", name);
  return 0;
}

/**
 * Set epair peer interface
 * @param state Server state
 * @param name Epair interface name
 * @param peer_name Peer interface name
 * @return 0 on success, -1 on failure
 */
int epair_set_peer(netd_state_t *state, const char *name,
                    const char *peer_name) {
  interface_t *epair_iface;

  if (!state || !name || !peer_name) {
    debug_log(ERROR,
              "Invalid parameters for epair peer setting: state=%p, name=%s, "
              "peer=%s",
              state, name ? name : "NULL", peer_name ? peer_name : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Setting peer '%s' for epair interface '%s'",
            peer_name, name);

  /* Find epair interface */
  epair_iface = interface_find(state, name);
  if (!epair_iface) {
    debug_log(ERROR, "Epair interface %s not found", name);
    return -1;
  }

  if (epair_iface->type != IF_TYPE_EPAIR) {
    debug_log(ERROR, "Interface %s is not an epair", name);
    return -1;
  }

  /* Set peer in FreeBSD */
  if (freebsd_epair_set_peer(name, peer_name) < 0) {
    debug_log(ERROR,
              "Failed to set peer %s for epair %s in FreeBSD", peer_name, name);
    return -1;
  }

  /* Update interface state */
  strlcpy(epair_iface->peer_name, peer_name, sizeof(epair_iface->peer_name));

  debug_log(INFO, "Set peer %s for epair interface %s", peer_name, name);
  return 0;
}

/**
 * Get epair peer interface
 * @param state Server state
 * @param name Epair interface name
 * @param peer_name Output parameter for peer interface name
 * @param peer_len Length of peer_name buffer
 * @return 0 on success, -1 on failure
 */
int epair_get_peer(netd_state_t *state, const char *name, char *peer_name,
                    size_t peer_len) {
  interface_t *epair_iface;

  if (!state || !name || !peer_name || peer_len == 0) {
    debug_log(ERROR,
              "Invalid parameters for epair peer retrieval: state=%p, name=%s, "
              "peer_name=%p, peer_len=%zu",
              state, name ? name : "NULL", peer_name, peer_len);
    return -1;
  }

  /* Find epair interface */
  epair_iface = interface_find(state, name);
  if (!epair_iface) {
    debug_log(ERROR, "Epair interface %s not found", name);
    return -1;
  }

  if (epair_iface->type != IF_TYPE_EPAIR) {
    debug_log(ERROR, "Interface %s is not an epair", name);
    return -1;
  }

  /* Return peer name */
  strlcpy(peer_name, epair_iface->peer_name, peer_len);

  debug_log(DEBUG, "Retrieved peer %s for epair interface %s",
            epair_iface->peer_name, name);
  return 0;
} 