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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Print VRF table from XML response
 * @param xml_response XML response string
 */
void print_vrf_table(const char *xml_response) {
  struct vrf_data vrfs[100]; /* Max 100 VRFs */
  struct table_format fmt;
  int count;

  if (!xml_response) {
    return;
  }

  /* Parse VRFs from XML using XML utilities */
  count = parse_vrfs_from_xml(xml_response, vrfs, 100);
  if (count < 0) {
    print_error("Failed to parse VRF XML");
    return;
  }

  /* Initialize table format */
  table_init(&fmt, "VRF Table");
  table_add_column(&fmt, "Name", 4);
  table_add_column(&fmt, "FIB", 3);
  table_add_column(&fmt, "Description", 11);

  /* First pass: calculate column widths */
  for (int i = 0; i < count; i++) {
    table_update_width(&fmt, 0, strlen(vrfs[i].name));
    table_update_width(&fmt, 1, snprintf(NULL, 0, "%d", vrfs[i].fib));
    table_update_width(&fmt, 2, strlen(vrfs[i].description));
  }

  /* Print table header */
  table_print_header(&fmt);

  /* Print each VRF */
  for (int i = 0; i < count; i++) {
    char fib_str[16];
    snprintf(fib_str, sizeof(fib_str), "%d", vrfs[i].fib);

    table_print_row(&fmt, vrfs[i].name, fib_str,
                    vrfs[i].description[0] != '\0' ? vrfs[i].description : "");
  }

  /* Print table footer */
  char footer_text[64];
  snprintf(footer_text, sizeof(footer_text), "Total VRFs: %d", count);
  table_print_footer(&fmt, footer_text);

  return;
}