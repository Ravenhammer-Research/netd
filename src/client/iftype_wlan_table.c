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
 * Parse wireless interface data from XML
 * @param xml XML string
 * @param interfaces Array to populate
 * @param max_interfaces Maximum number of interfaces
 * @return Number of interfaces parsed, -1 on error
 */
int parse_wlan_interfaces_from_xml(const char *xml,
                                   struct wlan_interface_data *interfaces,
                                   int max_interfaces) {
  debug_log(DEBUG_DEBUG, "Parsing WLAN interfaces from XML");

  if (!xml || !interfaces || max_interfaces <= 0) {
    return -1;
  }

  /* For now, we'll use the existing interface parsing and extend it */
  struct interface_data *base_interfaces =
      malloc(max_interfaces * sizeof(struct interface_data));
  if (!base_interfaces) {
    return -1;
  }

  int count = parse_interfaces_from_xml(xml, base_interfaces, max_interfaces);
  if (count < 0) {
    free(base_interfaces);
    return -1;
  }

  /* Convert to wlan_interface_data and extract wireless-specific info */
  for (int i = 0; i < count; i++) {
    /* Copy base data */
    memcpy(&interfaces[i].base, &base_interfaces[i],
           sizeof(struct interface_data));

    /* Initialize wireless-specific fields */
    strcpy(interfaces[i].ssid, "");
    strcpy(interfaces[i].channel, "");
    strcpy(interfaces[i].frequency, "");
    strcpy(interfaces[i].txpower, "");
    strcpy(interfaces[i].mode, "");
    strcpy(interfaces[i].security, "");
    strcpy(interfaces[i].signal_strength, "");
    strcpy(interfaces[i].noise, "");
    strcpy(interfaces[i].rate, "");

    /* TODO: Extract wireless-specific data from XML */
    /* This would require extending the XML parsing to handle wireless
     * attributes */
  }

  free(base_interfaces);
  debug_log(DEBUG_INFO, "Parsed %d WLAN interfaces from XML", count);
  return count;
}

/**
 * Print wireless interface groups summary
 * @param xml_response XML response string
 * @return 0 on success, -1 on failure
 */
int print_wlan_interface_groups_summary(const char *xml_response) {
  struct wlan_interface_data *interfaces = NULL;
  int interface_count = 0;
  int max_interfaces = 100;

  debug_log(DEBUG_INFO, "Printing WLAN interface groups summary");

  if (!xml_response) {
    print_error("XML response is NULL");
    return -1;
  }

  /* Allocate interface array */
  interfaces = malloc(max_interfaces * sizeof(struct wlan_interface_data));
  if (!interfaces) {
    print_error("Failed to allocate memory for interfaces");
    return -1;
  }

  /* Parse interfaces from XML */
  interface_count =
      parse_wlan_interfaces_from_xml(xml_response, interfaces, max_interfaces);
  if (interface_count < 0) {
    print_error("Failed to parse interfaces from XML");
    free(interfaces);
    return -1;
  }

  /* Create a hash table to count interfaces per group */
  struct wlan_group_count {
    char name[64];
    int count;
    char ssid[64];
    char channel[8];
    char mode[16];
  };

  struct wlan_group_count *groups =
      malloc(interface_count * sizeof(struct wlan_group_count));
  if (!groups) {
    print_error("Failed to allocate memory for groups");
    free(interfaces);
    return -1;
  }

  int group_count = 0;

  /* Count interfaces per group */
  for (int i = 0; i < interface_count; i++) {
    struct wlan_interface_data *data = &interfaces[i];

    if (data->base.groups[0] != '\0') {
      char *groups_copy = strdup(data->base.groups);
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
          strncpy(groups[group_count].ssid, data->ssid,
                  sizeof(groups[group_count].ssid) - 1);
          groups[group_count].ssid[sizeof(groups[group_count].ssid) - 1] = '\0';
          strncpy(groups[group_count].channel, data->channel,
                  sizeof(groups[group_count].channel) - 1);
          groups[group_count].channel[sizeof(groups[group_count].channel) - 1] =
              '\0';
          strncpy(groups[group_count].mode, data->mode,
                  sizeof(groups[group_count].mode) - 1);
          groups[group_count].mode[sizeof(groups[group_count].mode) - 1] = '\0';
          group_count++;
        }

        token = strtok(NULL, ",");
      }
      free(groups_copy);
    }
  }

  /* Print wireless group summary */
  printf("Wireless Interface Groups Summary:\n");
  printf("%-20s %-15s %-8s %-10s %s\n", "Group Name", "SSID", "Channel", "Mode",
         "Interface Count");
  printf("-------------------- --------------- -------- ---------- -----\n");

  for (int i = 0; i < group_count; i++) {
    printf("%-20s %-15s %-8s %-10s %d\n", groups[i].name, groups[i].ssid,
           groups[i].channel, groups[i].mode, groups[i].count);
  }

  printf("\nTotal wireless groups: %d\n", group_count);

  /* Cleanup */
  free(groups);
  free(interfaces);

  return 0;
}

/**
 * Print detailed wireless interface table
 * @param xml_response XML response string
 * @return 0 on success, -1 on failure
 */
void print_iftype_wlan_table(const char *xml_response) {
  struct table_format fmt;
  struct interface_data *interfaces = NULL;
  struct interface_data *filtered_interfaces = NULL;
  int max_interfaces = 100;
  int interface_count = 0;
  int filtered_count = 0;

  debug_log(DEBUG_INFO, "Printing wireless interface table");

  if (!xml_response) {
    print_error("XML response is NULL");
    return;
  }

  /* Allocate interface arrays */
  interfaces = malloc(max_interfaces * sizeof(struct interface_data));
  if (!interfaces) {
    print_error("Failed to allocate memory for interfaces");
    return;
  }

  filtered_interfaces = malloc(max_interfaces * sizeof(struct interface_data));
  if (!filtered_interfaces) {
    print_error("Failed to allocate memory for filtered interfaces");
    free(interfaces);
    return;
  }

  /* Parse interfaces from XML */
  interface_count =
      parse_interfaces_from_xml(xml_response, interfaces, max_interfaces);
  if (interface_count < 0) {
    print_error("Failed to parse interfaces from XML");
    free(interfaces);
    free(filtered_interfaces);
    return;
  }

  /* Filter for wireless interfaces */
  for (int i = 0; i < interface_count; i++) {
    struct interface_data *data = &interfaces[i];

    /* Check if this is a wireless interface */
    if (strncmp(data->name, "wlan", 4) == 0 ||
        strncmp(data->name, "ath", 3) == 0 ||
        strncmp(data->name, "iwn", 3) == 0 ||
        strncmp(data->name, "iwm", 3) == 0 ||
        strncmp(data->name, "iwl", 3) == 0 ||
        strncmp(data->name, "bwi", 3) == 0 ||
        strncmp(data->name, "rum", 3) == 0 ||
        strncmp(data->name, "run", 3) == 0 ||
        strncmp(data->name, "ural", 4) == 0 ||
        strncmp(data->name, "urtw", 4) == 0 ||
        strncmp(data->name, "zyd", 3) == 0) {

      memcpy(&filtered_interfaces[filtered_count], data,
             sizeof(struct interface_data));
      filtered_count++;
    }
  }

  /* Initialize table format for wireless interfaces */
  table_init(&fmt, "Wireless Interface Table");
  table_add_column(&fmt, "Name", 4);
  table_add_column(&fmt, "Status", 6);
  table_add_column(&fmt, "VRF", 3);
  table_add_column(&fmt, "Regdomain", 9);
  table_add_column(&fmt, "Country", 7);
  table_add_column(&fmt, "Auth", 4);
  table_add_column(&fmt, "Privacy", 7);
  table_add_column(&fmt, "TxPower", 7);
  table_add_column(&fmt, "Bmiss", 5);
  table_add_column(&fmt, "ScanValid", 9);
  table_add_column(&fmt, "Features", 8);
  table_add_column(&fmt, "Bintval", 7);
  table_add_column(&fmt, "Parent", 6);
  table_add_column(&fmt, "MTU", 3);
  table_add_column(&fmt, "Flags", 5);
  table_add_column(&fmt, "IPv4", 4);
  table_add_column(&fmt, "IPv6", 4);
  table_add_column(&fmt, "Groups", 6);

  /* First pass: calculate column widths */
  for (int i = 0; i < filtered_count; i++) {
    struct interface_data *data = &filtered_interfaces[i];

    /* Parse comma-separated addresses into arrays */
    data->addr_count = 0;
    data->addr6_count = 0;

    /* Parse IPv4 addresses */
    if (data->addr[0] != '\0') {
      char *addr_copy = strdup(data->addr);
      char *token = strtok(addr_copy, ",");
      while (token && data->addr_count < 10) {
        /* Trim whitespace */
        while (*token == ' ')
          token++;
        char *end = token + strlen(token) - 1;
        while (end > token && *end == ' ')
          end--;
        *(end + 1) = '\0';

        strncpy(data->addr_list[data->addr_count], token,
                sizeof(data->addr_list[0]) - 1);
        data->addr_list[data->addr_count][sizeof(data->addr_list[0]) - 1] =
            '\0';
        data->addr_count++;
        token = strtok(NULL, ",");
      }
      free(addr_copy);
    }

    /* Parse IPv6 addresses */
    if (data->addr6[0] != '\0') {
      char *addr6_copy = strdup(data->addr6);
      char *token = strtok(addr6_copy, ",");
      while (token && data->addr6_count < 10) {
        /* Trim whitespace */
        while (*token == ' ')
          token++;
        char *end = token + strlen(token) - 1;
        while (end > token && *end == ' ')
          end--;
        *(end + 1) = '\0';

        strncpy(data->addr6_list[data->addr6_count], token,
                sizeof(data->addr6_list[0]) - 1);
        data->addr6_list[data->addr6_count][sizeof(data->addr6_list[0]) - 1] =
            '\0';
        data->addr6_count++;
        token = strtok(NULL, ",");
      }
      free(addr6_copy);
    }

    table_update_width(&fmt, 0, strlen(data->name));
    table_update_width(
        &fmt, 1, strlen(strcmp(data->enabled, "true") == 0 ? "UP" : "DOWN"));
    table_update_width(&fmt, 2, strlen(data->fib));

    /* WiFi-specific width calculations */
    table_update_width(&fmt, 3, strlen(data->wifi_regdomain));
    table_update_width(&fmt, 4, strlen(data->wifi_country));
    table_update_width(&fmt, 5, strlen(data->wifi_authmode));
    table_update_width(&fmt, 6, strlen(data->wifi_privacy));
    char txpower_str[16];
    snprintf(txpower_str, sizeof(txpower_str), "%d", data->wifi_txpower);
    table_update_width(&fmt, 7, strlen(txpower_str));
    char bmiss_str[16];
    snprintf(bmiss_str, sizeof(bmiss_str), "%d", data->wifi_bmiss);
    table_update_width(&fmt, 8, strlen(bmiss_str));
    char scanvalid_str[16];
    snprintf(scanvalid_str, sizeof(scanvalid_str), "%d", data->wifi_scanvalid);
    table_update_width(&fmt, 9, strlen(scanvalid_str));
    table_update_width(&fmt, 10, strlen(data->wifi_features));
    char bintval_str[16];
    snprintf(bintval_str, sizeof(bintval_str), "%d", data->wifi_bintval);
    table_update_width(&fmt, 11, strlen(bintval_str));
    table_update_width(&fmt, 12, strlen(data->wifi_parent));
    table_update_width(&fmt, 13, strlen(data->mtu));

    /* Format flags */
    char flag_str[16] = "";
    int flag_val = atoi(data->flags);
    format_interface_flags(flag_val, flag_str, sizeof(flag_str));
    table_update_width(&fmt, 14, strlen(flag_str));

    /* Calculate max width for all IPv4 addresses */
    int max_ipv4_width = 0;
    for (int j = 0; j < data->addr_count; j++) {
      int len = strlen(data->addr_list[j]);
      if (len > max_ipv4_width)
        max_ipv4_width = len;
    }
    table_update_width(&fmt, 15, max_ipv4_width);

    /* Calculate max width for all IPv6 addresses */
    int max_ipv6_width = 0;
    for (int j = 0; j < data->addr6_count; j++) {
      int len = strlen(data->addr6_list[j]);
      if (len > max_ipv6_width)
        max_ipv6_width = len;
    }
    table_update_width(&fmt, 16, max_ipv6_width);

    /* Calculate width for groups */
    table_update_width(&fmt, 17, strlen(data->groups));
  }

  /* Print table header */
  table_print_header(&fmt);

  /* Print each wireless interface */
  for (int i = 0; i < filtered_count; i++) {
    struct interface_data *data = &filtered_interfaces[i];

    /* Format flags */
    char flag_str[16] = "";
    int flag_val = atoi(data->flags);
    format_interface_flags(flag_val, flag_str, sizeof(flag_str));

    /* Parse groups into array for multiline display */
    char group_array[10][64];
    int group_count = 0;
    if (data->groups[0] != '\0') {
      char *groups_copy = strdup(data->groups);
      char *token = strtok(groups_copy, ",");
      while (token && group_count < 10) {
        /* Trim whitespace */
        while (*token == ' ')
          token++;
        char *end = token + strlen(token) - 1;
        while (end > token && *end == ' ')
          end--;
        *(end + 1) = '\0';

        /* Skip "all" group - it's not a real group */
        if (strcmp(token, "all") != 0) {
          strncpy(group_array[group_count], token, sizeof(group_array[0]) - 1);
          group_array[group_count][sizeof(group_array[0]) - 1] = '\0';
          group_count++;
        }
        token = strtok(NULL, ",");
      }
      free(groups_copy);
    }

    /* Calculate number of lines needed for this interface */
    int max_lines = 1;
    if (data->addr_count > max_lines)
      max_lines = data->addr_count;
    if (data->addr6_count > max_lines)
      max_lines = data->addr6_count;
    if (group_count > max_lines)
      max_lines = group_count;

    /* Print each line */
    for (int line = 0; line < max_lines; line++) {
      const char *name = (line == 0) ? data->name : "";
      const char *status =
          (line == 0) ? (strcmp(data->enabled, "true") == 0 ? "UP" : "DOWN")
                      : "";
      const char *vrf = (line == 0) ? data->fib : "";

      /* WiFi-specific data */
      const char *regdomain = (line == 0) ? data->wifi_regdomain : "";
      const char *country = (line == 0) ? data->wifi_country : "";
      const char *authmode = (line == 0) ? data->wifi_authmode : "";
      const char *privacy = (line == 0) ? data->wifi_privacy : "";
      char txpower_str[16];
      snprintf(txpower_str, sizeof(txpower_str), "%d", data->wifi_txpower);
      const char *txpower = (line == 0) ? txpower_str : "";
      char bmiss_str[16];
      snprintf(bmiss_str, sizeof(bmiss_str), "%d", data->wifi_bmiss);
      const char *bmiss = (line == 0) ? bmiss_str : "";
      char scanvalid_str[16];
      snprintf(scanvalid_str, sizeof(scanvalid_str), "%d",
               data->wifi_scanvalid);
      const char *scanvalid = (line == 0) ? scanvalid_str : "";
      const char *features = (line == 0) ? data->wifi_features : "";
      char bintval_str[16];
      snprintf(bintval_str, sizeof(bintval_str), "%d", data->wifi_bintval);
      const char *bintval = (line == 0) ? bintval_str : "";
      const char *parent = (line == 0) ? data->wifi_parent : "";

      const char *mtu = (line == 0) ? data->mtu : "";
      const char *flags = (line == 0) ? flag_str : "";
      const char *ipv4 = (line < data->addr_count) ? data->addr_list[line] : "";
      const char *ipv6 =
          (line < data->addr6_count) ? data->addr6_list[line] : "";
      const char *groups = (line < group_count) ? group_array[line] : "";

      table_print_row(&fmt, name, status, vrf, regdomain, country, authmode,
                      privacy, txpower, bmiss, scanvalid, features, bintval,
                      parent, mtu, flags, ipv4, ipv6, groups);
    }
  }

  /* Print table footer */
  char footer_text[64];
  snprintf(footer_text, sizeof(footer_text), "Total wireless interfaces: %d",
           filtered_count);
  table_print_footer(&fmt, footer_text);

  free(interfaces);
  free(filtered_interfaces);
}