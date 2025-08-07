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

#include "netd.h"
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * Create a new interface or find existing one
 * @param state Server state
 * @param name Interface name
 * @param type Interface type
 * @return 0 on success, -1 on failure
 */
int interface_create(netd_state_t *state, const char *name, interface_type_t type)
{
    interface_t *iface;

    if (!state || !name || !is_valid_interface_name(name)) {
        debug_log(DEBUG_ERROR, "Invalid parameters for interface creation: state=%p, name=%s, type=%d", 
                  state, name ? name : "NULL", type);
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Creating interface '%s' of type '%s'", name, interface_type_to_string(type));

    /* Check if interface already exists in our list */
    iface = interface_find(state, name);
    if (iface) {
        debug_log(DEBUG_DEBUG, "Interface %s already exists in state, updating configuration", name);
        return 0; /* Interface exists, just return success */
    }

    /* Check if this is a hardware interface that already exists in the system */
    bool is_hardware = freebsd_is_hardware_interface(name);
    debug_log(DEBUG_DEBUG, "Interface %s is %s", name, is_hardware ? "hardware" : "virtual");

    /* Allocate new interface */
    iface = malloc(sizeof(*iface));
    if (!iface) {
        debug_log(DEBUG_ERROR, "Failed to allocate memory for interface %s", name);
        return -1;
    }

    /* Initialize interface */
    memset(iface, 0, sizeof(*iface));
    strlcpy(iface->name, name, sizeof(iface->name));
    iface->type = type;
    iface->fib = 0; /* Default FIB */
    iface->group_count = 0;
    iface->enabled = true;

    /* Only create interface in FreeBSD if it's not a hardware interface */
    if (!is_hardware) {
        /* Check if interface already exists in system */
        if (freebsd_interface_exists(name)) {
            debug_log(DEBUG_DEBUG, "Interface %s already exists in system, adding to state", name);
        } else {
            debug_log(DEBUG_DEBUG, "Creating virtual interface %s in FreeBSD", name);
            int result = freebsd_interface_create(name, type);
            if (result < 0) {
                debug_log(DEBUG_ERROR, "Failed to create interface %s in FreeBSD", name);
                free(iface);
                return -1;
            }
            debug_log(DEBUG_INFO, "Created virtual interface %s of type %s", name, interface_type_to_string(type));
        }
    } else {
        debug_log(DEBUG_INFO, "Adding existing hardware interface %s of type %s to state", name, interface_type_to_string(type));
    }

    /* Add to interface list */
    TAILQ_INSERT_TAIL(&state->interfaces, iface, entries);
    debug_log(DEBUG_DEBUG, "Added interface %s to state list", name);

    return 0;
}

/**
 * Delete an interface
 * @param state Server state
 * @param name Interface name
 * @return 0 on success, -1 on failure
 */
int interface_delete(netd_state_t *state, const char *name)
{
    interface_t *iface;

    if (!state || !name) {
        debug_log(DEBUG_ERROR, "Invalid parameters for interface deletion: state=%p, name=%s", 
                  state, name ? name : "NULL");
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Deleting interface '%s'", name);

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Found interface %s in state, deleting from FreeBSD", name);

    /* Delete interface in FreeBSD */
    if (freebsd_interface_delete(name) < 0) {
        debug_log(DEBUG_ERROR, "Failed to delete interface %s in FreeBSD", name);
        return -1;
    }

    /* Remove from interface list */
    TAILQ_REMOVE(&state->interfaces, iface, entries);
    free(iface);

    debug_log(DEBUG_INFO, "Deleted interface %s", name);
    return 0;
}

/**
 * Set interface FIB assignment
 * @param state Server state
 * @param name Interface name
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
int interface_set_fib(netd_state_t *state, const char *name, uint32_t fib)
{
    interface_t *iface;

    if (!state || !name || !is_valid_fib_number(fib)) {
        debug_log(DEBUG_ERROR, "Invalid parameters for interface FIB assignment: state=%p, name=%s, fib=%u", 
                  state, name ? name : "NULL", fib);
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Setting FIB %u for interface '%s'", fib, name);

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Found interface %s in state, setting FIB in FreeBSD", name);

    /* Set FIB in FreeBSD */
    if (freebsd_interface_set_fib(name, fib) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set FIB %u for interface %s in FreeBSD", fib, name);
        return -1;
    }

    /* Update interface state */
    iface->fib = fib;

    debug_log(DEBUG_INFO, "Set FIB %u for interface %s", fib, name);
    return 0;
}

/**
 * Add interface to group
 * @param state Server state
 * @param name Interface name
 * @param group Group name
 * @return 0 on success, -1 on failure
 */
int interface_add_group(netd_state_t *state, const char *name, const char *group)
{
    interface_t *iface;
    int i;

    if (!state || !name || !group) {
        debug_log(DEBUG_ERROR, "Invalid parameters for interface group addition: state=%p, name=%s, group=%s", 
                  state, name ? name : "NULL", group ? group : "NULL");
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Adding interface '%s' to group '%s'", name, group);

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
        return -1;
    }

    /* Check if already in group */
    for (i = 0; i < iface->group_count; i++) {
        if (strcmp(iface->groups[i], group) == 0) {
            debug_log(DEBUG_DEBUG, "Interface %s already in group %s", name, group);
            return 0; /* Already in group */
        }
    }

    /* Check if group limit reached */
    if (iface->group_count >= MAX_GROUPS_PER_IF) {
        debug_log(DEBUG_ERROR, "Interface %s has reached maximum number of groups (%d)", name, MAX_GROUPS_PER_IF);
        return -1;
    }

    /* Add to group */
    strlcpy(iface->groups[iface->group_count], group, MAX_GROUP_NAME_LEN);
    iface->group_count++;

    debug_log(DEBUG_INFO, "Added interface %s to group %s (total groups: %d)", name, group, iface->group_count);
    return 0;
}

/**
 * Remove interface from group
 * @param state Server state
 * @param name Interface name
 * @param group Group name
 * @return 0 on success, -1 on failure
 */
int interface_remove_group(netd_state_t *state, const char *name, const char *group)
{
    interface_t *iface;
    int i, j;

    if (!state || !name || !group) {
        debug_log(DEBUG_ERROR, "Invalid parameters for interface group removal: state=%p, name=%s, group=%s", 
                  state, name ? name : "NULL", group ? group : "NULL");
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Removing interface '%s' from group '%s'", name, group);

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
        return -1;
    }

    /* Find and remove group */
    for (i = 0; i < iface->group_count; i++) {
        if (strcmp(iface->groups[i], group) == 0) {
            debug_log(DEBUG_DEBUG, "Found group %s at index %d, removing", group, i);
            /* Shift remaining groups */
            for (j = i; j < iface->group_count - 1; j++) {
                strlcpy(iface->groups[j], iface->groups[j + 1], MAX_GROUP_NAME_LEN);
            }
            iface->group_count--;
            debug_log(DEBUG_INFO, "Removed interface %s from group %s (remaining groups: %d)", name, group, iface->group_count);
            return 0;
        }
    }

    debug_log(DEBUG_ERROR, "Interface %s not in group %s", name, group);
    return -1; /* Group not found */
}

/**
 * Set interface address
 * @param state Server state
 * @param name Interface name
 * @param address Address string
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int interface_set_address(netd_state_t *state, const char *name, const char *address, int family)
{
    interface_t *iface;

    if (!state || !name || !address) {
        debug_log(DEBUG_ERROR, "Invalid parameters for interface address assignment: state=%p, name=%s, address=%s", 
                  state, name ? name : "NULL", address ? address : "NULL");
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Setting %s address '%s' for interface '%s'", 
              family == AF_INET ? "IPv4" : family == AF_INET6 ? "IPv6" : "unknown", address, name);

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
        return -1;
    }

    /* Set address in FreeBSD */
    if (freebsd_interface_set_address(name, address, family) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set address %s for interface %s in FreeBSD", address, name);
        return -1;
    }

    debug_log(DEBUG_INFO, "Set address %s for interface %s", address, name);
    return 0;
}

/**
 * Delete interface address
 * @param state Server state
 * @param name Interface name
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int interface_delete_address(netd_state_t *state, const char *name, int family)
{
    interface_t *iface;

    if (!state || !name) {
        debug_log(DEBUG_ERROR, "Invalid parameters for interface address deletion: state=%p, name=%s", 
                  state, name ? name : "NULL");
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Deleting %s address from interface '%s'", 
              family == AF_INET ? "IPv4" : family == AF_INET6 ? "IPv6" : "unknown", name);

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
        return -1;
    }

    /* Delete address in FreeBSD */
    if (freebsd_interface_delete_address(name, family) < 0) {
        debug_log(DEBUG_ERROR, "Failed to delete address from interface %s in FreeBSD", name);
        return -1;
    }

    debug_log(DEBUG_INFO, "Deleted address from interface %s", name);
    return 0;
}

/**
 * Set interface MTU
 * @param state Server state
 * @param name Interface name
 * @param mtu MTU value
 * @return 0 on success, -1 on failure
 */
int interface_set_mtu(netd_state_t *state, const char *name, int mtu)
{
    interface_t *iface;

    if (!state || !name || mtu <= 0) {
        debug_log(DEBUG_ERROR, "Invalid parameters for interface MTU setting: state=%p, name=%s, mtu=%d", 
                  state, name ? name : "NULL", mtu);
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Setting MTU %d for interface '%s'", mtu, name);

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found in state", name);
        return -1;
    }

    /* Set MTU in FreeBSD */
    if (freebsd_interface_set_mtu(name, mtu) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set MTU %d for interface %s in FreeBSD", mtu, name);
        return -1;
    }

    debug_log(DEBUG_INFO, "Set MTU %d for interface %s", mtu, name);
    return 0;
}

/**
 * Find interface by name
 * @param state Server state
 * @param name Interface name
 * @return Interface structure or NULL if not found
 */
interface_t *interface_find(netd_state_t *state, const char *name)
{
    interface_t *iface;

    if (!state || !name) {
        return NULL;
    }

    TAILQ_FOREACH(iface, &state->interfaces, entries) {
        if (strcmp(iface->name, name) == 0) {
            return iface;
        }
    }

    return NULL;
}

/**
 * List interfaces
 * @param state Server state
 * @param type Interface type filter (IF_TYPE_UNKNOWN for all)
 * @return 0 on success, -1 on failure
 */
int interface_list(netd_state_t *state, interface_type_t type)
{
    interface_t *iface;
    int count = 0;

    if (!state) {
        debug_log(DEBUG_ERROR, "Invalid state parameter for interface listing");
        return -1;
    }

    debug_log(DEBUG_INFO, "Listing interfaces%s", 
              type == IF_TYPE_UNKNOWN ? "" : " of specific type");

    TAILQ_FOREACH(iface, &state->interfaces, entries) {
        if (type == IF_TYPE_UNKNOWN || iface->type == type) {
            debug_log(DEBUG_INFO, "  %s (%s) fib=%u enabled=%s", 
                      iface->name, 
                      interface_type_to_string(iface->type),
                      iface->fib,
                      iface->enabled ? "yes" : "no");
            
            if (iface->group_count > 0) {
                debug_log(DEBUG_INFO, "    Groups:");
                for (int i = 0; i < iface->group_count; i++) {
                    debug_log(DEBUG_INFO, "      %s", iface->groups[i]);
                }
            }
            count++;
        }
    }

    if (count == 0) {
        debug_log(DEBUG_INFO, "  No interfaces found");
    } else {
        debug_log(DEBUG_INFO, "Total interfaces: %d", count);
    }

    return 0;
} 

/**
 * Enumerate system interfaces and populate internal state
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int interface_enumerate_system(netd_state_t *state)
{
    if (!state) {
        debug_log(DEBUG_ERROR, "Invalid state parameter for system interface enumeration");
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Starting system interface enumeration");
    
    /* Call the system-specific enumeration function */
    int result = freebsd_enumerate_interfaces(state);
    if (result == 0) {
        debug_log(DEBUG_DEBUG, "System interface enumeration completed successfully");
    } else {
        debug_log(DEBUG_ERROR, "System interface enumeration failed");
    }
    
    return result;
}

/**
 * Get all interfaces as XML for NETCONF response
 * @param state Server state
 * @return XML string (allocated) or NULL on failure
 */
char *interface_get_all(netd_state_t *state)
{
    interface_t *iface;
    char *xml = NULL;
    char *temp_xml = NULL;
    int interface_count = 0;
    
    if (!state) {
        debug_log(DEBUG_ERROR, "Invalid state parameter for interface XML generation");
        return NULL;
    }

    debug_log(DEBUG_DEBUG, "Generating XML for all interfaces");

    /* Always enumerate from system to get fresh data */
    debug_log(DEBUG_DEBUG, "Enumerating system interfaces for XML generation");
    
    /* Clear existing interface list */
    interface_t *iface_next;
    int cleared_count = 0;
    TAILQ_FOREACH_SAFE(iface, &state->interfaces, entries, iface_next) {
        TAILQ_REMOVE(&state->interfaces, iface, entries);
        free(iface);
        cleared_count++;
    }
    debug_log(DEBUG_DEBUG, "Cleared %d existing interfaces from state", cleared_count);
    
    if (interface_enumerate_system(state) < 0) {
        debug_log(DEBUG_ERROR, "Failed to enumerate system interfaces for XML generation");
        return NULL;
    }

    /* Start XML */
    asprintf(&xml, "    <interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">\n");
    if (!xml) {
        debug_log(DEBUG_ERROR, "Failed to allocate memory for XML header");
        return NULL;
    }

    TAILQ_FOREACH(iface, &state->interfaces, entries) {
        interface_count++;
        debug_log(DEBUG_TRACE, "Processing interface %d: %s (type: %s)", 
                  interface_count, iface->name, interface_type_to_string(iface->type));
        
        if (strncmp(iface->name, "bridge", 6) == 0) {
            debug_log(DEBUG_TRACE, "Generating XML for bridge interface: %s", iface->name);
        }

        /* Generate interface XML */
        const char *namespace = interface_type_get_namespace(iface->type);
        const char *type_str = interface_type_to_string(iface->type);
        
        if (strcmp(namespace, "netd") == 0) {
            /* Use netd namespace for FreeBSD-specific interface types */
            asprintf(&temp_xml,
                    "      <interface>\n"
                    "        <name>%s</name>\n"
                    "        <type xmlns:netd=\"urn:ietf:params:xml:ns:yang:netd\">netd:%s</type>\n"
                    "        <enabled>%s</enabled>\n"
                    "        <oper-status>%s</oper-status>\n"
                    "        <flags xmlns=\"urn:ietf:params:xml:ns:yang:netd\">%d</flags>\n",
                    iface->name,
                    type_str,
                    iface->enabled ? "true" : "false",
                    freebsd_get_interface_oper_status(iface->flags),
                    iface->flags);
        } else {
            /* Use IANA namespace for standard interface types */
            asprintf(&temp_xml,
                    "      <interface>\n"
                    "        <name>%s</name>\n"
                    "        <type xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\">ianaift:%s</type>\n"
                    "        <enabled>%s</enabled>\n"
                    "        <oper-status>%s</oper-status>\n"
                    "        <flags xmlns=\"urn:ietf:params:xml:ns:yang:netd\">%d</flags>\n",
                    iface->name,
                    type_str,
                    iface->enabled ? "true" : "false",
                    freebsd_get_interface_oper_status(iface->flags),
                    iface->flags);
        }

        if (temp_xml) {
            /* Append to main XML */
            char *new_xml;
            asprintf(&new_xml, "%s%s", xml, temp_xml);
            free(xml);
            free(temp_xml);
            xml = new_xml;
        }

        /* Add IPv4 container with all addresses */
        if (iface->primary_address[0] != '\0' || iface->alias_count > 0) {
            debug_log(DEBUG_TRACE, "Adding IPv4 addresses for interface %s", iface->name);
            asprintf(&temp_xml,
                    "        <ipv4 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">\n"
                    "          <mtu>%d</mtu>\n",
                    iface->mtu);
            if (temp_xml) {
                char *new_xml;
                asprintf(&new_xml, "%s%s", xml, temp_xml);
                free(xml);
                free(temp_xml);
                xml = new_xml;
            }

            /* Add primary IPv4 address */
            if (iface->primary_address[0] != '\0') {
                debug_log(DEBUG_TRACE, "Adding primary IPv4 address %s for interface %s", iface->primary_address, iface->name);
                asprintf(&temp_xml,
                        "          <address>\n"
                        "            <ip>%s</ip>\n"
                        "            <prefix-length>24</prefix-length>\n"
                        "          </address>\n",
                        iface->primary_address);
                if (temp_xml) {
                    char *new_xml;
                    asprintf(&new_xml, "%s%s", xml, temp_xml);
                    free(xml);
                    free(temp_xml);
                    xml = new_xml;
                }
            }

            /* Add alias IPv4 addresses */
            for (int i = 0; i < iface->alias_count; i++) {
                debug_log(DEBUG_TRACE, "Adding IPv4 alias %s for interface %s", iface->alias_addresses[i], iface->name);
                asprintf(&temp_xml,
                        "          <address>\n"
                        "            <ip>%s</ip>\n"
                        "            <prefix-length>24</prefix-length>\n"
                        "          </address>\n",
                        iface->alias_addresses[i]);
                if (temp_xml) {
                    char *new_xml;
                    asprintf(&new_xml, "%s%s", xml, temp_xml);
                    free(xml);
                    free(temp_xml);
                    xml = new_xml;
                }
            }

            /* Close IPv4 container */
            asprintf(&temp_xml, "        </ipv4>\n");
            if (temp_xml) {
                char *new_xml;
                asprintf(&new_xml, "%s%s", xml, temp_xml);
                free(xml);
                free(temp_xml);
                xml = new_xml;
            }
        }

        /* Add IPv6 container with all addresses */
        if (iface->primary_address6[0] != '\0' || iface->alias_count6 > 0) {
            debug_log(DEBUG_TRACE, "Adding IPv6 addresses for interface %s", iface->name);
            asprintf(&temp_xml,
                    "        <ipv6 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">\n");

            if (temp_xml) {
                char *new_xml;
                asprintf(&new_xml, "%s%s", xml, temp_xml);
                free(xml);
                free(temp_xml);
                xml = new_xml;
            }

            /* Add primary IPv6 address */
            if (iface->primary_address6[0] != '\0') {
                debug_log(DEBUG_TRACE, "Adding primary IPv6 address %s for interface %s", iface->primary_address6, iface->name);
                asprintf(&temp_xml,
                        "          <address>\n"
                        "            <ip>%s</ip>\n"
                        "            <prefix-length>64</prefix-length>\n"
                        "          </address>\n",
                        iface->primary_address6);
                if (temp_xml) {
                    char *new_xml;
                    asprintf(&new_xml, "%s%s", xml, temp_xml);
                    free(xml);
                    free(temp_xml);
                    xml = new_xml;
                }
            }

            /* Add alias IPv6 addresses */
            for (int i = 0; i < iface->alias_count6; i++) {
                debug_log(DEBUG_TRACE, "Adding IPv6 alias %s for interface %s", iface->alias_addresses6[i], iface->name);
                asprintf(&temp_xml,
                        "          <address>\n"
                        "            <ip>%s</ip>\n"
                        "            <prefix-length>64</prefix-length>\n"
                        "          </address>\n",
                        iface->alias_addresses6[i]);
                if (temp_xml) {
                    char *new_xml;
                    asprintf(&new_xml, "%s%s", xml, temp_xml);
                    free(xml);
                    free(temp_xml);
                    xml = new_xml;
                }
            }

            /* Close IPv6 container */
            asprintf(&temp_xml, "        </ipv6>\n");
            if (temp_xml) {
                char *new_xml;
                asprintf(&new_xml, "%s%s", xml, temp_xml);
                free(xml);
                free(temp_xml);
                xml = new_xml;
            }
        }

        /* Add groups if any */
        if (iface->group_count > 0) {
            debug_log(DEBUG_TRACE, "Adding %d groups for interface %s", iface->group_count, iface->name);
            /* Join all groups into a comma-separated string */
            char group_string[256] = "";
            for (int i = 0; i < iface->group_count; i++) {
                if (i > 0) {
                    strcat(group_string, ",");
                }
                strcat(group_string, iface->groups[i]);
            }
            
            asprintf(&temp_xml, "        <group xmlns=\"urn:ietf:params:xml:ns:yang:netd\">%s</group>\n", group_string);
            if (temp_xml) {
                char *new_xml;
                asprintf(&new_xml, "%s%s", xml, temp_xml);
                free(xml);
                free(temp_xml);
                xml = new_xml;
            }
        }

        /* Add bridge members if any */
        if (iface->type == IF_TYPE_BRIDGE && iface->bridge_members[0] != '\0') {
            debug_log(DEBUG_TRACE, "Adding bridge members %s for interface %s", iface->bridge_members, iface->name);
            asprintf(&temp_xml, "        <bridge-members xmlns=\"urn:ietf:params:xml:ns:yang:netd\">%s</bridge-members>\n", iface->bridge_members);
            if (temp_xml) {
                char *new_xml;
                asprintf(&new_xml, "%s%s", xml, temp_xml);
                free(xml);
                free(temp_xml);
                xml = new_xml;
            }
        }
        
        /* Add VLAN information if any */
        if ((iface->type == IF_TYPE_VLAN || strchr(iface->name, '.') != NULL) && iface->vlan_id > 0) {
            debug_log(DEBUG_TRACE, "Adding VLAN info for interface %s: id=%d, proto=%s, pcp=%d, parent=%s", 
                      iface->name, iface->vlan_id, iface->vlan_proto, iface->vlan_pcp, iface->vlan_parent);
            asprintf(&temp_xml, 
                    "        <vlan-info xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
                    "          <vlan-id>%d</vlan-id>\n"
                    "          <vlan-proto>%s</vlan-proto>\n"
                    "          <vlan-pcp>%d</vlan-pcp>\n"
                    "          <vlan-parent>%s</vlan-parent>\n"
                    "        </vlan-info>\n", 
                    iface->vlan_id, iface->vlan_proto, iface->vlan_pcp, iface->vlan_parent);
            if (temp_xml) {
                char *new_xml;
                asprintf(&new_xml, "%s%s", xml, temp_xml);
                free(xml);
                free(temp_xml);
                xml = new_xml;
            }
                }
        
        /* Add WiFi information if any */
        if (iface->type == IF_TYPE_WIRELESS && iface->wifi_regdomain[0] != '\0') {
            debug_log(DEBUG_TRACE, "Adding WiFi info for interface %s: regdomain=%s, country=%s, authmode=%s, privacy=%s, txpower=%d, bmiss=%d, scanvalid=%d, features=%s, bintval=%d, parent=%s", 
                      iface->name, iface->wifi_regdomain, iface->wifi_country, iface->wifi_authmode, 
                      iface->wifi_privacy, iface->wifi_txpower, iface->wifi_bmiss, iface->wifi_scanvalid,
                      iface->wifi_features, iface->wifi_bintval, iface->wifi_parent);
            asprintf(&temp_xml, 
                    "        <wifi-info xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
                    "          <regdomain>%s</regdomain>\n"
                    "          <country>%s</country>\n"
                    "          <authmode>%s</authmode>\n"
                    "          <privacy>%s</privacy>\n"
                    "          <txpower>%d</txpower>\n"
                    "          <bmiss>%d</bmiss>\n"
                    "          <scanvalid>%d</scanvalid>\n"
                    "          <features>%s</features>\n"
                    "          <bintval>%d</bintval>\n"
                    "          <parent>%s</parent>\n"
                    "        </wifi-info>\n", 
                    iface->wifi_regdomain, iface->wifi_country, iface->wifi_authmode, 
                    iface->wifi_privacy, iface->wifi_txpower, iface->wifi_bmiss, iface->wifi_scanvalid,
                    iface->wifi_features, iface->wifi_bintval, iface->wifi_parent);
            if (temp_xml) {
                char *new_xml;
                asprintf(&new_xml, "%s%s", xml, temp_xml);
                free(xml);
                free(temp_xml);
                xml = new_xml;
            }
        }
        
        /* Add statistics container with mandatory discontinuity-time */
        /* Use the lowest possible date representing "no recent discontinuities" */
        asprintf(&temp_xml,
                "        <statistics>\n"
                "          <discontinuity-time>1970-01-01T00:00:00Z</discontinuity-time>\n"
                "        </statistics>\n");
        if (temp_xml) {
            char *new_xml;
            asprintf(&new_xml, "%s%s", xml, temp_xml);
            free(xml);
            free(temp_xml);
            xml = new_xml;
        }

        /* Close interface tag */
        asprintf(&temp_xml, "      </interface>\n");
        if (temp_xml) {
            char *new_xml;
            asprintf(&new_xml, "%s%s", xml, temp_xml);
            free(xml);
            free(temp_xml);
            xml = new_xml;
        }
    }

    /* Close interfaces tag */
    asprintf(&temp_xml, "    </interfaces>\n");
    if (temp_xml) {
        char *new_xml;
        asprintf(&new_xml, "%s%s", xml, temp_xml);
        free(xml);
        free(temp_xml);
        xml = new_xml;
    }

    /* Add network instances data for leafref validation */
    debug_log(DEBUG_DEBUG, "Adding VRF data for leafref validation");
    char *vrf_xml = vrf_get_all(state);
    if (vrf_xml) {
        char *new_xml;
        asprintf(&new_xml, "%s%s", xml, vrf_xml);
        free(xml);
        free(vrf_xml);
        xml = new_xml;
        debug_log(DEBUG_DEBUG, "Added VRF data to interface XML");
    }

    /* Validate the generated XML against YANG schema if YANG context is available */
    if (state->yang_ctx && xml) {
        debug_log(DEBUG_DEBUG, "Validating generated interface XML against YANG schema");
        if (yang_validate_xml(state, xml) < 0) {
            debug_log(DEBUG_WARN, "Generated interface XML failed YANG validation, but returning anyway");
            /* Don't fail the request, just log a warning */
        } else {
            debug_log(DEBUG_DEBUG, "Generated interface XML validated successfully against YANG schema");
        }
    } else {
        debug_log(DEBUG_DEBUG, "Skipping YANG validation (no context or XML)");
    }

    debug_log(DEBUG_INFO, "Generated XML for %d interfaces (%zu bytes)", interface_count, xml ? strlen(xml) : 0);
    return xml;
} 

/**
 * Add an interface to the system
 * @param state Server state
 * @param name Interface name
 * @return 0 on success, -1 on failure
 */
int interface_add(netd_state_t *state, const char *name)
{
    if (!state || !name) {
        debug_log(DEBUG_ERROR, "Invalid parameters for interface add: state=%p, name=%s", 
                  state, name ? name : "NULL");
        return -1;
    }

    /* Check if interface already exists */
    if (interface_find(state, name)) {
        debug_log(DEBUG_WARN, "Interface %s already exists in state", name);
        return 0; /* Already exists, consider it a success */
    }

    /* For now, just add to state without creating in system */
    /* This is a placeholder - in a real implementation, you'd create the interface */
    debug_log(DEBUG_INFO, "Adding interface %s to state (placeholder implementation)", name);
    return 0;
}

/**
 * Modify an interface property
 * @param state Server state
 * @param name Interface name
 * @param property Property to modify
 * @param value New value
 * @return 0 on success, -1 on failure
 */
int interface_modify(netd_state_t *state, const char *name, const char *property, const char *value)
{
    if (!state || !name || !property || !value) {
        debug_log(DEBUG_ERROR, "Invalid parameters for interface modify: state=%p, name=%s, property=%s, value=%s", 
                  state, name ? name : "NULL", property ? property : "NULL", value ? value : "NULL");
        return -1;
    }

    /* Find the interface */
    interface_t *iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found", name);
        return -1;
    }

    /* Handle different properties */
    if (strcmp(property, "vrf") == 0) {
        /* Set VRF/FIB for interface */
        char *endptr;
        uint32_t fib = strtoul(value, &endptr, 10);
        if (*endptr != '\0') {
            debug_log(DEBUG_ERROR, "Invalid VRF/FIB number: %s", value);
            return -1;
        }
        return interface_set_fib(state, name, fib);
    } else if (strcmp(property, "peer") == 0) {
        /* Handle peer property for epair interfaces */
        debug_log(DEBUG_INFO, "Setting peer %s for interface %s", value, name);
        /* This would typically involve setting up the epair peer relationship */
        return 0;
    } else {
        debug_log(DEBUG_WARN, "Unsupported interface property: %s", property);
        return -1;
    }
} 