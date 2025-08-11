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

#include <net.h>

/* Structure to hold interface parsing context */
struct interface_parse_context {
    struct interface_data *interfaces;
    int max_interfaces;
    int interface_count;
    struct interface_data *current_interface;
    int in_interface;
    int in_group;
    int in_alias;
    char current_tag[64];
    char temp_content[256];
    int in_ipv4;
    int in_ipv6;
    char interface_type[32]; /* Store the interface type for later use */
  };
  
  /* Structure to hold bridge interface parsing context */
  struct bridge_parse_context {
    struct bridge_interface_data *interfaces;
    int max_interfaces;
    int interface_count;
    struct bridge_interface_data *current_interface;
    int in_interface;
    int in_bridge_members;
    char current_tag[64];
    char temp_content[256];
    int in_ipv4;
    int in_ipv6;
  };

/* Common XML utilities */
int find_tag_content(const char *xml, const char *tag_start, const char *tag_end,
                     char *result, int max_len);
int extract_xml_content(const char *xml, const char *tag, char *result,
                        int max_len);
int xml_contains_tag(const char *xml, const char *tag);
int count_xml_tags(const char *xml, const char *tag);
int validate_xml_structure(const char *xml);
int find_tag_content_expat(const char *xml, const char *tag, char *content,
                           size_t max_len);
char *extract_xml_content_simple(const char *xml, const char *tag, char *buffer,
                                 size_t max_len);
char *extract_xml_content_bounded(const char *xml, const char *end_boundary,
                                  const char *tag, char *buffer,
                                  size_t max_len);

/* Interface XML parsing */
int parse_interfaces_from_xml(const char *xml,
                              struct interface_data *interfaces,
                              int max_interfaces);

/* Bridge interface XML parsing */
int parse_bridge_interfaces_from_xml(const char *xml,
                                     struct bridge_interface_data *interfaces,
                                     int max_interfaces);

/* Route XML parsing */
int parse_routes_from_xml(const char *xml, struct route_data *routes,
                           int max_routes);

/* VRF XML parsing */
int parse_vrfs_from_xml(const char *xml, struct vrf_data *vrfs, int max_vrfs);

#endif /* XML_H */ 