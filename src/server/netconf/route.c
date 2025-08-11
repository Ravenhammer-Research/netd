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

/**
 * Query route table and return as XML for NETCONF response
 * @param state Server state
 * @param fib FIB number to query
 * @return XML string (allocated) or NULL on failure
 */
char *route_table_query(netd_state_t *state, uint32_t fib) {
  netd_route_t *route;
  char *xml = NULL;
  char *temp_xml = NULL;
  char dest_str[INET6_ADDRSTRLEN];
  char gw_str[INET6_ADDRSTRLEN];
  char netmask_str[INET6_ADDRSTRLEN];
  int route_count = 0;

  if (!state) {
    debug_log(ERROR, "Invalid state parameter for route XML generation");
    return NULL;
  }

  debug_log(DEBUG, "Generating XML for all routes");

  /* Clear existing routes from local state (not the actual routing table) */
  route_clear_all(state);

  /* Populate state with current system routes for the specified FIB */
  if (freebsd_route_list(state, fib, AF_UNSPEC) < 0) {
    debug_log(ERROR, "Failed to get routes from system for FIB %u", fib);
    return NULL;
  }

  /* Start XML */
  asprintf(&xml,
           "    <routing xmlns=\"urn:ietf:params:xml:ns:yang:ietf-routing\">\n"
           "      <ribs>\n"
           "        <rib>\n"
           "          <name>vrf%u</name>\n"
           "          <routes>\n",
           fib);
  if (!xml) {
    debug_log(ERROR, "Failed to allocate memory for route XML header");
    return NULL;
  }

  debug_log(DEBUG, "Processing routes from state...");
  TAILQ_FOREACH(route, &state->routes, entries) {
    route_count++;
    debug_log(DEBUG, "Processing route %d from state", route_count);

    /* Format addresses */
    if (format_address(&route->destination, dest_str, sizeof(dest_str)) < 0) {
      strlcpy(dest_str, "invalid", sizeof(dest_str));
      debug_log(WARN, "Failed to format destination address for route %d",
                route_count);
    }

    if (route->gateway.ss_family != AF_UNSPEC) {
      if (format_address(&route->gateway, gw_str, sizeof(gw_str)) < 0) {
        strlcpy(gw_str, "invalid", sizeof(gw_str));
        debug_log(WARN, "Failed to format gateway address for route %d",
                  route_count);
      }
    } else {
      strlcpy(gw_str, "", sizeof(gw_str));
    }

    if (format_address(&route->netmask, netmask_str, sizeof(netmask_str)) < 0) {
      strlcpy(netmask_str, "", sizeof(netmask_str));
      debug_log(WARN, "Failed to format netmask for route %d",
                route_count);
    }

    debug_log(DEBUG, "Adding route %d: %s via %s on %s", route_count,
              dest_str, gw_str, route->interface);

    asprintf(&temp_xml,
             "            <route>\n"
             "              <destination-prefix>%s</destination-prefix>\n"
             "              <next-hop>\n"
             "                <next-hop-address>%s</next-hop-address>\n"
             "                <outgoing-interface>%s</outgoing-interface>\n"
             "              </next-hop>\n"
             "              <prefix-length>%d</prefix-length>\n"
             "              <scope-interface>%s</scope-interface>\n"
             "              <flags>%d</flags>\n"
             "              <expire>%d</expire>\n"
             "            </route>\n",
             dest_str,
             gw_str,
             route->interface[0] != '\0' ? route->interface : "",
             route->prefix_length,
             route->scope_interface[0] != '\0' ? route->scope_interface : "",
             route->flags,
             route->expire);

    if (temp_xml) {
      char *new_xml;
      asprintf(&new_xml, "%s%s", xml, temp_xml);
      free(xml);
      free(temp_xml);
      xml = new_xml;
    }
  }

  /* Close XML tags */
  asprintf(&temp_xml, "          </routes>\n"
                      "        </rib>\n"
                      "      </ribs>\n"
                      "    </routing>\n");
  if (temp_xml) {
    char *new_xml;
    asprintf(&new_xml, "%s%s", xml, temp_xml);
    free(xml);
    free(temp_xml);
    xml = new_xml;
  }

  debug_log(INFO, "Generated XML for %d routes (%zu bytes)", route_count,
            xml ? strlen(xml) : 0);
  return xml;
}