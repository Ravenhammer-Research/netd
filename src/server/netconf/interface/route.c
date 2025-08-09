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

#include "../../system/freebsd/proto.h"
#include "../../netd.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Create routes XML response
 * @param state Server state
 * @param message_id Message ID for the response
 * @param fib FIB number (0 for all)
 * @return Routes XML response string (allocated)
 */
char *create_routes_xml_response(netd_state_t *state, const char *message_id, uint32_t fib) {
  char *response;
  char *temp;
  int len, total_len;
  netd_route_t *route;
  char dest_str[64], gw_str[64];

  if (!state || !message_id) {
    return NULL;
  }

  /* Calculate total length needed */
  total_len = snprintf(NULL, 0,
                       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                       "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                       "message-id=\"%s\">\n"
                       "  <data>\n"
                       "    <routes xmlns=\"urn:ietf:params:xml:ns:yang:ietf-routing\">\n",
                       message_id);

  /* Add route entries */
  TAILQ_FOREACH(route, &state->routes, entries) {
    if (fib == 0 || route->fib == fib) {
      format_address(&route->destination, dest_str, sizeof(dest_str));
      format_address(&route->gateway, gw_str, sizeof(gw_str));
      
      len = snprintf(NULL, 0,
                     "      <route>\n"
                     "        <destination>%s</destination>\n"
                     "        <gateway>%s</gateway>\n"
                     "        <interface>%s</interface>\n"
                     "        <fib>%u</fib>\n"
                     "        <flags>0x%x</flags>\n"
                     "      </route>\n",
                     dest_str, gw_str, route->interface, route->fib, route->flags);
      total_len += len;
    }
  }

  total_len += snprintf(NULL, 0,
                        "    </routes>\n"
                        "  </data>\n"
                        "</rpc-reply>");

  /* Allocate and build response */
  response = malloc(total_len + 1);
  if (!response) {
    debug_log(DEBUG_ERROR, "Failed to allocate memory for routes response");
    return NULL;
  }

  len = snprintf(response, total_len + 1,
                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                 "message-id=\"%s\">\n"
                 "  <data>\n"
                 "    <routes xmlns=\"urn:ietf:params:xml:ns:yang:ietf-routing\">\n",
                 message_id);

  /* Add route entries */
  TAILQ_FOREACH(route, &state->routes, entries) {
    if (fib == 0 || route->fib == fib) {
      format_address(&route->destination, dest_str, sizeof(dest_str));
      format_address(&route->gateway, gw_str, sizeof(gw_str));
      
      temp = response + len;
      len += snprintf(temp, total_len + 1 - len,
                     "      <route>\n"
                     "        <destination>%s</destination>\n"
                     "        <gateway>%s</gateway>\n"
                     "        <interface>%s</interface>\n"
                     "        <fib>%u</fib>\n"
                     "        <flags>0x%x</flags>\n"
                     "      </route>\n",
                     dest_str, gw_str, route->interface, route->fib, route->flags);
    }
  }

  len += snprintf(response + len, total_len + 1 - len,
                  "    </routes>\n"
                  "  </data>\n"
                  "</rpc-reply>");

  return response;
} 