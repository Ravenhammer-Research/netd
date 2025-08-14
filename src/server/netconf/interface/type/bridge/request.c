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

#include <bridge.h>
#include <system/freebsd/bridge/bridge.h>
#include <types.h>
#include <netconf/netconf.h>
#include <netd.h>
#include <debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global yang context - set by netconf.c */
extern struct ly_ctx *yang_ctx;

/**
 * Get bridge interface data from FreeBSD system
 * @param iface_name interface name
 * @param bridge_data pointer to netd_bridge_t structure to populate
 * @return 0 on success, -1 on failure
 */
int get_bridge_data(const char *iface_name, netd_bridge_t *bridge_data) {
    if (!iface_name || !bridge_data) {
        debug_log(ERROR, "Invalid parameters for bridge data acquisition");
        return -1;
    }
    
    /* Get complete bridge data from system */
    if (freebsd_get_bridge_data(iface_name, bridge_data) != 0) {
        debug_log(ERROR, "Failed to get bridge data for %s", iface_name);
        return -1;
    }
    
    debug_log(DEBUG, "Successfully acquired bridge data for %s with %d members", 
              iface_name, bridge_data->members.count);
    return 0;
}

/**
 * Add member to bridge interface
 * @param bridge_name bridge interface name
 * @param member_name member interface name
 * @return 0 on success, -1 on failure
 */
int bridge_add_member(const char *bridge_name, const char *member_name) {
    return freebsd_bridge_add_member(bridge_name, member_name);
}

/**
 * Remove member from bridge interface
 * @param bridge_name bridge interface name
 * @param member_name member interface name
 * @return 0 on success, -1 on failure
 */
int bridge_remove_member(const char *bridge_name, const char *member_name) {
    return freebsd_bridge_remove_member(bridge_name, member_name);
}

/**
 * Get bridge members
 * @param bridge_name bridge interface name
 * @param members array to store member names
 * @param max_members maximum number of members to retrieve
 * @return number of members on success, -1 on failure
 */
int bridge_get_members(const char *bridge_name, char **members, int max_members) {
    netd_bridge_members_t bridge_members;
    int member_count = 0;
    
    if (freebsd_get_bridge_members(bridge_name, &bridge_members) != 0) {
        return -1;
    }
    
    /* Convert TAILQ to array */
    netd_bridge_member_t *member;
    TAILQ_FOREACH(member, &bridge_members.head, entries) {
        if (member_count >= max_members) {
            break;
        }
        
        /* For now, we'll use a placeholder name since the interface pointer is NULL */
        const char *member_name = member->interface ? member->interface->name : "unknown";
        members[member_count] = strdup(member_name);
        member_count++;
    }
    
    /* Clean up the TAILQ structure */
    netd_bridge_members_clear(&bridge_members);
    
    return member_count;
}
