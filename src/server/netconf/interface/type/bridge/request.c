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
#include <netconf/netconf.h>
#include <netd.h>
#include <debug.h>
#include <libyang/tree_data.h>
#include <libyang/tree_schema.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global yang context - set by netconf.c */
extern struct ly_ctx *yang_ctx;

/**
 * Get bridge interface data from FreeBSD system
 * @param ctx libyang context
 * @param iface_name interface name
 * @param iface_node interface node to add bridge data to
 * @return 0 on success, -1 on failure
 */
int get_bridge_data(struct ly_ctx *ctx, const char *iface_name, struct lyd_node *iface_node) {
    struct lyd_node *bridge_node = NULL;
    int ret;
    
    /* Create bridge node */
    ret = lyd_new_inner(iface_node, ctx, "netd", "bridge", 0, &bridge_node);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create bridge node");
        return -1;
    }
    
    /* Get bridge members from system */
    char members[1024];
    if (freebsd_get_bridge_members(iface_name, members, sizeof(members)) == 0) {
        /* Parse members and add to bridge node */
        char *member = strtok(members, " ");
        while (member != NULL) {
            struct lyd_node *member_node = NULL;
            ret = lyd_new_inner(bridge_node, ctx, "netd", "member", 0, &member_node);
            if (ret != LY_SUCCESS) {
                debug_log(ERROR, "Failed to create bridge member node");
                break;
            }
            
            ret = lyd_new_term(member_node, ctx, "netd", "name", member, 0, NULL);
            if (ret != LY_SUCCESS) {
                debug_log(ERROR, "Failed to set bridge member name");
            }
            
            member = strtok(NULL, " ");
        }
    }
    
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
    char member_array[MAX_IFNAME_LEN][max_members];
    int member_count;
    
    if (freebsd_get_bridge_members_array(bridge_name, member_array, max_members, &member_count) != 0) {
        return -1;
    }
    
    for (int i = 0; i < member_count; i++) {
        members[i] = strdup(member_array[i]);
    }
    
    return member_count;
}
