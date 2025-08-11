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
#include <netconf.h>
#include <vrf.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Create a new VRF
 * @param state Server state
 * @param name VRF name
 * @param fib_number FIB number
 * @return 0 on success, -1 on failure
 */
int vrf_create(netd_state_t *state, const char *name, uint32_t fib_number) {
  vrf_t *vrf;

  if (!state || !name || !is_valid_vrf_name(name) ||
      !is_valid_fib_number(fib_number)) {
    debug_log(ERROR,
              "Invalid parameters for VRF creation: state=%p, name=%s, fib=%u",
              state, name ? name : "NULL", fib_number);
    return -1;
  }

  debug_log(DEBUG, "Creating VRF '%s' with FIB %u", name, fib_number);

  /* Check if VRF already exists */
  if (vrf_find_by_name(state, name)) {
    debug_log(ERROR, "VRF %s already exists in state", name);
    return -1;
  }

  if (vrf_find_by_fib(state, fib_number)) {
    debug_log(ERROR, "FIB %u already assigned to another VRF",
              fib_number);
    return -1;
  }

  /* Allocate new VRF */
  vrf = malloc(sizeof(*vrf));
  if (!vrf) {
    debug_log(ERROR, "Failed to allocate memory for VRF %s", name);
    return -1;
  }

  /* Initialize VRF */
  memset(vrf, 0, sizeof(*vrf));
  strlcpy(vrf->name, name, sizeof(vrf->name));
  vrf->fib_number = fib_number;

  /* Add to VRF list */
  TAILQ_INSERT_TAIL(&state->vrfs, vrf, entries);
  debug_log(DEBUG, "Added VRF %s to state list", name);

  debug_log(INFO, "Created VRF %s with FIB %u", name, fib_number);
  return 0;
} 