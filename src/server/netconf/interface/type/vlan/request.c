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

#include <vlan.h>
#include <system/freebsd/vlan/vlan.h>
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
 * Get VLAN interface data from FreeBSD system
 * @param ctx libyang context
 * @param iface_name interface name
 * @param iface_node interface node to add VLAN data to
 * @return 0 on success, -1 on failure
 */
int get_vlan_data(struct ly_ctx *ctx, const char *iface_name, struct lyd_node *iface_node) {
    struct lyd_node *vlan_node = NULL;
    int ret;
    
    /* Create VLAN node */
    ret = lyd_new_inner(iface_node, ctx, "netd", "vlan", 0, &vlan_node);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create VLAN node");
        return -1;
    }
    
    /* Get VLAN data from system */
    int vlan_id;
    char vlan_proto[32];
    int vlan_pcp;
    char vlan_parent[MAX_IFNAME_LEN];
    
    if (freebsd_vlan_show(iface_name, &vlan_id, vlan_proto, sizeof(vlan_proto), 
                         &vlan_pcp, vlan_parent, sizeof(vlan_parent)) == 0) {
        /* Set VLAN ID */
        char vlan_id_str[16];
        snprintf(vlan_id_str, sizeof(vlan_id_str), "%d", vlan_id);
        ret = lyd_new_term(vlan_node, ctx, "netd", "vlan-id", vlan_id_str, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set VLAN ID");
        }
        
        /* Set VLAN parent */
        ret = lyd_new_term(vlan_node, ctx, "netd", "parent", vlan_parent, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set VLAN parent");
        }
        
        /* Set VLAN protocol */
        ret = lyd_new_term(vlan_node, ctx, "netd", "protocol", vlan_proto, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set VLAN protocol");
        }
        
        /* Set VLAN priority */
        char vlan_pcp_str[8];
        snprintf(vlan_pcp_str, sizeof(vlan_pcp_str), "%d", vlan_pcp);
        ret = lyd_new_term(vlan_node, ctx, "netd", "priority", vlan_pcp_str, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set VLAN priority");
        }
    }
    
    return 0;
}

/**
 * Create VLAN interface
 * @param vlan_name VLAN interface name
 * @param parent_name parent interface name
 * @param vlan_id VLAN ID
 * @return 0 on success, -1 on failure
 */
int vlan_create(const char *vlan_name, const char *parent_name, int vlan_id) {
    return freebsd_vlan_create(vlan_name, parent_name, vlan_id);
}

/**
 * Delete VLAN interface
 * @param vlan_name VLAN interface name
 * @return 0 on success, -1 on failure
 */
int vlan_delete(const char *vlan_name) {
    return freebsd_vlan_delete(vlan_name);
} 