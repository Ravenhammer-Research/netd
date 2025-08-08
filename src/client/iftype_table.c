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

#include "net.h"
#include <bsdxml.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Print interface groups summary from XML response
 * @param xml_response XML response string
 * @return 0 on success, -1 on failure
 */
int print_interface_groups_summary(const char *xml_response) {
  struct interface_data *interfaces = NULL;
  int interface_count = 0;
  int max_interfaces = 100;

  debug_log(DEBUG_INFO, "Printing interface groups summary");

  if (!xml_response) {
    print_error("XML response is NULL");
    return -1;
  }

  /* Allocate interface array */
  interfaces = malloc(max_interfaces * sizeof(struct interface_data));
  if (!interfaces) {
    print_error("Failed to allocate memory for interfaces");
    return -1;
  }

  /* Parse interfaces from XML */
  debug_log(DEBUG_DEBUG, "Parsing interfaces from XML for groups summary");
  interface_count =
      parse_interfaces_from_xml(xml_response, interfaces, max_interfaces);
  if (interface_count < 0) {
    print_error("Failed to parse interfaces from XML");
    free(interfaces);
    return -1;
  }
  debug_log(DEBUG_INFO, "Parsed %d interfaces from XML", interface_count);

  /* Create a hash table to count interfaces per group */
  struct group_count {
    char name[64];
    int count;
  };

  struct group_count *groups =
      malloc(interface_count * sizeof(struct group_count));
  if (!groups) {
    print_error("Failed to allocate memory for groups");
    free(interfaces);
    return -1;
  }

  int group_count = 0;

  /* Count interfaces per group */
  for (int i = 0; i < interface_count; i++) {
    struct interface_data *data = &interfaces[i];

    if (data->groups[0] != '\0') {
      char *groups_copy = strdup(data->groups);
      char *token = strtok(groups_copy, ",");

      while (token) {
        /* Trim whitespace */
        while (*token == ' ')
          token++;
        char *end = token + strlen(token) - 1;
        while (end > token && *end == ' ')
          end--;
        *(end + 1) = '\0';

        /* Find or add group */
        int found = -1;
        for (int j = 0; j < group_count; j++) {
          if (strcmp(groups[j].name, token) == 0) {
            found = j;
            break;
          }
        }

        if (found >= 0) {
          groups[found].count++;
        } else {
          strncpy(groups[group_count].name, token,
                  sizeof(groups[group_count].name) - 1);
          groups[group_count].name[sizeof(groups[group_count].name) - 1] = '\0';
          groups[group_count].count = 1;
          group_count++;
        }

        token = strtok(NULL, ",");
      }
      free(groups_copy);
    }
  }

  /* Print group summary */
  printf("Interface Groups Summary:\n");
  printf("%-20s %s\n", "Group Name", "Interface Count");
  printf("-------------------- -----\n");

  for (int i = 0; i < group_count; i++) {
    printf("%-20s %d\n", groups[i].name, groups[i].count);
  }

  printf("\nTotal groups: %d\n", group_count);

  /* Cleanup */
  free(groups);
  free(interfaces);

  return 0;
}