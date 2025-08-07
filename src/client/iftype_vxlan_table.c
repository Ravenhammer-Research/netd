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

#include "net.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Print VXLAN interface table from XML response
 * @param xml_response XML response string
 */
void print_iftype_vxlan_table(const char *xml_response)
{
    struct table_format fmt;
    struct interface_data *interfaces = NULL;
    int max_interfaces = 100;
    int interface_count = 0;
    
    debug_log(DEBUG_INFO, "Printing VXLAN interface table");
    
    if (!xml_response) {
        print_error("XML response is NULL");
        return;
    }
    
    /* Allocate interface array */
    interfaces = malloc(max_interfaces * sizeof(struct interface_data));
    if (!interfaces) {
        print_error("Failed to allocate memory for interfaces");
        return;
    }
    
    /* Parse all interfaces from XML */
    debug_log(DEBUG_DEBUG, "Parsing interfaces from XML for VXLAN table");
    interface_count = parse_interfaces_from_xml(xml_response, interfaces, max_interfaces);
    if (interface_count < 0) {
        free(interfaces);
        print_error("Failed to parse XML response");
        return;
    }
    debug_log(DEBUG_INFO, "Parsed %d interfaces from XML", interface_count);
    
    /* Filter interfaces by VXLAN type (check if name starts with vxlan) */
    debug_log(DEBUG_DEBUG, "Filtering interfaces by VXLAN type");
    int filtered_count = 0;
    for (int i = 0; i < interface_count; i++) {
        struct interface_data *data = &interfaces[i];
        
        /* Check if interface is a VXLAN interface by looking for vxlan in name */
        if (strncmp(data->name, "vxlan", 5) == 0) {
            /* Move this interface to the front of the array */
            if (filtered_count != i) {
                interfaces[filtered_count] = interfaces[i];
            }
            filtered_count++;
        }
    }
    
    if (filtered_count == 0) {
        printf("No VXLAN interfaces found.\n");
        free(interfaces);
        return;
    }
    
    /* Initialize table format for VXLAN interfaces */
    table_init(&fmt, "VXLAN Interface Table");
    table_add_column(&fmt, "Name", 4);
    table_add_column(&fmt, "Status", 6);
    table_add_column(&fmt, "VRF", 3);
    table_add_column(&fmt, "MTU", 3);
    table_add_column(&fmt, "Flags", 5);
    table_add_column(&fmt, "IPv4", 4);
    table_add_column(&fmt, "IPv6", 4);
    table_add_column(&fmt, "Groups", 6);
    
    /* First pass: calculate column widths */
    for (int i = 0; i < filtered_count; i++) {
        struct interface_data *data = &interfaces[i];
        
        /* Parse comma-separated addresses into arrays */
        data->addr_count = 0;
        data->addr6_count = 0;
        
        /* Parse IPv4 addresses */
        if (data->addr[0] != '\0') {
            char *addr_copy = strdup(data->addr);
            char *token = strtok(addr_copy, ",");
            while (token && data->addr_count < 10) {
                /* Trim whitespace */
                while (*token == ' ') token++;
                char *end = token + strlen(token) - 1;
                while (end > token && *end == ' ') end--;
                *(end + 1) = '\0';
                
                strncpy(data->addr_list[data->addr_count], token, sizeof(data->addr_list[0]) - 1);
                data->addr_list[data->addr_count][sizeof(data->addr_list[0]) - 1] = '\0';
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
                while (*token == ' ') token++;
                char *end = token + strlen(token) - 1;
                while (end > token && *end == ' ') end--;
                *(end + 1) = '\0';
                
                strncpy(data->addr6_list[data->addr6_count], token, sizeof(data->addr6_list[0]) - 1);
                data->addr6_list[data->addr6_count][sizeof(data->addr6_list[0]) - 1] = '\0';
                data->addr6_count++;
                token = strtok(NULL, ",");
            }
            free(addr6_copy);
        }
        
        table_update_width(&fmt, 0, strlen(data->name));
        table_update_width(&fmt, 1, strlen(strcmp(data->enabled, "true") == 0 ? "UP" : "DOWN"));
        table_update_width(&fmt, 2, strlen(data->fib));
        table_update_width(&fmt, 3, strlen(data->mtu));
        
        /* Format flags */
        char flag_str[16] = "";
        int flag_val = atoi(data->flags);
        format_interface_flags(flag_val, flag_str, sizeof(flag_str));
        table_update_width(&fmt, 4, strlen(flag_str));
        
        /* Calculate max width for all IPv4 addresses */
        int max_ipv4_width = 0;
        for (int j = 0; j < data->addr_count; j++) {
            int len = strlen(data->addr_list[j]);
            if (len > max_ipv4_width) max_ipv4_width = len;
        }
        table_update_width(&fmt, 5, max_ipv4_width);
        
        /* Calculate max width for all IPv6 addresses */
        int max_ipv6_width = 0;
        for (int j = 0; j < data->addr6_count; j++) {
            int len = strlen(data->addr6_list[j]);
            if (len > max_ipv6_width) max_ipv6_width = len;
        }
        table_update_width(&fmt, 6, max_ipv6_width);
        
        /* Calculate width for groups */
        table_update_width(&fmt, 7, strlen(data->groups));
    }
    
    /* Print table header */
    table_print_header(&fmt);
    
    /* Print each VXLAN interface */
    for (int i = 0; i < filtered_count; i++) {
        struct interface_data *data = &interfaces[i];
        
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
                while (*token == ' ') token++;
                char *end = token + strlen(token) - 1;
                while (end > token && *end == ' ') end--;
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
        if (data->addr_count > max_lines) max_lines = data->addr_count;
        if (data->addr6_count > max_lines) max_lines = data->addr6_count;
        if (group_count > max_lines) max_lines = group_count;
        
        /* Print each line */
        for (int line = 0; line < max_lines; line++) {
            const char *name = (line == 0) ? data->name : "";
            const char *status = (line == 0) ? (strcmp(data->enabled, "true") == 0 ? "UP" : "DOWN") : "";
            const char *vrf = (line == 0) ? data->fib : "";
            const char *mtu = (line == 0) ? data->mtu : "";
            const char *flags = (line == 0) ? flag_str : "";
            const char *ipv4 = (line < data->addr_count) ? data->addr_list[line] : "";
            const char *ipv6 = (line < data->addr6_count) ? data->addr6_list[line] : "";
            const char *groups = (line < group_count) ? group_array[line] : "";
            
            table_print_row(&fmt, name, status, vrf, mtu, flags, ipv4, ipv6, groups);
        }
    }
    
    /* Print table footer */
    char footer_text[64];
    snprintf(footer_text, sizeof(footer_text), "Total VXLAN interfaces: %d", filtered_count);
    table_print_footer(&fmt, footer_text);
    
    free(interfaces);
} 