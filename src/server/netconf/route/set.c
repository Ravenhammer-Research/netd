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
 * Add a route to the routing table
 * @param state Server state
 * @param fib FIB number
 * @param destination Destination address
 * @param gateway Gateway address (can be NULL for reject/blackhole)
 * @param interface Interface name (can be NULL)
 * @param flags Route flags
 * @return 0 on success, -1 on failure
 */
int route_add(netd_state_t *state, uint32_t fib, const char *destination,
              const char *gateway, const char *interface, int flags) {
  netd_route_t *route;
  int ret;

  if (!state || !destination || !is_valid_fib_number(fib)) {
    debug_log(ERROR,
              "Invalid parameters for route addition: state=%p, "
              "destination=%s, fib=%u",
              state, destination ? destination : "NULL", fib);
    return -1;
  }

  debug_log(DEBUG, "Adding route to %s via %s on %s (FIB %u, flags 0x%x)",
            destination, gateway ? gateway : "direct",
            interface ? interface : "none", fib, flags);

  /* Allocate new route entry */
  route = malloc(sizeof(*route));
  if (!route) {
    debug_log(ERROR, "Failed to allocate memory for route to %s",
              destination);
    return -1;
  }

  /* Initialize route */
  memset(route, 0, sizeof(*route));
  if (parse_address(destination, &route->destination) < 0) {
    debug_log(ERROR, "Failed to parse destination address %s",
              destination);
    free(route);
    return -1;
  }
  debug_log(DEBUG, "Parsed destination address %s successfully",
            destination);

  if (gateway && parse_address(gateway, &route->gateway) < 0) {
    debug_log(ERROR, "Failed to parse gateway address %s", gateway);
    free(route);
    return -1;
  }
  if (gateway) {
    debug_log(DEBUG, "Parsed gateway address %s successfully", gateway);
  }

  if (interface) {
    strlcpy(route->interface, interface, sizeof(route->interface));
    debug_log(DEBUG, "Set interface to %s", interface);
  }
  route->fib = fib;
  route->flags = flags;

  /* Add route in FreeBSD */
  debug_log(DEBUG, "Adding route to FreeBSD kernel");
  ret = freebsd_route_add(fib, destination, gateway, interface, flags);
  if (ret < 0) {
    debug_log(ERROR, "Failed to add route to FreeBSD kernel");
    free(route);
    return -1;
  }

  /* Add to route list */
  TAILQ_INSERT_TAIL(&state->routes, route, entries);
  debug_log(DEBUG, "Added route to internal state list");

  debug_log(INFO, "Added route to %s via %s in FIB %u", destination,
            gateway ? gateway : "direct", fib);
  return 0;
} 