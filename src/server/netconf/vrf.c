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
    debug_log(DEBUG_ERROR,
              "Invalid parameters for VRF creation: state=%p, name=%s, fib=%u",
              state, name ? name : "NULL", fib_number);
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Creating VRF '%s' with FIB %u", name, fib_number);

  /* Check if VRF already exists */
  if (vrf_find_by_name(state, name)) {
    debug_log(DEBUG_ERROR, "VRF %s already exists in state", name);
    return -1;
  }

  if (vrf_find_by_fib(state, fib_number)) {
    debug_log(DEBUG_ERROR, "FIB %u already assigned to another VRF",
              fib_number);
    return -1;
  }

  /* Allocate new VRF */
  vrf = malloc(sizeof(*vrf));
  if (!vrf) {
    debug_log(DEBUG_ERROR, "Failed to allocate memory for VRF %s", name);
    return -1;
  }

  /* Initialize VRF */
  memset(vrf, 0, sizeof(*vrf));
  strlcpy(vrf->name, name, sizeof(vrf->name));
  vrf->fib_number = fib_number;

  /* Add to VRF list */
  TAILQ_INSERT_TAIL(&state->vrfs, vrf, entries);
  debug_log(DEBUG_DEBUG, "Added VRF %s to state list", name);

  debug_log(DEBUG_INFO, "Created VRF %s with FIB %u", name, fib_number);
  return 0;
}

/**
 * Create VRFs XML response
 * @param state Server state
 * @param message_id Message ID for the response
 * @return VRFs XML response string (allocated)
 */
char *create_vrfs_xml_response(netd_state_t *state, const char *message_id) {
  char *response;
  char *temp;
  int len, total_len;
  vrf_t *vrf;

  if (!state || !message_id) {
    return NULL;
  }

  /* Calculate total length needed */
  total_len = snprintf(NULL, 0,
                       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                       "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                       "message-id=\"%s\">\n"
                       "  <data>\n"
                       "    <vrfs xmlns=\"urn:ietf:params:xml:ns:yang:frr-vrf\">\n",
                       message_id);

  /* Add VRF entries */
  TAILQ_FOREACH(vrf, &state->vrfs, entries) {
    len = snprintf(NULL, 0,
                   "      <vrf>\n"
                   "        <name>%s</name>\n"
                   "        <fib>%u</fib>\n"
                   "        <description>%s</description>\n"
                   "      </vrf>\n",
                   vrf->name, vrf->fib_number, vrf->description);
    total_len += len;
  }

  total_len += snprintf(NULL, 0,
                        "    </vrfs>\n"
                        "  </data>\n"
                        "</rpc-reply>");

  /* Allocate and build response */
  response = malloc(total_len + 1);
  if (!response) {
    debug_log(DEBUG_ERROR, "Failed to allocate memory for VRFs response");
    return NULL;
  }

  len = snprintf(response, total_len + 1,
                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                 "message-id=\"%s\">\n"
                 "  <data>\n"
                 "    <vrfs xmlns=\"urn:ietf:params:xml:ns:yang:frr-vrf\">\n",
                 message_id);

  /* Add VRF entries */
  TAILQ_FOREACH(vrf, &state->vrfs, entries) {
    temp = response + len;
    len += snprintf(temp, total_len + 1 - len,
                   "      <vrf>\n"
                   "        <name>%s</name>\n"
                   "        <fib>%u</fib>\n"
                   "        <description>%s</description>\n"
                   "      </vrf>\n",
                   vrf->name, vrf->fib_number, vrf->description);
  }

  len += snprintf(response + len, total_len + 1 - len,
                  "    </vrfs>\n"
                  "  </data>\n"
                  "</rpc-reply>");

  return response;
}

/**
 * Create VRF routes XML response
 * @param state Server state
 * @param message_id Message ID for the response
 * @param vrf VRF structure
 * @return VRF routes XML response string (allocated)
 */
char *create_vrf_routes_xml_response(netd_state_t *state, const char *message_id, vrf_t *vrf) {
  char *response;
  char *temp;
  int len, total_len;
  netd_route_t *route;
  char dest_str[64], gw_str[64];

  if (!state || !message_id || !vrf) {
    return NULL;
  }

  /* Calculate total length needed */
  total_len = snprintf(NULL, 0,
                       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                       "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                       "message-id=\"%s\">\n"
                       "  <data>\n"
                       "    <vrf-routes xmlns=\"urn:ietf:params:xml:ns:yang:frr-vrf\">\n"
                       "      <vrf>\n"
                       "        <name>%s</name>\n"
                       "        <fib>%u</fib>\n"
                       "        <routes>\n",
                       message_id, vrf->name, vrf->fib_number);

  /* Add route entries for this VRF */
  TAILQ_FOREACH(route, &state->routes, entries) {
    if (route->fib == vrf->fib_number) {
      format_address(&route->destination, dest_str, sizeof(dest_str));
      format_address(&route->gateway, gw_str, sizeof(gw_str));
      
      len = snprintf(NULL, 0,
                     "          <route>\n"
                     "            <destination>%s</destination>\n"
                     "            <gateway>%s</gateway>\n"
                     "            <interface>%s</interface>\n"
                     "            <flags>0x%x</flags>\n"
                     "          </route>\n",
                     dest_str, gw_str, route->interface, route->flags);
      total_len += len;
    }
  }

  total_len += snprintf(NULL, 0,
                        "        </routes>\n"
                        "      </vrf>\n"
                        "    </vrf-routes>\n"
                        "  </data>\n"
                        "</rpc-reply>");

  /* Allocate and build response */
  response = malloc(total_len + 1);
  if (!response) {
    debug_log(DEBUG_ERROR, "Failed to allocate memory for VRF routes response");
    return NULL;
  }

  len = snprintf(response, total_len + 1,
                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                 "message-id=\"%s\">\n"
                 "  <data>\n"
                 "    <vrf-routes xmlns=\"urn:ietf:params:xml:ns:yang:frr-vrf\">\n"
                 "      <vrf>\n"
                 "        <name>%s</name>\n"
                 "        <fib>%u</fib>\n"
                         "        <routes>\n",
                 message_id, vrf->name, vrf->fib_number);

  /* Add route entries for this VRF */
  TAILQ_FOREACH(route, &state->routes, entries) {
    if (route->fib == vrf->fib_number) {
      format_address(&route->destination, dest_str, sizeof(dest_str));
      format_address(&route->gateway, gw_str, sizeof(gw_str));
      
      temp = response + len;
      len += snprintf(temp, total_len + 1 - len,
                     "          <route>\n"
                     "            <destination>%s</destination>\n"
                     "            <gateway>%s</gateway>\n"
                     "            <interface>%s</interface>\n"
                     "            <flags>0x%x</flags>\n"
                     "          </route>\n",
                     dest_str, gw_str, route->interface, route->flags);
    }
  }

  len += snprintf(response + len, total_len + 1 - len,
                  "        </routes>\n"
                  "      </vrf>\n"
                  "    </vrf-routes>\n"
                  "  </data>\n"
                  "</rpc-reply>");

  return response;
}

/**
 * Delete a VRF
 * @param state Server state
 * @param name VRF name
 * @return 0 on success, -1 on failure
 */
int vrf_delete(netd_state_t *state, const char *name) {
  vrf_t *vrf;
  interface_t *iface;

  if (!state || !name) {
    debug_log(DEBUG_ERROR,
              "Invalid parameters for VRF deletion: state=%p, name=%s", state,
              name ? name : "NULL");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Deleting VRF '%s'", name);

  /* Cannot delete default VRF */
  if (strcmp(name, "default") == 0) {
    debug_log(DEBUG_ERROR, "Cannot delete default VRF");
    return -1;
  }

  /* Find VRF */
  vrf = vrf_find_by_name(state, name);
  if (!vrf) {
    debug_log(DEBUG_ERROR, "VRF %s not found in state", name);
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Found VRF %s in state, processing interfaces", name);

  /* Remove all interfaces from this VRF */
  int moved_interfaces = 0;
  TAILQ_FOREACH(iface, &state->interfaces, entries) {
    if (iface->fib == vrf->fib_number) {
      iface->fib = 0; /* Move to default VRF */
      moved_interfaces++;
      debug_log(DEBUG_DEBUG, "Moved interface %s to default VRF", iface->name);
    }
  }
  debug_log(DEBUG_DEBUG, "Moved %d interfaces from VRF %s to default VRF",
            moved_interfaces, name);

  /* Flush all routes for this FIB */
  debug_log(DEBUG_DEBUG, "Flushing routes for FIB %u", vrf->fib_number);
  route_flush_fib(state, vrf->fib_number);

  /* Remove from VRF list */
  TAILQ_REMOVE(&state->vrfs, vrf, entries);
  free(vrf);
  debug_log(DEBUG_DEBUG, "Removed VRF %s from state list", name);

  debug_log(DEBUG_INFO, "Deleted VRF %s (moved %d interfaces to default VRF)",
            name, moved_interfaces);
  return 0;
}

/**
 * Find VRF by name
 * @param state Server state
 * @param name VRF name
 * @return VRF structure or NULL if not found
 */
vrf_t *vrf_find_by_name(netd_state_t *state, const char *name) {
  vrf_t *vrf;

  if (!state || !name) {
    return NULL;
  }

  TAILQ_FOREACH(vrf, &state->vrfs, entries) {
    if (strcmp(vrf->name, name) == 0) {
      return vrf;
    }
  }

  return NULL;
}

/**
 * Find VRF by FIB number
 * @param state Server state
 * @param fib_number FIB number
 * @return VRF structure or NULL if not found
 */
vrf_t *vrf_find_by_fib(netd_state_t *state, uint32_t fib_number) {
  vrf_t *vrf;

  if (!state) {
    return NULL;
  }

  TAILQ_FOREACH(vrf, &state->vrfs, entries) {
    if (vrf->fib_number == fib_number) {
      return vrf;
    }
  }

  return NULL;
}

/**
 * List all VRFs
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int vrf_list(netd_state_t *state) {
  vrf_t *vrf;
  int count = 0;

  if (!state) {
    debug_log(DEBUG_ERROR, "Invalid state parameter for VRF listing");
    return -1;
  }

  debug_log(DEBUG_INFO, "Listing all VRFs");

  /* Always show default VRF */
  debug_log(DEBUG_INFO, "  default (FIB 0)");
  count++;

  TAILQ_FOREACH(vrf, &state->vrfs, entries) {
    debug_log(DEBUG_INFO, "  %s (FIB %u)", vrf->name, vrf->fib_number);
    if (vrf->description[0] != '\0') {
      debug_log(DEBUG_INFO, "    Description: %s", vrf->description);
    }
    count++;
  }

  debug_log(DEBUG_INFO, "Total VRFs: %d", count);
  return 0;
}

/**
 * Get all VRFs as XML for NETCONF response
 * @param state Server state
 * @return XML string (allocated) or NULL on failure
 */
char *vrf_get_all(netd_state_t *state) {
  vrf_t *vrf;
  interface_t *iface;
  char *xml = NULL;
  char *temp_xml = NULL;
  bool fib_used[256] = {false}; // Track which FIBs are in use
  int vrf_count = 0;

  if (!state) {
    debug_log(DEBUG_ERROR, "Invalid state parameter for VRF XML generation");
    return NULL;
  }

  debug_log(DEBUG_DEBUG, "Generating XML for all VRFs");

  /* Scan all interfaces to find FIBs in use */
  debug_log(DEBUG_DEBUG, "Scanning interfaces for FIBs...");
  int interface_count = 0;
  TAILQ_FOREACH(iface, &state->interfaces, entries) {
    interface_count++;
    debug_log(DEBUG_TRACE, "Interface %d: %s has FIB %u", interface_count,
              iface->name, iface->fib);
    if (iface->fib < 256) {
      fib_used[iface->fib] = true;
    }
  }
  debug_log(DEBUG_DEBUG, "Scanned %d interfaces for FIB usage",
            interface_count);

  /* Start XML */
  asprintf(&xml, "    <lib xmlns=\"http://frrouting.org/yang/vrf\">\n");
  if (!xml) {
    debug_log(DEBUG_ERROR, "Failed to allocate memory for VRF XML header");
    return NULL;
  }

  /* Always include default VRF (FIB 0) */
  debug_log(DEBUG_DEBUG, "Adding default VRF (FIB 0)");
  asprintf(&temp_xml, "      <vrf>\n"
                      "        <name>default</name>\n"
                      "        <state>\n"
                      "          <id>0</id>\n"
                      "          <active>true</active>\n"
                      "        </state>\n"
                      "      </vrf>\n");
  if (temp_xml) {
    char *new_xml;
    asprintf(&new_xml, "%s%s", xml, temp_xml);
    free(xml);
    free(temp_xml);
    xml = new_xml;
  }
  vrf_count++;

  /* Include explicitly created VRFs */
  debug_log(DEBUG_DEBUG, "Checking for explicitly created VRFs...");
  TAILQ_FOREACH(vrf, &state->vrfs, entries) {
    vrf_count++;
    debug_log(DEBUG_DEBUG, "Found explicit VRF %d: %s (FIB %u)", vrf_count,
              vrf->name, vrf->fib_number);
    asprintf(&temp_xml,
             "      <vrf>\n"
             "        <name>%s</name>\n"
             "        <state>\n"
             "          <id>%u</id>\n"
             "          <active>true</active>\n"
             "        </state>\n"
             "      </vrf>\n",
             vrf->name, vrf->fib_number);

    if (temp_xml) {
      char *new_xml;
      asprintf(&new_xml, "%s%s", xml, temp_xml);
      free(xml);
      free(temp_xml);
      xml = new_xml;
    }
  }

  /* Include FIBs that are in use but don't have explicit VRF names */
  debug_log(DEBUG_DEBUG, "Checking for auto-detected FIBs...");
  int auto_vrf_count = 0;
  for (int i = 1; i < 256; i++) { // Skip FIB 0 (default)
    if (fib_used[i] && !vrf_find_by_fib(state, i)) {
      auto_vrf_count++;
      vrf_count++;
      debug_log(DEBUG_TRACE, "Adding auto-detected VRF %d: %d (FIB %d)",
                auto_vrf_count, i, i);
      asprintf(&temp_xml,
               "      <vrf>\n"
               "        <name>%d</name>\n"
               "        <state>\n"
               "          <id>%d</id>\n"
               "          <active>true</active>\n"
               "        </state>\n"
               "      </vrf>\n",
               i, i);

      if (temp_xml) {
        char *new_xml;
        asprintf(&new_xml, "%s%s", xml, temp_xml);
        free(xml);
        free(temp_xml);
        xml = new_xml;
      }
    }
  }
  debug_log(DEBUG_DEBUG, "Added %d auto-detected VRFs", auto_vrf_count);

  /* Close XML tags */
  asprintf(&temp_xml, "    </lib>\n");
  if (temp_xml) {
    char *new_xml;
    asprintf(&new_xml, "%s%s", xml, temp_xml);
    free(xml);
    free(temp_xml);
    xml = new_xml;
  }

  debug_log(
      DEBUG_INFO,
      "Generated XML for %d VRFs (%zu bytes): %d explicit, %d auto-detected",
      vrf_count, xml ? strlen(xml) : 0, vrf_count - auto_vrf_count - 1,
      auto_vrf_count);
  return xml;
}