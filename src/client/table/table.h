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

#ifndef TABLE_H
#define TABLE_H

#include <net.h>

/* Error handling function */
void print_error(const char *format, ...);

/* Table utility functions */
void table_init(struct table_format *fmt, const char *title);
void table_add_column(struct table_format *fmt, const char *header, int min_width);
void table_update_width(struct table_format *fmt, int col_idx, int content_len);
void table_print_header(const struct table_format *fmt);
void table_print_row(const struct table_format *fmt, ...);
void table_print_row_multiline(const struct table_format *fmt, int num_lines, ...);
void table_print_footer(const struct table_format *fmt, const char *footer_text);
void print_separator(int width);
void format_interface_flags(int flags, char *flag_str, size_t max_len);

/* Interface table functions */
void print_interface_table(const char *xml_response);
void print_interface_footer(void);

/* Interface type specific table functions */
void print_bridge_table(const char *xml_response);
void print_vlan_table(const char *xml_response);
void print_ethernet_table(const char *xml_response);
void print_lagg_table(const char *xml_response);
void print_tap_table(const char *xml_response);
void print_gif_table(const char *xml_response);
void print_epair_table(const char *xml_response);
void print_vxlan_table(const char *xml_response);
void print_loopback_table(const char *xml_response);
void print_wlan_table(const char *xml_response);

/* General table functions */
void print_interface_table(const char *xml_response);
void print_interface_groups_table(const char *xml_response);
void print_vrf_table(const char *xml_response);
void print_routes_table(const char *xml_response);
int print_interface_groups_summary(const char *xml_response);

/* Bridge specific functions */
int parse_bridge_interfaces_from_xml(const char *xml, struct bridge_interface_data *interfaces, int max_interfaces);

/* LAGG specific functions */
int parse_lagg_interfaces_from_xml(const char *xml, struct lagg_interface_data *interfaces, int max_interfaces);

/* VXLAN specific functions */
int parse_vxlan_interfaces_from_xml(const char *xml, struct vxlan_interface_data *interfaces, int max_interfaces);

/* WiFi specific functions */
int parse_wifi_interfaces_from_xml(const char *xml, struct wifi_interface_data *interfaces, int max_interfaces);
int print_wifi_interface_groups_summary(const char *xml_response);

#endif /* TABLE_H */ 