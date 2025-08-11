#include <net.h>
#include <table.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Parse VXLAN interfaces from XML response
 * @param xml XML response string
 * @param interfaces Array to store parsed interfaces
 * @param max_interfaces Maximum number of interfaces to parse
 * @return Number of interfaces parsed, or -1 on error
 */
int parse_vxlan_interfaces_from_xml(const char *xml,
                                    struct vxlan_interface_data *interfaces,
                                    int max_interfaces) {
    if (!xml || !interfaces || max_interfaces <= 0) {
        return -1;
    }

    /* Parse all interfaces first */
    struct interface_data *base_interfaces =
        malloc(max_interfaces * sizeof(struct interface_data));
    if (!base_interfaces) {
        return -1;
    }

    int base_count = parse_interfaces_from_xml(xml, base_interfaces, max_interfaces);
    if (base_count < 0) {
        free(base_interfaces);
        return -1;
    }

    /* Filter for VXLAN interfaces and copy to vxlan_interface_data */
    int vxlan_count = 0;
    for (int i = 0; i < base_count && vxlan_count < max_interfaces; i++) {
        struct interface_data *base = &base_interfaces[i];
        
        /* Check if interface is a VXLAN by type */
        if (strcmp(base->type, "vxlan") == 0) {
            
            /* Copy base data */
            memcpy(&interfaces[vxlan_count].base, base, sizeof(struct interface_data));
            
            /* Initialize VXLAN-specific fields */
            interfaces[vxlan_count].vni = 0;  /* Will be populated from XML if available */
            memset(interfaces[vxlan_count].vxlan_local, 0, sizeof(interfaces[vxlan_count].vxlan_local));
            memset(interfaces[vxlan_count].vxlan_remote, 0, sizeof(interfaces[vxlan_count].vxlan_remote));
            memset(interfaces[vxlan_count].vxlan_dev, 0, sizeof(interfaces[vxlan_count].vxlan_dev));
            interfaces[vxlan_count].vxlan_ttl = 0;
            memset(interfaces[vxlan_count].vxlan_group, 0, sizeof(interfaces[vxlan_count].vxlan_group));
            
            vxlan_count++;
        }
    }

    free(base_interfaces);
    return vxlan_count;
}

void print_vxlan_table(const char *xml_response) {
    struct table_format fmt;
    struct vxlan_interface_data *interfaces = NULL;
    int max_interfaces = 100;
    int interface_count = 0;
    int filtered_count = 0;

    debug_log(INFO, "Printing VXLAN interface table");

    if (!xml_response) {
        print_error("XML response is NULL");
        return;
    }

    /* Allocate interface arrays */
    interfaces = malloc(max_interfaces * sizeof(struct vxlan_interface_data));
    if (!interfaces) {
        print_error("Failed to allocate memory for interfaces");
        return;
    }

    /* Parse VXLAN interfaces from XML */
    interface_count = parse_vxlan_interfaces_from_xml(xml_response, interfaces, max_interfaces);
    if (interface_count < 0) {
        print_error("Failed to parse interfaces from XML");
        free(interfaces);
        return;
    }

    if (interface_count == 0) {
        printf("No VXLAN interfaces found.\n");
        free(interfaces);
        return;
    }

    filtered_count = interface_count;

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
    for (int i = 0; i < filtered_count; i++) {
        struct vxlan_interface_data *data = &interfaces[i];
            table_update_width(&fmt, 0, strlen(data->base.name));
            table_update_width(&fmt, 1, strlen(strcmp(data->base.enabled, "true") == 0 ? "UP" : "DOWN"));
            table_update_width(&fmt, 2, strlen(data->base.fib));
            table_update_width(&fmt, 3, strlen(data->base.mtu));
            
            /* Format flags */
            char flag_str[16] = "";
            int flag_val = atoi(data->base.flags);
            format_interface_flags(flag_val, flag_str, sizeof(flag_str));
            table_update_width(&fmt, 4, strlen(flag_str));
            
            /* VNI field */
            char vni_str[16];
            snprintf(vni_str, sizeof(vni_str), "%d", data->vni);
            table_update_width(&fmt, 5, strlen(vni_str));
            
            table_update_width(&fmt, 6, strlen(data->base.addr));
            table_update_width(&fmt, 7, strlen(data->base.addr6));
            table_update_width(&fmt, 8, strlen(data->base.groups));
    }

    /* Print table header */
    table_print_header(&fmt);

    /* Print each VXLAN interface */
    for (int i = 0; i < filtered_count; i++) {
        struct vxlan_interface_data *data = &interfaces[i];
            /* Format flags */
            char flag_str[16] = "";
            int flag_val = atoi(data->base.flags);
            format_interface_flags(flag_val, flag_str, sizeof(flag_str));
            
            /* VNI field */
            char vni_str[16];
            snprintf(vni_str, sizeof(vni_str), "%d", data->vni);

            table_print_row(&fmt, data->base.name, 
                           strcmp(data->base.enabled, "true") == 0 ? "UP" : "DOWN",
                           data->base.fib, data->base.mtu, flag_str, vni_str,
                           data->base.addr, data->base.addr6, data->base.groups);
    }

    /* Print table footer */
    char footer_text[64];
    snprintf(footer_text, sizeof(footer_text), "Total VXLAN interfaces: %d", filtered_count);
    table_print_footer(&fmt, footer_text);

    free(interfaces);
} 