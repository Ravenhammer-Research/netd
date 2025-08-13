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
#include <libyang/tree_data.h>
#include <libyang/tree_schema.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global yang context - set by netconf.c */
extern struct ly_ctx *yang_ctx;

/* Get the module for ietf-netconf */
static const struct lys_module *get_ietf_netconf_module(struct ly_ctx *ctx) {
    return ly_ctx_get_module(ctx, "ietf-netconf", NULL);
}

/**
 * Create interface set response data
 * @param ctx libyang context
 * @param iface_name interface name
 * @param success operation success status
 * @return libyang data tree with interface set response data, NULL on error
 */
struct lyd_node *create_interface_set_response(struct ly_ctx *ctx, const char *iface_name, bool success) {
    struct lyd_node *response = NULL;
    struct lyd_node *error = NULL;
    int ret;
    const struct lys_module *ietf_netconf_mod;

    /* Get the ietf-netconf module */
    ietf_netconf_mod = get_ietf_netconf_module(ctx);
    if (!ietf_netconf_mod) {
        debug_log(ERROR, "ietf-netconf module not found");
        return NULL;
    }

    ret = lyd_new_inner(NULL, ietf_netconf_mod, "rpc-reply", 0, &response);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create rpc-reply container");
        return NULL;
    }

    if (success) {
        lyd_new_term(response, ietf_netconf_mod, "ok", NULL, 0, NULL);
    } else {
        ret = lyd_new_inner(response, ietf_netconf_mod, "rpc-error", 0, &error);
        if (ret == LY_SUCCESS) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Interface configuration failed for %s", iface_name);
            
            lyd_new_term(error, ietf_netconf_mod, "error-type", "application", 0, NULL);
            lyd_new_term(error, ietf_netconf_mod, "error-tag", "operation-failed", 0, NULL);
            lyd_new_term(error, ietf_netconf_mod, "error-severity", "error", 0, NULL);
            lyd_new_term(error, ietf_netconf_mod, "error-message", error_msg, 0, NULL);
        }
    }

    return response;
} 