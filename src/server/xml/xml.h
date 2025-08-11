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

#ifndef XML_H
#define XML_H

#include <stdbool.h>
#include <stdint.h>
#include <netd.h>

/* Maximum number of XML elements to track during parsing.
 * Set to 8192 to handle complex nested XML structures without practical limits */
#define MAX_XML_ELEMENTS 8192

/* Edit config data structure */
struct edit_config_data {
  bool is_edit_config;
  bool in_config;
  bool in_vrf;
  bool in_route;
  bool in_set;
  bool has_route_config;
  bool has_vrf_config;
  bool has_set_config;
  char vrf_name[64];
  char vrf_table[16];
  char route_destination[64];
  char route_gateway[64];
  char route_interface[64];
  char route_type[32];
  bool route_enabled;
  char set_object[32];
  char set_type[32];
  char set_name[64];
  char set_property[32];
  char set_value[64];
  char set_sub_property[32];
  char set_sub_value[64];
  char current_tag[64];
  char temp_content[256];
};

/* General XML parsing utilities */
char *extract_message_id(const char *request);
bool xml_contains_elements(const char *request, const char **elements, int element_count);

/* VRF/FIB specific XML parsing utility */
char *extract_vrf_name_from_request(const char *request);

/* Edit-config XML parsing utilities */
bool is_edit_config_request(const char *request);
int process_edit_config_request(netd_state_t *state, const char *request);

#endif /* XML_H */ 