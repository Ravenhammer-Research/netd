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

#include "ifgrp_wlan_table.h"
#include "if_table.h"
#include "table_utils.h"
#include "net.h"
#include <bsdxml.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * Parse wireless interface data from XML
 * @param xml XML string
 * @param interfaces Array to populate
 * @param max_interfaces Maximum number of interfaces
 * @return Number of interfaces parsed, -1 on error
 */
int parse_wlan_interfaces_from_xml(const char *xml, struct wlan_interface_data *interfaces, int max_interfaces)
{
    if (!xml || !interfaces || max_interfaces <= 0) {
        return -1;
    }

    /* For now, we'll use the existing interface parsing and extend it */
    struct interface_data *base_interfaces = malloc(max_interfaces * sizeof(struct interface_data));
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
        memcpy(&interfaces[i].base, &base_interfaces[i], sizeof(struct interface_data));
        
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
        /* This would require extending the XML parsing to handle wireless attributes */
    }

    free(base_interfaces);
    return count;
}

/**
 * Print wireless interface groups summary
 * @param xml_response XML response string
 * @return 0 on success, -1 on failure
 */
int print_wlan_interface_groups_summary(const char *xml_response)
{
    struct wlan_interface_data *interfaces = NULL;
    int interface_count = 0;
    int max_interfaces = 100;
    
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
    interface_count = parse_wlan_interfaces_from_xml(xml_response, interfaces, max_interfaces);
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
    
    struct wlan_group_count *groups = malloc(interface_count * sizeof(struct wlan_group_count));
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
                while (*token == ' ') token++;
                char *end = token + strlen(token) - 1;
                while (end > token && *end == ' ') end--;
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
                    strncpy(groups[group_count].name, token, sizeof(groups[group_count].name) - 1);
                    groups[group_count].name[sizeof(groups[group_count].name) - 1] = '\0';
                    groups[group_count].count = 1;
                    strncpy(groups[group_count].ssid, data->ssid, sizeof(groups[group_count].ssid) - 1);
                    groups[group_count].ssid[sizeof(groups[group_count].ssid) - 1] = '\0';
                    strncpy(groups[group_count].channel, data->channel, sizeof(groups[group_count].channel) - 1);
                    groups[group_count].channel[sizeof(groups[group_count].channel) - 1] = '\0';
                    strncpy(groups[group_count].mode, data->mode, sizeof(groups[group_count].mode) - 1);
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
    printf("%-20s %-15s %-8s %-10s %s\n", "Group Name", "SSID", "Channel", "Mode", "Interface Count");
    printf("-------------------- --------------- -------- ---------- -----\n");
    
    for (int i = 0; i < group_count; i++) {
        printf("%-20s %-15s %-8s %-10s %d\n", 
               groups[i].name, 
               groups[i].ssid, 
               groups[i].channel, 
               groups[i].mode, 
               groups[i].count);
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
int print_wlan_interface_table(const char *xml_response)
{
    struct table_format fmt;
    struct wlan_interface_data *interfaces = NULL;
    int interface_count = 0;
    int max_interfaces = 100;
    
    if (!xml_response) {
        print_error("XML response is NULL");
        return -1;
    }

    /* Initialize table format */
    table_init(&fmt, "Wireless Interface Table (Group: wlan)");
    table_add_column(&fmt, "Name", 4);
    table_add_column(&fmt, "Status", 6);
    table_add_column(&fmt, "SSID", 4);
    table_add_column(&fmt, "Channel", 7);
    table_add_column(&fmt, "Frequency", 9);
    table_add_column(&fmt, "Mode", 4);
    table_add_column(&fmt, "Security", 8);
    table_add_column(&fmt, "Signal", 6);
    table_add_column(&fmt, "Rate", 4);
    table_add_column(&fmt, "Groups", 6);
    
    /* Allocate interface array */
    interfaces = malloc(max_interfaces * sizeof(struct wlan_interface_data));
    if (!interfaces) {
        print_error("Failed to allocate memory for interfaces");
        return -1;
    }
    
    /* Parse interfaces from XML */
    interface_count = parse_wlan_interfaces_from_xml(xml_response, interfaces, max_interfaces);
    if (interface_count < 0) {
        print_error("Failed to parse interfaces from XML");
        free(interfaces);
        return -1;
    }
    
    /* Filter for wlan group only */
    int wlan_count = 0;
    for (int i = 0; i < interface_count; i++) {
        struct wlan_interface_data *data = &interfaces[i];
        if (strstr(data->base.groups, "wlan") != NULL) {
            /* Move this interface to the front of the array */
            if (wlan_count != i) {
                interfaces[wlan_count] = interfaces[i];
            }
            wlan_count++;
        }
    }
    
    /* Calculate column widths for wlan interfaces only */
    for (int i = 0; i < wlan_count; i++) {
        struct wlan_interface_data *data = &interfaces[i];
        
        table_update_width(&fmt, 0, strlen(data->base.name));
        table_update_width(&fmt, 1, strlen(strcmp(data->base.enabled, "true") == 0 ? "UP" : "DOWN"));
        table_update_width(&fmt, 2, strlen(data->ssid));
        table_update_width(&fmt, 3, strlen(data->channel));
        table_update_width(&fmt, 4, strlen(data->frequency));
        table_update_width(&fmt, 5, strlen(data->mode));
        table_update_width(&fmt, 6, strlen(data->security));
        table_update_width(&fmt, 7, strlen(data->signal_strength));
        table_update_width(&fmt, 8, strlen(data->rate));
        table_update_width(&fmt, 9, strlen(data->base.groups));
    }
    
    /* Print table header */
    table_print_header(&fmt);
    
    /* Print each wlan interface */
    for (int i = 0; i < wlan_count; i++) {
        struct wlan_interface_data *data = &interfaces[i];
        
        const char *status = strcmp(data->base.enabled, "true") == 0 ? "UP" : "DOWN";
        
        table_print_row(&fmt, 
                       data->base.name,
                       status,
                       data->ssid,
                       data->channel,
                       data->frequency,
                       data->mode,
                       data->security,
                       data->signal_strength,
                       data->rate,
                       data->base.groups);
    }
    
    /* Cleanup */
    free(interfaces);
    
    /* Print footer */
    printf("Total interfaces in group 'wlan': %d\n", wlan_count);
    printf("Total wireless interfaces displayed above.\n");
    printf("Flags: U=UP, B=BROADCAST, R=RUNNING, P=PROMISC, M=MULTICAST, L=LOOPBACK\n\n");
    
    return 0;
} 