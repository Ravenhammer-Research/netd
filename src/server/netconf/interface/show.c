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

#include "netd.h"
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * Find interface by name
 * @param state Server state
 * @param name Interface name
 * @return Interface structure or NULL if not found
 */
interface_t *interface_find(netd_state_t *state, const char *name) {
  interface_t *iface;

  if (!state || !name) {
    return NULL;
  }

  TAILQ_FOREACH(iface, &state->interfaces, entries) {
    if (strcmp(iface->name, name) == 0) {
      return iface;
    }
  }

  return NULL;
}

/**
 * List interfaces
 * @param state Server state
 * @param type Interface type filter (IF_TYPE_UNKNOWN for all)
 * @return 0 on success, -1 on failure
 */
int interface_list(netd_state_t *state, interface_type_t type) {
  interface_t *iface;
  int count = 0;

  if (!state) {
    debug_log(DEBUG_ERROR, "Invalid state parameter for interface listing");
    return -1;
  }

  debug_log(DEBUG_INFO, "Listing interfaces%s",
            type == IF_TYPE_UNKNOWN ? "" : " of specific type");

  TAILQ_FOREACH(iface, &state->interfaces, entries) {
    if (type == IF_TYPE_UNKNOWN || iface->type == type) {
      debug_log(DEBUG_INFO, "  %s (%s) fib=%u enabled=%s", iface->name,
                interface_type_to_string(iface->type), iface->fib,
                iface->enabled ? "yes" : "no");

      if (iface->group_count > 0) {
        debug_log(DEBUG_INFO, "    Groups:");
        for (int i = 0; i < iface->group_count; i++) {
          debug_log(DEBUG_INFO, "      %s", iface->groups[i]);
        }
      }
      count++;
    }
  }

  if (count == 0) {
    debug_log(DEBUG_INFO, "  No interfaces found");
  } else {
    debug_log(DEBUG_INFO, "Total interfaces: %d", count);
  }

  return 0;
}

/**
 * Enumerate system interfaces and populate internal state
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int interface_enumerate_system(netd_state_t *state) {
  if (!state) {
    debug_log(DEBUG_ERROR,
              "Invalid state parameter for system interface enumeration");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Starting system interface enumeration");

  /* Call the system-specific enumeration function */
  int result = freebsd_enumerate_interfaces(state);
  if (result == 0) {
    debug_log(DEBUG_DEBUG,
              "System interface enumeration completed successfully");
  } else {
    debug_log(DEBUG_ERROR, "System interface enumeration failed");
  }

  return result;
}

/**
 * Get all interfaces and return as XML response
 * @param state Server state
 * @param xml_response Pointer to store XML response string
 * @return 0 on success, -1 on failure
 */
char *interface_get_all(netd_state_t *state) {
  interface_t *iface;
  char *response = NULL;
  char *iface_xml = NULL;
  char *groups_xml = NULL;
  char *group_xml = NULL;
  char *temp_response = NULL;
  char *final_response = NULL;
  int interface_count = 0;
  
  if (!state) {
    debug_log(DEBUG_ERROR, "Invalid parameters for interface get all");
    return NULL;
  }

  debug_log(DEBUG_DEBUG, "Getting all interfaces for XML response");

  /* Start XML response */
  if (asprintf(&response, "<interfaces>") < 0) {
    debug_log(DEBUG_ERROR, "Failed to allocate memory for XML response");
    goto cleanup;
  }

  /* Add each interface to XML */
  TAILQ_FOREACH(iface, &state->interfaces, entries) {
    interface_count++;
    
    /* Generate XML for this interface */
    if (asprintf(&iface_xml, 
                 "<interface><name>%s</name><type>%s</type><fib>%u</fib><enabled>%s</enabled>",
                 iface->name,
                 interface_type_to_string(iface->type),
                 iface->fib,
                 iface->enabled ? "true" : "false") < 0) {
      debug_log(DEBUG_ERROR, "Failed to generate interface XML for %s", iface->name);
      goto cleanup;
    }

    /* Add groups if any */
    if (iface->group_count > 0) {
      if (asprintf(&groups_xml, "<groups>") < 0) {
        debug_log(DEBUG_ERROR, "Failed to allocate memory for groups XML");
        goto cleanup;
      }
      
      for (int i = 0; i < iface->group_count; i++) {
        if (asprintf(&group_xml, "<group>%s</group>", iface->groups[i]) < 0) {
          debug_log(DEBUG_ERROR, "Failed to generate group XML");
          goto cleanup;
        }
        
        char *new_groups_xml = NULL;
        if (asprintf(&new_groups_xml, "%s%s", groups_xml, group_xml) < 0) {
          debug_log(DEBUG_ERROR, "Failed to append group XML");
          free(new_groups_xml);
          goto cleanup;
        }
        
        free(groups_xml);
        free(group_xml);
        groups_xml = new_groups_xml;
      }
      
      char *groups_end = "</groups>";
      char *new_groups_xml = NULL;
      if (asprintf(&new_groups_xml, "%s%s", groups_xml, groups_end) < 0) {
        debug_log(DEBUG_ERROR, "Failed to close groups XML");
        free(new_groups_xml);
        goto cleanup;
      }
      
      free(groups_xml);
      groups_xml = new_groups_xml;
      
      /* Append groups to interface XML */
      char *new_iface_xml = NULL;
      if (asprintf(&new_iface_xml, "%s%s", iface_xml, groups_xml) < 0) {
        debug_log(DEBUG_ERROR, "Failed to append groups to interface XML");
        free(new_iface_xml);
        goto cleanup;
      }
      
      free(iface_xml);
      iface_xml = new_iface_xml;
    }

    /* Close interface tag */
    char *iface_end = "</interface>";
    char *new_iface_xml = NULL;
    if (asprintf(&new_iface_xml, "%s%s", iface_xml, iface_end) < 0) {
      debug_log(DEBUG_ERROR, "Failed to close interface XML");
      free(new_iface_xml);
      goto cleanup;
    }
    
    free(iface_xml);
    iface_xml = new_iface_xml;

    /* Append interface to response */
    if (asprintf(&temp_response, "%s%s", response, iface_xml) < 0) {
      debug_log(DEBUG_ERROR, "Failed to append interface to response");
      free(temp_response);
      goto cleanup;
    }
    
    free(response);
    free(iface_xml);
    response = temp_response;
    temp_response = NULL;
  }

  /* Close interfaces tag */
  char *end_tag = "</interfaces>";
  if (asprintf(&final_response, "%s%s", response, end_tag) < 0) {
    debug_log(DEBUG_ERROR, "Failed to close interfaces XML");
    goto cleanup;
  }
  
  debug_log(DEBUG_DEBUG, "Generated XML response for %d interfaces", interface_count);
  return final_response;

cleanup:
  free(response);
  free(iface_xml);
  free(groups_xml);
  free(group_xml);
  free(temp_response);
  free(final_response);
  return NULL;
} 