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
#include <route.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system/freebsd/route/route.h>

/**
 * Delete a route from the routing table
 * @param state Server state
 * @param fib FIB number
 * @param destination Destination address
 * @return 0 on success, -1 on failure
 */
int route_delete(netd_state_t *state, uint32_t fib, const char *destination) {
  netd_route_t *route;
  struct sockaddr_storage dest_addr;

  if (!state || !destination || !is_valid_fib_number(fib)) {
    debug_log(ERROR,
              "Invalid parameters for route deletion: state=%p, "
              "destination=%s, fib=%u",
              state, destination ? destination : "NULL", fib);
    return -1;
  }

  debug_log(DEBUG, "Deleting route to %s in FIB %u", destination, fib);

  /* Parse destination address */
  if (parse_address(destination, &dest_addr) < 0) {
    debug_log(ERROR, "Failed to parse destination address %s",
              destination);
    return -1;
  }
  debug_log(DEBUG, "Parsed destination address %s successfully",
            destination);

  /* Find and remove route from list */
  int found_in_state = 0;
  TAILQ_FOREACH(route, &state->routes, entries) {
    if (route->fib == fib &&
        memcmp(&route->destination, &dest_addr, sizeof(dest_addr)) == 0) {
      TAILQ_REMOVE(&state->routes, route, entries);
      free(route);
      found_in_state = 1;
      debug_log(DEBUG, "Removed route from internal state list");
      break;
    }
  }

  if (!found_in_state) {
    debug_log(DEBUG, "Route not found in internal state list");
  }

  /* Delete route in FreeBSD */
  debug_log(DEBUG, "Deleting route from FreeBSD kernel");
  if (freebsd_route_delete(fib, destination) < 0) {
    debug_log(ERROR, "Failed to delete route from FreeBSD kernel");
    return -1;
  }

  debug_log(INFO, "Deleted route to %s in FIB %u", destination, fib);
  return 0;
}

/**
 * Flush all routes for a FIB
 * @param state Server state
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
int route_flush_fib(netd_state_t *state, uint32_t fib) {
  debug_log(DEBUG1, "route_flush_fib called with state=%p, fib=%u", state, fib);
  
  /* Not implemented: FreeBSD doesn't have a direct "flush all routes for FIB" operation */
  /* We would need to iterate through all routes and delete them individually */
  return -1;
}

/**
 * Clear all routes from state
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int route_clear_all(netd_state_t *state) {
  netd_route_t *route, *next;
  int cleared_count = 0;

  if (!state) {
    return -1;
  }

  TAILQ_FOREACH_SAFE(route, &state->routes, entries, next) {
    TAILQ_REMOVE(&state->routes, route, entries);
    free(route);
    cleared_count++;
  }

  return cleared_count;
} 