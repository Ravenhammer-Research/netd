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

#include <gif.h>
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
 * Create GIF response data
 * @param ctx libyang context
 * @param gif_name GIF interface name
 * @return libyang data tree with GIF response data, NULL on error
 */
struct lyd_node *create_gif_response(struct ly_ctx *ctx, const char *gif_name) {
    struct lyd_node *response = NULL;
    int ret;

    ret = lyd_new_inner(NULL, ctx, "ietf-interfaces", "interfaces", 0, &response);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create interfaces response container");
        return NULL;
    }

    struct lyd_node *iface_node = NULL;
    ret = lyd_new_inner(response, ctx, "ietf-interfaces", "interface", 0, &iface_node);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create interface node");
        return NULL;
    }

    lyd_new_term(iface_node, ctx, "ietf-interfaces", "name", gif_name, 0, NULL);
    lyd_new_term(iface_node, ctx, "ietf-interfaces", "type", "iana-if-type:tunnel", 0, NULL);
    lyd_new_term(iface_node, ctx, "ietf-interfaces", "enabled", 1, 0, NULL);
    lyd_new_term(iface_node, ctx, "ietf-interfaces", "oper-status", "up", 0, NULL);

    return response;
} 