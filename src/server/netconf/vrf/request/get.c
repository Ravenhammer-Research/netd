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

#include <request.h>
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
extern netd_state_t netd_state;

/**
 * Get VRF data from netd state
 * @param ctx libyang context
 * @return libyang data tree with VRF data, NULL on error
 */
static const struct lys_module *get_ietf_routing_module(struct ly_ctx *ctx) {
    return ly_ctx_get_module(ctx, "ietf-routing", NULL);
}

struct lyd_node *get_vrf_data(struct ly_ctx *ctx) {
    struct lyd_node *vrfs = NULL;
    int ret;
    const struct lys_module *routing_module = get_ietf_routing_module(ctx);
    
    /* Create VRFs container */
    ret = lyd_new_inner(NULL, routing_module, "routing-instance", 0, &vrfs);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create routing-instance container");
        return NULL;
    }
    
    netd_vrf_t *vrf;
    TAILQ_FOREACH(vrf, &netd_state.vrfs, entries) {
        struct lyd_node *vrf_node = NULL;
        ret = lyd_new_inner(vrfs, routing_module, "routing-instance", 0, &vrf_node);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to create routing-instance node");
            continue;
        }

        lyd_new_term(vrf_node, routing_module, "name", vrf->name, 0, NULL);
        lyd_new_term(vrf_node, routing_module, "type", "l3vpn:l3vpn", 0, NULL);
        lyd_new_term(vrf_node, routing_module, "enabled", "true", 0, NULL);
    }
    
    return vrfs;
}

/**
 * Handle get-config RPC for VRFs
 * @param rpc RPC node
 * @param session NETCONF session
 * @return NETCONF reply
 */
struct nc_server_reply *handle_get_config_vrfs(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    (void)session;

    struct lyd_node *data = get_vrf_data(yang_ctx);
    if (!data) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_OP_FAILED, NULL));
    }

    return nc_server_reply_data(data, NC_WD_UNKNOWN, NC_PARAMTYPE_FREE);
} 