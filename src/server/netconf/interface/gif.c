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
#include <system/freebsd/gif/gif.h>

/**
 * Create a GIF interface
 * @param state Server state
 * @param name GIF interface name
 * @return 0 on success, -1 on failure
 */
int gif_interface_create(netd_state_t *state, const char *name) {
  if (!state || !name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for GIF creation: state=%p, name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Creating GIF interface '%s'", name);

  /* Create the GIF interface using the general interface creation */
  int result = interface_create(state, name, IF_TYPE_GIF);
  if (result < 0) {
    debug_log(DEBUG_ERROR, "Failed to create GIF interface %s", name);
    return -1;
  }

  /* Create GIF in FreeBSD */
  if (freebsd_gif_create(name) < 0) {
    debug_log(DEBUG_ERROR,
              "Failed to create GIF interface %s in FreeBSD", name);
    /* Clean up the interface from state */
    interface_delete(state, name);
    return -1;
  }

  debug_log(DEBUG_INFO, "Created GIF interface %s", name);
  return 0;
}

/**
 * Set GIF tunnel endpoints
 * @param state Server state
 * @param name GIF interface name
 * @param local_addr Local tunnel address
 * @param remote_addr Remote tunnel address
 * @return 0 on success, -1 on failure
 */
int gif_set_tunnel(netd_state_t *state, const char *name,
                    const char *local_addr, const char *remote_addr) {
  interface_t *gif_iface;

  if (!state || !name || !local_addr || !remote_addr) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for GIF tunnel setting: state=%p, name=%s, "
              "local=%s, remote=%s",
              state, name ? name : "NULL", local_addr ? local_addr : "NULL",
              remote_addr ? remote_addr : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG,
            "Setting tunnel endpoints for GIF interface '%s': local=%s, remote=%s",
            name, local_addr, remote_addr);

  /* Find GIF interface */
  gif_iface = interface_find(state, name);
  if (!gif_iface) {
    debug_log(DEBUG_ERROR, "GIF interface %s not found", name);
    return -1;
  }

  if (gif_iface->type != IF_TYPE_GIF) {
    debug_log(DEBUG_ERROR, "Interface %s is not a GIF", name);
    return -1;
  }

  /* Set tunnel endpoints in FreeBSD */
  if (freebsd_gif_set_tunnel(name, local_addr, remote_addr) < 0) {
    debug_log(DEBUG_ERROR,
              "Failed to set tunnel endpoints for GIF %s in FreeBSD", name);
    return -1;
  }

  /* Update interface state */
  strlcpy(gif_iface->tunnel_local, local_addr, sizeof(gif_iface->tunnel_local));
  strlcpy(gif_iface->tunnel_remote, remote_addr, sizeof(gif_iface->tunnel_remote));

  debug_log(DEBUG_INFO,
            "Set tunnel endpoints for GIF interface %s: local=%s, remote=%s",
            name, local_addr, remote_addr);
  return 0;
}

/**
 * Get GIF tunnel endpoints
 * @param state Server state
 * @param name GIF interface name
 * @param local_addr Output parameter for local tunnel address
 * @param local_len Length of local_addr buffer
 * @param remote_addr Output parameter for remote tunnel address
 * @param remote_len Length of remote_addr buffer
 * @return 0 on success, -1 on failure
 */
int gif_get_tunnel(netd_state_t *state, const char *name, char *local_addr,
                    size_t local_len, char *remote_addr, size_t remote_len) {
  interface_t *gif_iface;

  if (!state || !name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for GIF tunnel retrieval: state=%p, name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  /* Find GIF interface */
  gif_iface = interface_find(state, name);
  if (!gif_iface) {
    debug_log(DEBUG_ERROR, "GIF interface %s not found", name);
    return -1;
  }

  if (gif_iface->type != IF_TYPE_GIF) {
    debug_log(DEBUG_ERROR, "Interface %s is not a GIF", name);
    return -1;
  }

  /* Return tunnel endpoints */
  if (local_addr && local_len > 0) {
    strlcpy(local_addr, gif_iface->tunnel_local, local_len);
  }
  if (remote_addr && remote_len > 0) {
    strlcpy(remote_addr, gif_iface->tunnel_remote, remote_len);
  }

  debug_log(DEBUG_DEBUG,
            "Retrieved tunnel endpoints for GIF interface %s: local=%s, remote=%s",
            name, gif_iface->tunnel_local, gif_iface->tunnel_remote);
  return 0;
} 