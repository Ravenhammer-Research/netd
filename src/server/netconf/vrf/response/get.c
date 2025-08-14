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

#include <response.h>
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
 * Create VRF response data from populated system query structure
 * @param ctx libyang context
 * @param system_query populated netd_system_query_t structure
 * @return libyang data tree with VRF response data, NULL on error
 */
struct lyd_node *create_vrf_response(struct ly_ctx *ctx, const netd_system_query_t *system_query) {
    struct lyd_node *vrfs = NULL;
    const struct lys_module *ietf_routing_mod;
    int ret;

    if (!system_query) {
        debug_log(ERROR, "Invalid system query data for VRF response creation");
        return NULL;
    }

    /* Get the ietf-routing module */
    ietf_routing_mod = ly_ctx_get_module_implemented(ctx, "ietf-routing");
    if (!ietf_routing_mod) {
        debug_log(ERROR, "Failed to get ietf-routing module");
        return NULL;
    }

    /* Create VRFs container */
    ret = lyd_new_inner(NULL, ietf_routing_mod, "routing-instance", 0, &vrfs);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create routing-instance container");
        return NULL;
    }

    /* Convert netd VRFs to libyang data */
    netd_vrf_t *vrf;
    TAILQ_FOREACH(vrf, &system_query->vrfs, entries) {
        struct lyd_node *vrf_node = NULL;
        
        /* Create VRF node */
        ret = lyd_new_inner(vrfs, ietf_routing_mod, "routing-instance", 0, &vrf_node);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to create routing-instance node");
            continue;
        }

        /* Add VRF attributes */
        ret = lyd_new_term(vrf_node, ietf_routing_mod, "name", vrf->name, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set VRF name");
        }

        ret = lyd_new_term(vrf_node, ietf_routing_mod, "type", "l3vpn:l3vpn", 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set VRF type");
        }

        ret = lyd_new_term(vrf_node, ietf_routing_mod, "enabled", "true", 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set VRF enabled");
        }
    }

    return vrfs;
} 