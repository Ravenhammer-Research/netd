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

#include <net.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Print route table from XML response
 * @param xml_response XML response string
 */
void print_routes_table(const char *xml_response) {
  struct route_data routes[1000]; /* Max 1000 routes */
  struct table_format fmt;
  int count;

  if (!xml_response) {
    return;
  }

  /* Parse routes from XML using XML utilities */
  count = parse_routes_from_xml(xml_response, routes, 1000);
  if (count < 0) {
    print_error("Failed to parse route XML");
    return;
  }

  /* Initialize table format */
  table_init(&fmt, "Route Table");
  table_add_column(&fmt, "Destination", 11);
  table_add_column(&fmt, "Prefix", 6);
  table_add_column(&fmt, "Gateway", 7);
  table_add_column(&fmt, "Interface", 9);
  table_add_column(&fmt, "Scope", 5);
  table_add_column(&fmt, "Flags", 5);
  table_add_column(&fmt, "Expire", 6);

  /* First pass: calculate column widths */
  for (int i = 0; i < count; i++) {
    table_update_width(&fmt, 0, strlen(routes[i].destination));
    table_update_width(&fmt, 1, 3); /* prefix length is max 3 chars */
    table_update_width(&fmt, 2, strlen(routes[i].gateway));
    table_update_width(&fmt, 3, strlen(routes[i].interface));
    table_update_width(&fmt, 4, strlen(routes[i].scope_interface));
    table_update_width(&fmt, 5, 8); /* flags is max 8 chars */
    table_update_width(&fmt, 6, 8); /* expire is max 8 chars */
  }

  /* Print table header */
  table_print_header(&fmt);

  /* Print each route */
  for (int i = 0; i < count; i++) {
    char prefix_str[8], flags_str[16], expire_str[16];
    
    /* Format prefix length */
    if (routes[i].prefix_length > 0) {
      snprintf(prefix_str, sizeof(prefix_str), "%d", routes[i].prefix_length);
    } else {
      strlcpy(prefix_str, "-", sizeof(prefix_str));
    }
    
    /* Format flags */
    if (routes[i].flags != 0) {
      snprintf(flags_str, sizeof(flags_str), "0x%x", routes[i].flags);
    } else {
      strlcpy(flags_str, "-", sizeof(flags_str));
    }
    
    /* Format expire time */
    if (routes[i].expire > 0) {
      snprintf(expire_str, sizeof(expire_str), "%d", routes[i].expire);
    } else {
      strlcpy(expire_str, "-", sizeof(expire_str));
    }
    
    table_print_row(&fmt, routes[i].destination,
                    prefix_str,
                    routes[i].gateway[0] != '\0' ? routes[i].gateway : "-",
                    routes[i].interface[0] != '\0' ? routes[i].interface : "-",
                    routes[i].scope_interface[0] != '\0' ? routes[i].scope_interface : "-",
                    flags_str,
                    expire_str);
  }

  /* Print table footer */
  char footer_text[64];
  snprintf(footer_text, sizeof(footer_text), "Total routes: %d", count);
  table_print_footer(&fmt, footer_text);

  return;
} 