#include "net.h"
#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_vxlan_table(const char *xml_response) {
    struct table_format fmt;
    struct interface_data *interfaces = NULL;
    int max_interfaces = 100;
    int interface_count = 0;
    int filtered_count = 0;

    debug_log(DEBUG_INFO, "Printing VXLAN interface table");

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

    /* Parse interfaces from XML */
    interface_count = parse_interfaces_from_xml(xml_response, interfaces, max_interfaces);
    if (interface_count < 0) {
        print_error("Failed to parse interfaces from XML");
        free(interfaces);
        return;
    }

    /* Filter for VXLAN interfaces */
    for (int i = 0; i < interface_count; i++) {
        struct interface_data *data = &interfaces[i];
        if (strcmp(data->type, "vxlan") == 0) {
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
    table_add_column(&fmt, "VNI", 3);
    table_add_column(&fmt, "IPv4", 4);
    table_add_column(&fmt, "IPv6", 4);
    table_add_column(&fmt, "Groups", 6);

    /* First pass: calculate column widths */
    for (int i = 0; i < interface_count; i++) {
        struct interface_data *data = &interfaces[i];
        if (strcmp(data->type, "vxlan") == 0) {
            table_update_width(&fmt, 0, strlen(data->name));
            table_update_width(&fmt, 1, strlen(strcmp(data->enabled, "true") == 0 ? "UP" : "DOWN"));
            table_update_width(&fmt, 2, strlen(data->fib));
            table_update_width(&fmt, 3, strlen(data->mtu));
            
            /* Format flags */
            char flag_str[16] = "";
            int flag_val = atoi(data->flags);
            format_interface_flags(flag_val, flag_str, sizeof(flag_str));
            table_update_width(&fmt, 4, strlen(flag_str));
            
            /* VNI field */
            char vni_str[16];
            snprintf(vni_str, sizeof(vni_str), "%d", data->vni);
            table_update_width(&fmt, 5, strlen(vni_str));
            
            table_update_width(&fmt, 6, strlen(data->addr));
            table_update_width(&fmt, 7, strlen(data->addr6));
            table_update_width(&fmt, 8, strlen(data->groups));
        }
    }

    /* Print table header */
    table_print_header(&fmt);

    /* Print each VXLAN interface */
    for (int i = 0; i < interface_count; i++) {
        struct interface_data *data = &interfaces[i];
        if (strcmp(data->type, "vxlan") == 0) {
            /* Format flags */
            char flag_str[16] = "";
            int flag_val = atoi(data->flags);
            format_interface_flags(flag_val, flag_str, sizeof(flag_str));
            
            /* VNI field */
            char vni_str[16];
            snprintf(vni_str, sizeof(vni_str), "%d", data->vni);

            table_print_row(&fmt, data->name, 
                           strcmp(data->enabled, "true") == 0 ? "UP" : "DOWN",
                           data->fib, data->mtu, flag_str, vni_str,
                           data->addr, data->addr6, data->groups);
        }
    }

    /* Print table footer */
    char footer_text[64];
    snprintf(footer_text, sizeof(footer_text), "Total VXLAN interfaces: %d", filtered_count);
    table_print_footer(&fmt, footer_text);

    free(interfaces);
} 