/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef IF_TABLE_H
#define IF_TABLE_H

#include <stddef.h>

/* Forward declarations for functions from other modules */
extern int find_tag_content(const char *xml, const char *tag, char *content, size_t max_len);
extern void format_interface_flags(int flags, char *flag_str, size_t max_len);

/* Interface table column widths structure */
struct if_table_widths {
    int name_width;
    int status_width;
    int vrf_width;
    int tunvrf_width;
    int mtu_width;
    int flags_width;
    int ipv4_width;
    int ipv6_width;
    int groups_width;
};

/* Interface data structure */
struct interface_data {
    char name[64];
    char enabled[16];
    char fib[16];
    char mtu[16];
    char flags[16];
    char addr[64];
    char addr6[64];
    char addr_list[10][64];  /* All IPv4 addresses */
    char addr6_list[10][64]; /* All IPv6 addresses */
    int addr_count;          /* Number of IPv4 addresses */
    int addr6_count;         /* Number of IPv6 addresses */
    char groups[256];
    char bridge_members[256];
};

/* Function declarations */
int print_interface_table(const char *xml_response);
int print_interface_table_filtered(const char *xml_response, const char *group_name);

/* Helper functions */
void calculate_column_widths(const char *xml_response, struct if_table_widths *widths);
void extract_interface_data(const char *pos, struct interface_data *data);
void print_interface_row(const struct interface_data *data, const struct if_table_widths *widths);
void print_interface_header(const struct if_table_widths *widths);
void print_interface_footer(void);

/* XML parsing functions */
int parse_interfaces_from_xml(const char *xml, struct interface_data *interfaces, int max_interfaces);

#endif /* IF_TABLE_H */ 