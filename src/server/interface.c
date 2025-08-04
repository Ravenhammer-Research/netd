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
#include <ifaddrs.h>

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
        debug_log(DEBUG_ERROR, "Invalid parameters for interface creation");
        return -1;
    }

    /* Check if interface already exists in our list */
    iface = interface_find(state, name);
    if (iface) {
        debug_log(DEBUG_DEBUG, "Interface %s already exists in state, updating configuration", name);
        return 0; /* Interface exists, just return success */
    }

    /* Check if this is a hardware interface that already exists in the system */
    bool is_hardware = false;
    if (strncmp(name, "em", 2) == 0 || 
        strncmp(name, "igb", 3) == 0 ||
        strncmp(name, "ix", 2) == 0 ||
        strncmp(name, "bge", 3) == 0 ||
        strncmp(name, "fxp", 3) == 0 ||
        strncmp(name, "re", 2) == 0 ||
        strncmp(name, "rl", 2) == 0 ||
        strncmp(name, "sk", 2) == 0 ||
        strncmp(name, "ti", 2) == 0 ||
        strncmp(name, "tx", 2) == 0 ||
        strncmp(name, "vr", 2) == 0 ||
        strncmp(name, "xl", 2) == 0 ||
        strncmp(name, "wlan", 4) == 0 ||
        strncmp(name, "ath", 3) == 0 ||
        strncmp(name, "iwn", 3) == 0 ||
        strncmp(name, "iwm", 3) == 0 ||
        strncmp(name, "iwl", 3) == 0 ||
        strncmp(name, "bwi", 3) == 0 ||
        strncmp(name, "rum", 3) == 0 ||
        strncmp(name, "run", 3) == 0 ||
        strncmp(name, "ural", 4) == 0 ||
        strncmp(name, "urtw", 4) == 0 ||
        strncmp(name, "zyd", 3) == 0 ||
        strcmp(name, "lo0") == 0) {
        is_hardware = true;
    }

    /* Allocate new interface */
    iface = malloc(sizeof(*iface));
    if (!iface) {
        debug_log(DEBUG_ERROR, "Failed to allocate memory for interface");
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
        debug_log(DEBUG_ERROR, "Invalid parameters for interface deletion");
        return -1;
    }

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found", name);
        return -1;
    }

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
        debug_log(DEBUG_ERROR, "Invalid parameters for interface FIB assignment");
        return -1;
    }

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found", name);
        return -1;
    }

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
        debug_log(DEBUG_ERROR, "Invalid parameters for interface group addition");
        return -1;
    }

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found", name);
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
        debug_log(DEBUG_ERROR, "Interface %s has reached maximum number of groups", name);
        return -1;
    }

    /* Add to group */
    strlcpy(iface->groups[iface->group_count], group, MAX_GROUP_NAME_LEN);
    iface->group_count++;

    debug_log(DEBUG_INFO, "Added interface %s to group %s", name, group);
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
        debug_log(DEBUG_ERROR, "Invalid parameters for interface group removal");
        return -1;
    }

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found", name);
        return -1;
    }

    /* Find and remove group */
    for (i = 0; i < iface->group_count; i++) {
        if (strcmp(iface->groups[i], group) == 0) {
            /* Shift remaining groups */
            for (j = i; j < iface->group_count - 1; j++) {
                strlcpy(iface->groups[j], iface->groups[j + 1], MAX_GROUP_NAME_LEN);
            }
            iface->group_count--;
            debug_log(DEBUG_INFO, "Removed interface %s from group %s", name, group);
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
        debug_log(DEBUG_ERROR, "Invalid parameters for interface address assignment");
        return -1;
    }

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found", name);
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
        debug_log(DEBUG_ERROR, "Invalid parameters for interface address deletion");
        return -1;
    }

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found", name);
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
        debug_log(DEBUG_ERROR, "Invalid parameters for interface MTU setting");
        return -1;
    }

    /* Find interface */
    iface = interface_find(state, name);
    if (!iface) {
        debug_log(DEBUG_ERROR, "Interface %s not found", name);
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
        return -1;
    }

    debug_log(DEBUG_INFO, "Listing interfaces");

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
    struct ifaddrs *ifap, *ifa;
    interface_t *iface;
    char *last_name = NULL;
    
    if (!state) {
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Enumerating system interfaces");

    /* Get interface addresses */
    if (getifaddrs(&ifap) != 0) {
        debug_log(DEBUG_ERROR, "Failed to get interface addresses: %s", strerror(errno));
        return -1;
    }
    
    /* Iterate through interfaces */
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        /* Skip if we've already processed this interface name */
        if (last_name && strcmp(last_name, ifa->ifa_name) == 0) {
            continue;
        }
        
        /* Skip interfaces without addresses, except for certain virtual interface types */
        if (!ifa->ifa_addr) {
            /* For virtual interfaces like tap, tun, bridge, etc., we want to include them even without addresses */
            if (strncmp(ifa->ifa_name, "tap", 3) == 0 ||
                strncmp(ifa->ifa_name, "tun", 3) == 0 ||
                strncmp(ifa->ifa_name, "bridge", 6) == 0 ||
                strncmp(ifa->ifa_name, "vlan", 4) == 0 ||
                strncmp(ifa->ifa_name, "vxlan", 5) == 0 ||
                strncmp(ifa->ifa_name, "gif", 3) == 0 ||
                strncmp(ifa->ifa_name, "gre", 3) == 0 ||
                strncmp(ifa->ifa_name, "lagg", 4) == 0 ||
                strncmp(ifa->ifa_name, "epair", 5) == 0) {
                /* Continue processing these virtual interfaces even without addresses */
                debug_log(DEBUG_DEBUG, "Including virtual interface %s without address", ifa->ifa_name);
            } else {
                debug_log(DEBUG_DEBUG, "Skipping interface %s without address", ifa->ifa_name);
            continue;
            }
        }

        /* Skip loopback interface if it's not the main loopback */
        if (strcmp(ifa->ifa_name, "lo0") != 0 && 
            strncmp(ifa->ifa_name, "lo", 2) == 0) {
            continue;
        }

        /* Check if interface already exists in our list */
        if (interface_find(state, ifa->ifa_name)) {
            last_name = strdup(ifa->ifa_name);
            continue;
        }

        /* Determine interface type based on name */
        interface_type_t type = IF_TYPE_UNKNOWN;
        if (strncmp(ifa->ifa_name, "em", 2) == 0 || 
            strncmp(ifa->ifa_name, "igb", 3) == 0 ||
            strncmp(ifa->ifa_name, "ix", 2) == 0 ||
            strncmp(ifa->ifa_name, "bge", 3) == 0 ||
            strncmp(ifa->ifa_name, "fxp", 3) == 0 ||
            strncmp(ifa->ifa_name, "re", 2) == 0 ||
            strncmp(ifa->ifa_name, "rl", 2) == 0 ||
            strncmp(ifa->ifa_name, "sk", 2) == 0 ||
            strncmp(ifa->ifa_name, "ti", 2) == 0 ||
            strncmp(ifa->ifa_name, "tx", 2) == 0 ||
            strncmp(ifa->ifa_name, "vr", 2) == 0 ||
            strncmp(ifa->ifa_name, "xl", 2) == 0) {
            type = IF_TYPE_ETHERNET;
        } else if (strncmp(ifa->ifa_name, "wlan", 4) == 0 ||
                   strncmp(ifa->ifa_name, "ath", 3) == 0 ||
                   strncmp(ifa->ifa_name, "iwn", 3) == 0 ||
                   strncmp(ifa->ifa_name, "iwm", 3) == 0 ||
                   strncmp(ifa->ifa_name, "iwl", 3) == 0 ||
                   strncmp(ifa->ifa_name, "bwi", 3) == 0 ||
                   strncmp(ifa->ifa_name, "rum", 3) == 0 ||
                   strncmp(ifa->ifa_name, "run", 3) == 0 ||
                   strncmp(ifa->ifa_name, "ural", 4) == 0 ||
                   strncmp(ifa->ifa_name, "urtw", 4) == 0 ||
                   strncmp(ifa->ifa_name, "zyd", 3) == 0) {
            type = IF_TYPE_WIRELESS;
        } else if (strncmp(ifa->ifa_name, "epair", 5) == 0) {
            type = IF_TYPE_EPAIR;
        } else if (strncmp(ifa->ifa_name, "gif", 3) == 0) {
            type = IF_TYPE_GIF;
        } else if (strncmp(ifa->ifa_name, "gre", 3) == 0) {
            type = IF_TYPE_GRE;
        } else if (strncmp(ifa->ifa_name, "lagg", 4) == 0) {
            type = IF_TYPE_LAGG;
        } else if (strcmp(ifa->ifa_name, "lo0") == 0) {
            type = IF_TYPE_LOOPBACK;
        } else if (strncmp(ifa->ifa_name, "ovpn", 4) == 0) {
            type = IF_TYPE_OVPN;
        } else if (strncmp(ifa->ifa_name, "tun", 3) == 0) {
            type = IF_TYPE_TUN;
        } else if (strncmp(ifa->ifa_name, "tap", 3) == 0) {
            type = IF_TYPE_TAP;
        } else if (strncmp(ifa->ifa_name, "vlan", 4) == 0) {
            type = IF_TYPE_VLAN;
        } else if (strncmp(ifa->ifa_name, "vxlan", 5) == 0) {
            type = IF_TYPE_VXLAN;
        } else if (strncmp(ifa->ifa_name, "bridge", 6) == 0) {
            type = IF_TYPE_BRIDGE;
            debug_log(DEBUG_INFO, "Processing bridge interface: %s", ifa->ifa_name);
        }

        /* Allocate new interface */
        iface = malloc(sizeof(*iface));
        if (!iface) {
            debug_log(DEBUG_ERROR, "Failed to allocate memory for interface %s", ifa->ifa_name);
            continue;
        }

        /* Initialize interface */
        memset(iface, 0, sizeof(*iface));
        strlcpy(iface->name, ifa->ifa_name, sizeof(iface->name));
        iface->type = type;
        iface->fib = 0; /* Default FIB */
        iface->group_count = 0;
        iface->enabled = (ifa->ifa_flags & IFF_UP) != 0;
        iface->flags = ifa->ifa_flags;
        iface->mtu = 0; /* Will be set below */
        iface->bridge_members[0] = '\0'; /* Initialize bridge members to empty */

        /* Get additional interface information */
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock >= 0) {
            struct ifreq ifr;
            
            /* Get FIB */
            memset(&ifr, 0, sizeof(ifr));
            strlcpy(ifr.ifr_name, ifa->ifa_name, sizeof(ifr.ifr_name));
            if (ioctl(sock, SIOCGIFFIB, &ifr) == 0) {
                iface->fib = ifr.ifr_fib;
                debug_log(DEBUG_DEBUG, "Found FIB for %s: %u", ifa->ifa_name, iface->fib);
            } else {
                debug_log(DEBUG_DEBUG, "Failed to get FIB for %s: %s", ifa->ifa_name, strerror(errno));
            }
            
            /* Get MTU */
            memset(&ifr, 0, sizeof(ifr));
            strlcpy(ifr.ifr_name, ifa->ifa_name, sizeof(ifr.ifr_name));
            if (ioctl(sock, SIOCGIFMTU, &ifr) == 0) {
                iface->mtu = ifr.ifr_mtu;
                debug_log(DEBUG_DEBUG, "Found MTU for %s: %d", ifa->ifa_name, iface->mtu);
            } else {
                debug_log(DEBUG_DEBUG, "Failed to get MTU for %s: %s", ifa->ifa_name, strerror(errno));
            }
            
            /* Get group information */
            struct ifgroupreq ifgr;
            struct ifg_req *ifg;
            size_t len;
            
            memset(&ifgr, 0, sizeof(ifgr));
            strlcpy(ifgr.ifgr_name, ifa->ifa_name, IFNAMSIZ);
            
            /* First call to get the required buffer size */
            if (ioctl(sock, SIOCGIFGROUP, (caddr_t)&ifgr) == 0) {
                len = ifgr.ifgr_len;
                if (len > 0) {
                    /* Allocate memory for group list */
                    ifgr.ifgr_groups = (struct ifg_req *)calloc(len / sizeof(struct ifg_req), sizeof(struct ifg_req));
                    if (ifgr.ifgr_groups != NULL) {
                        /* Second call to get the actual group data */
                        if (ioctl(sock, SIOCGIFGROUP, (caddr_t)&ifgr) == 0) {
                            /* Process each group */
                            for (ifg = ifgr.ifgr_groups; ifg && len >= sizeof(*ifg); ifg++) {
                                len -= sizeof(*ifg);
                                if (iface->group_count < MAX_GROUPS_PER_IF) {
                                    strlcpy(iface->groups[iface->group_count], ifg->ifgrq_group, MAX_GROUP_NAME_LEN);
                                    iface->group_count++;
                                    debug_log(DEBUG_DEBUG, "Found group for %s: %s (total groups: %d)", ifa->ifa_name, ifg->ifgrq_group, iface->group_count);
                                }
                            }
                        } else {
                            debug_log(DEBUG_DEBUG, "Failed to get groups for %s: %s", ifa->ifa_name, strerror(errno));
                        }
                        free(ifgr.ifgr_groups);
                    }
                }
            } else {
                debug_log(DEBUG_DEBUG, "Failed to get group info for %s: %s", ifa->ifa_name, strerror(errno));
            }
            
            /* Get bridge member information for bridge interfaces */
            if (type == IF_TYPE_BRIDGE) {
                freebsd_get_bridge_members(ifa->ifa_name, iface->bridge_members, sizeof(iface->bridge_members));
            }
            
            close(sock);
        }
        
        /* Get all addresses */
        struct ifaddrs *ifa_addr;
        bool primary_ipv4_found = false;
        bool primary_ipv6_found = false;
        
        for (ifa_addr = ifap; ifa_addr; ifa_addr = ifa_addr->ifa_next) {
            if (strcmp(ifa_addr->ifa_name, ifa->ifa_name) == 0 && ifa_addr->ifa_addr) {
                if (ifa_addr->ifa_addr->sa_family == AF_INET) {
                    struct sockaddr_in *sin = (struct sockaddr_in *)ifa_addr->ifa_addr;
                    char addr_str[64];
                    inet_ntop(AF_INET, &sin->sin_addr, addr_str, sizeof(addr_str));
                    
                    if (!primary_ipv4_found) {
                        /* First IPv4 address is primary */
                        strncpy(iface->primary_address, addr_str, sizeof(iface->primary_address) - 1);
                        iface->primary_address[sizeof(iface->primary_address) - 1] = '\0';
                        primary_ipv4_found = true;
                        debug_log(DEBUG_DEBUG, "Found primary IPv4 address for %s: %s", ifa->ifa_name, iface->primary_address);
                    } else if (iface->alias_count < 10) {
                        /* Additional IPv4 addresses are aliases */
                        strncpy(iface->alias_addresses[iface->alias_count], addr_str, sizeof(iface->alias_addresses[0]) - 1);
                        iface->alias_addresses[iface->alias_count][sizeof(iface->alias_addresses[0]) - 1] = '\0';
                        iface->alias_count++;
                        debug_log(DEBUG_DEBUG, "Found IPv4 alias for %s: %s", ifa->ifa_name, addr_str);
                    }
                } else if (ifa_addr->ifa_addr->sa_family == AF_INET6) {
                    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ifa_addr->ifa_addr;
                    char addr_str[64];
                    inet_ntop(AF_INET6, &sin6->sin6_addr, addr_str, sizeof(addr_str));
                    
                    if (!primary_ipv6_found) {
                        /* First IPv6 address is primary */
                        strncpy(iface->primary_address6, addr_str, sizeof(iface->primary_address6) - 1);
                        iface->primary_address6[sizeof(iface->primary_address6) - 1] = '\0';
                        primary_ipv6_found = true;
                        debug_log(DEBUG_DEBUG, "Found primary IPv6 address for %s: %s", ifa->ifa_name, iface->primary_address6);
                    } else if (iface->alias_count6 < 10) {
                        /* Additional IPv6 addresses are aliases */
                        strncpy(iface->alias_addresses6[iface->alias_count6], addr_str, sizeof(iface->alias_addresses6[0]) - 1);
                        iface->alias_addresses6[iface->alias_count6][sizeof(iface->alias_addresses6[0]) - 1] = '\0';
                        iface->alias_count6++;
                        debug_log(DEBUG_DEBUG, "Found IPv6 alias for %s: %s", ifa->ifa_name, addr_str);
                    }
                }
            }
        }

        /* Add to interface list */
        TAILQ_INSERT_TAIL(&state->interfaces, iface, entries);
        
        if (strncmp(ifa->ifa_name, "bridge", 6) == 0) {
            debug_log(DEBUG_INFO, "Added bridge interface to list: %s", ifa->ifa_name);
        }
        
        debug_log(DEBUG_DEBUG, "Added system interface %s (type: %s, enabled: %s, fib: %u, mtu: %d, addr: %s, addr6: %s, groups: %d)", 
                  ifa->ifa_name, 
                  interface_type_to_string(type),
                  iface->enabled ? "yes" : "no",
                  iface->fib,
                  iface->mtu,
                  iface->primary_address[0] ? iface->primary_address : "none",
                  iface->primary_address6[0] ? iface->primary_address6 : "none",
                  iface->group_count);

        last_name = strdup(ifa->ifa_name);
    }

    freeifaddrs(ifap);
    if (last_name) {
        free(last_name);
    }

    debug_log(DEBUG_INFO, "System interface enumeration complete");
    return 0;
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
    
    if (!state) {
        return NULL;
    }

    /* Always enumerate from system to get fresh data */
    debug_log(DEBUG_DEBUG, "Enumerating system interfaces for XML generation");
    
    /* Clear existing interface list */
    interface_t *iface_next;
    TAILQ_FOREACH_SAFE(iface, &state->interfaces, entries, iface_next) {
        TAILQ_REMOVE(&state->interfaces, iface, entries);
        free(iface);
    }
    
    if (interface_enumerate_system(state) < 0) {
        debug_log(DEBUG_ERROR, "Failed to enumerate system interfaces");
        return NULL;
    }

    /* Start XML */
    asprintf(&xml, "    <interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">\n");
    if (!xml) {
        return NULL;
    }

    TAILQ_FOREACH(iface, &state->interfaces, entries) {
        if (strncmp(iface->name, "bridge", 6) == 0) {
            debug_log(DEBUG_INFO, "Generating XML for bridge interface: %s", iface->name);
        }
        /* Get VRF name */
        const char *vrf_name = "default";
        if (iface->fib != 0) {
            vrf_t *vrf = vrf_find_by_fib(state, iface->fib);
            if (vrf) {
                vrf_name = vrf->name;
            } else {
                /* VRF exists but no name configured, use ID */
                static char vrf_id_str[16];
                snprintf(vrf_id_str, sizeof(vrf_id_str), "%u", iface->fib);
                vrf_name = vrf_id_str;
            }
        }

        /* Generate interface XML */
        asprintf(&temp_xml,
                "      <interface>\n"
                "        <name>%s</name>\n"
                "        <type xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\">ianaift:%s</type>\n"
                "        <enabled>%s</enabled>\n"
                "        <vrf>%s</vrf>\n"
                "        <mtu>%d</mtu>\n"
                "        <flags>%d</flags>\n"
                "        <primary-address>%s</primary-address>\n"
                "        <primary-address6>%s</primary-address6>\n",
                iface->name,
                interface_type_to_string(iface->type),
                iface->enabled ? "true" : "false",
                vrf_name,
                iface->mtu,
                iface->flags,
                iface->primary_address[0] ? iface->primary_address : "",
                iface->primary_address6[0] ? iface->primary_address6 : "");

        if (temp_xml) {
            /* Append to main XML */
            char *new_xml;
            asprintf(&new_xml, "%s%s", xml, temp_xml);
            free(xml);
            free(temp_xml);
            xml = new_xml;
        }

        /* Add alias addresses if any */
        for (int i = 0; i < iface->alias_count; i++) {
            asprintf(&temp_xml, "        <alias>\n          <address>%s</address>\n        </alias>\n", 
                     iface->alias_addresses[i]);
            if (temp_xml) {
                char *new_xml;
                asprintf(&new_xml, "%s%s", xml, temp_xml);
                free(xml);
                free(temp_xml);
                xml = new_xml;
            }
        }
        
        for (int i = 0; i < iface->alias_count6; i++) {
            asprintf(&temp_xml, "        <alias>\n          <address6>%s</address6>\n        </alias>\n", 
                     iface->alias_addresses6[i]);
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
            asprintf(&temp_xml, "        <group>\n");
            if (temp_xml) {
                char *new_xml;
                asprintf(&new_xml, "%s%s", xml, temp_xml);
                free(xml);
                free(temp_xml);
                xml = new_xml;
            }

            for (int i = 0; i < iface->group_count; i++) {
                asprintf(&temp_xml, "          <group-name>%s</group-name>\n", iface->groups[i]);
                if (temp_xml) {
                    char *new_xml;
                    asprintf(&new_xml, "%s%s", xml, temp_xml);
                    free(xml);
                    free(temp_xml);
                    xml = new_xml;
                }
            }

            asprintf(&temp_xml, "        </group>\n");
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
            asprintf(&temp_xml, "        <bridge-members>%s</bridge-members>\n", iface->bridge_members);
            if (temp_xml) {
                char *new_xml;
                asprintf(&new_xml, "%s%s", xml, temp_xml);
                free(xml);
                free(temp_xml);
                xml = new_xml;
            }
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

    /* Validate the generated XML against YANG schema if YANG context is available */
    if (state->yang_ctx && xml) {
        if (yang_validate_xml(state, xml) < 0) {
            debug_log(DEBUG_WARN, "Generated interface XML failed YANG validation, but returning anyway");
            /* Don't fail the request, just log a warning */
        } else {
            debug_log(DEBUG_DEBUG, "Generated interface XML validated successfully against YANG schema");
        }
    }

    return xml;
} 