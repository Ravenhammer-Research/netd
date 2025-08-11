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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netconf/netconf.h>



/**
 * Create interfaces XML response
 * @param state Server state
 * @param message_id Message ID for the response
 * @return Interfaces XML response string (allocated)
 */
char *create_interfaces_xml_response(netd_state_t *state, const char *message_id) {
  char *response;
  int len;
  interface_t *iface;

  if (!state || !message_id) {
    return NULL;
  }

  /* Allocate buffer for response */
  response = malloc(NETCONF_RESPONSE_BUFFER_SIZE);
  if (!response) {
    debug_log(ERROR, "Failed to allocate memory for interfaces response");
    return NULL;
  }

  len = prepare_response(response, 0,
                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                 "message-id=\"%s\">\n"
                 "  <data>\n"
                 "    <interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">\n",
                 message_id);

  /* Add interface entries */
  int interface_count = 0;
  TAILQ_FOREACH(iface, &state->interfaces, entries) {
    interface_count++;
    debug_log(DEBUG, "Processing interface %d: %s", interface_count, iface->name);
    /* Build groups string */
    char groups_str[256] = "";
    if (iface->group_count > 0) {
      for (int i = 0; i < iface->group_count; i++) {
        if (i > 0) strcat(groups_str, ",");
        strcat(groups_str, iface->groups[i]);
      }
    }
    
    /* Find VRF name for this FIB */
    char vrf_name[64] = "";
    if (iface->fib > 0) {
      vrf_t *vrf = vrf_find_by_fib(state, iface->fib);
      if (vrf) {
        snprintf(vrf_name, sizeof(vrf_name), "%s", vrf->name);
      } else {
        snprintf(vrf_name, sizeof(vrf_name), "fib%d", iface->fib);
      }
    } else {
      strcpy(vrf_name, "default");
    }
    
    len = prepare_response(response, len,
                   "      <interface>\n"
                   "        <name>%s</name>\n"
                   "        <type xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\">"
                   "ianaift:%s</type>\n"
                   "        <enabled>%s</enabled>\n"
                   "        <bind-ni-name>%s</bind-ni-name>\n"
                   "        <mtu>%d</mtu>\n"
                   "        <flags>%d</flags>\n",
                   iface->name,
                   interface_type_to_string(iface->type),
                   iface->enabled ? "true" : "false",
                   vrf_name,
                   iface->mtu,
                   iface->flags);

    /* Add IPv4 addresses */
    if (iface->address_count > 0) {
      len = prepare_response(response, len,
                     "        <ipv4 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">\n");
      for (int i = 0; i < iface->address_count; i++) {
        len = prepare_response(response, len,
                       "          <address>\n"
                       "            <ip>%s</ip>\n"
                       "            <prefix-length>%d</prefix-length>\n"
                       "          </address>\n",
                       iface->addresses[i].addr,
                       iface->addresses[i].prefixlen);
      }
      len = prepare_response(response, len, "        </ipv4>\n");
    }

    /* Add IPv6 addresses */
    if (iface->address_count6 > 0) {
      len = prepare_response(response, len,
                     "        <ipv6 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">\n");
      for (int i = 0; i < iface->address_count6; i++) {
        len = prepare_response(response, len,
                       "          <address>\n"
                       "            <ip>%s</ip>\n"
                       "            <prefix-length>%d</prefix-length>\n"
                       "          </address>\n",
                       iface->addresses6[i].addr,
                       iface->addresses6[i].prefixlen);
      }
      len = prepare_response(response, len, "        </ipv6>\n");
    }

    len = prepare_response(response, len,
                   "        <groups>%s</groups>\n"
                   "      </interface>\n",
                   groups_str);
    debug_log(DEBUG, "Completed interface %d: %s", interface_count, iface->name);
  }

    len = prepare_response(response, len,
                 "    </interfaces>\n"
                 "  </data>\n"
                 "</rpc-reply>");

  debug_log(DEBUG, "Generated interfaces XML response: %d bytes", len);
  return response;
} 