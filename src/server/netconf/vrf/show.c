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
#include <route.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  /* FIB 0 is the default VRF and always exists */
  if (fib_number == 0) {
    /* Return a special default VRF structure */
    static vrf_t default_vrf = {
      .fib_number = 0,
      .name = "default",
      .description = "Default VRF"
    };
    return &default_vrf;
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
    debug_log(ERROR, "Invalid state parameter for VRF listing");
    return -1;
  }

  debug_log(INFO, "Listing all VRFs");

  /* Always show default VRF */
  debug_log(INFO, "  default (FIB 0)");
  count++;

  TAILQ_FOREACH(vrf, &state->vrfs, entries) {
    debug_log(INFO, "  %s (FIB %u)", vrf->name, vrf->fib_number);
    if (vrf->description[0] != '\0') {
      debug_log(INFO, "    Description: %s", vrf->description);
    }
    count++;
  }

  debug_log(INFO, "Total VRFs: %d", count);
  return 0;
}

/**
 * Get all VRFs as XML for NETCONF response
 * @param state Server state
 * @return XML string (allocated) or NULL on failure
 */
char *vrf_get_all(netd_state_t *state) {
  vrf_t *vrf;
  char *xml = NULL;
  char *temp_xml = NULL;
  int vrf_count = 0;

  if (!state) {
    debug_log(ERROR, "Invalid state parameter for VRF XML generation");
    return NULL;
  }

  debug_log(DEBUG, "Generating XML for all VRFs");

  /* Include all VRFs that were created at startup */
  debug_log(DEBUG, "Generating XML for all created VRFs");

  /* Start XML */
  asprintf(&xml, "    <lib xmlns=\"http://frrouting.org/yang/vrf\">\n");
  if (!xml) {
    debug_log(ERROR, "Failed to allocate memory for VRF XML header");
    return NULL;
  }

  /* Always include default VRF (FIB 0) */
  debug_log(DEBUG, "Adding default VRF (FIB 0)");
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

  /* Include all created VRFs */
  debug_log(DEBUG, "Including all created VRFs...");
  TAILQ_FOREACH(vrf, &state->vrfs, entries) {
    vrf_count++;
    debug_log(DEBUG, "Found VRF %d: %s (FIB %u)", vrf_count,
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
      INFO,
      "Generated XML for %d VRFs (%zu bytes)",
      vrf_count, xml ? strlen(xml) : 0);
  return xml;
}

/**
 * Create VRFs XML response
 * @param state Server state
 * @param message_id Message ID for the response
 * @return VRFs XML response string (allocated)
 */
char *create_vrfs_xml_response(netd_state_t *state, const char *message_id) {
  char *response;
  char *vrfs_xml;

  if (!state || !message_id) {
    return NULL;
  }

  /* Get VRFs XML */
  vrfs_xml = vrf_get_all(state);
  if (!vrfs_xml) {
    debug_log(ERROR, "Failed to get VRFs XML");
    return NULL;
  }

  /* Allocate buffer for response */
  response = malloc(NETCONF_RESPONSE_BUFFER_SIZE);
  if (!response) {
    debug_log(ERROR, "Failed to allocate memory for VRFs response");
    free(vrfs_xml);
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
                       message_id, vrfs_xml) == -1) {
    free(vrfs_xml);
    free(response);
    return NULL;
  }

  free(vrfs_xml);
  debug_log(INFO, "Created VRFs XML response");
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
  char *routes_response;

  if (!state || !message_id || !vrf) {
    return NULL;
  }

  /* Get routes for this VRF using the existing route function */
  routes_response = create_routes_xml_response(state, message_id, vrf->fib_number);
  if (!routes_response) {
    return NULL;
  }

  /* For now, just return the routes response directly */
  /* TODO: Wrap in VRF structure if needed */
  return routes_response;
} 