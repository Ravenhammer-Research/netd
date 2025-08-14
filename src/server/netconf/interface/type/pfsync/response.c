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

#include <pfsync.h>
#include <netconf/netconf.h>
#include <netd.h>
#include <debug.h>
#include <types.h>
#include <libyang/tree_data.h>
#include <libyang/tree_schema.h>
#include <libyang/context.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Create PFSYNC response data from populated PFSYNC structure
 * @param ctx libyang context
 * @param pfsync_data populated netd_pfsync_t structure
 * @return libyang data tree with PFSYNC response data, NULL on error
 */
struct lyd_node *create_pfsync_response(struct ly_ctx *ctx, const netd_pfsync_t *pfsync_data) {
    struct lyd_node *response = NULL;
    struct lyd_node *iface_node = NULL;
    struct lyd_node *pfsync_node = NULL;
    const struct lys_module *ietf_interfaces_mod;
    const struct lys_module *netd_mod;
    int ret;

    if (!pfsync_data) {
        debug_log(ERROR, "Invalid PFSYNC data for response creation");
        return NULL;
    }

    /* Get the required modules */
    ietf_interfaces_mod = ly_ctx_get_module_implemented(ctx, "ietf-interfaces");
    if (!ietf_interfaces_mod) {
        debug_log(ERROR, "Failed to get ietf-interfaces module");
        return NULL;
    }
    
    netd_mod = ly_ctx_get_module_implemented(ctx, "netd");
    if (!netd_mod) {
        debug_log(ERROR, "Failed to get netd module");
        return NULL;
    }

    /* Create interfaces container */
    ret = lyd_new_inner(NULL, ietf_interfaces_mod, "interfaces", 0, &response);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create interfaces response container");
        return NULL;
    }

    /* Create interface node */
    ret = lyd_new_inner(response, ietf_interfaces_mod, "interface", 0, &iface_node);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create interface node");
        return NULL;
    }

    /* Set interface basic properties */
    ret = lyd_new_term(iface_node, ietf_interfaces_mod, "name", pfsync_data->base.name, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set interface name");
    }
    
    ret = lyd_new_term(iface_node, ietf_interfaces_mod, "type", "iana-if-type:softwareLoopback", 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set interface type");
    }
    
    ret = lyd_new_term(iface_node, ietf_interfaces_mod, "enabled", "true", 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set interface enabled");
    }
    
    ret = lyd_new_term(iface_node, ietf_interfaces_mod, "oper-status", "up", 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set interface oper-status");
    }

    /* Create PFSYNC-specific node */
    ret = lyd_new_inner(iface_node, netd_mod, "pfsync", 0, &pfsync_node);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create PFSYNC node");
        return response; /* Return partial response */
    }

    /* Add PFSYNC-specific properties */
    char enabled_str[8];
    snprintf(enabled_str, sizeof(enabled_str), "%d", pfsync_data->enabled);
    ret = lyd_new_term(pfsync_node, netd_mod, "enabled", enabled_str, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set PFSYNC enabled");
    }

    char sync_peer_str[16];
    snprintf(sync_peer_str, sizeof(sync_peer_str), "%d", pfsync_data->sync_peer);
    ret = lyd_new_term(pfsync_node, netd_mod, "sync-peer", sync_peer_str, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set PFSYNC sync peer");
    }

    return response;
}
