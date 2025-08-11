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

/**
 * Create routes XML response for NETCONF
 * @param state Server state
 * @param message_id Message ID for the response
 * @param fib FIB number to filter routes
 * @return Routes XML response string (allocated)
 */
char *create_routes_xml_response(netd_state_t *state, const char *message_id, uint32_t fib) {
  char *response;
  char *routes_xml;

  if (!state || !message_id) {
    return NULL;
  }

  /* Get routes XML for the specified FIB */
  routes_xml = route_table_query(state, fib);
  if (!routes_xml) {
    debug_log(ERROR, "Failed to get routes XML for FIB %u", fib);
    return NULL;
  }

  /* Allocate buffer for response */
  response = malloc(NETCONF_RESPONSE_BUFFER_SIZE);
  if (!response) {
    debug_log(ERROR, "Failed to allocate memory for routes response");
    free(routes_xml);
    return NULL;
  }

  if (prepare_response(response,
                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                 "message-id=\"%s\">\n"
                 "  <data>\n"
                 "%s"
                 "  </data>\n"
                 "</rpc-reply>\n",
                 message_id, routes_xml) == -1) {
    free(routes_xml);
    free(response);
    return NULL;
  }

  free(routes_xml);
  debug_log(INFO, "Created routes XML response for FIB %u", fib);
  return response;
} 