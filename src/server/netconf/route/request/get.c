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
#include <system/freebsd/route/route.h>
#include <netconf/netconf.h>
#include <netd.h>
#include <debug.h>
#include <libyang/tree_data.h>
#include <libyang/tree_schema.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

/* Global yang context - set by netconf.c */
extern struct ly_ctx *yang_ctx;

/* Get the module for ietf-routing */
static const struct lys_module *get_ietf_routing_module(struct ly_ctx *ctx) {
    return ly_ctx_get_module(ctx, "ietf-routing", NULL);
}

/**
 * Get route data from FreeBSD system
 * @param ctx libyang context
 * @param fib FIB number (VRF)
 * @return libyang data tree with route data, NULL on error
 */
struct lyd_node *get_route_data(struct ly_ctx *ctx, uint32_t fib) {
    struct lyd_node *routes = NULL;
    netd_state_t state = {0};
    int ret;
    const struct lys_module *ietf_routing_mod;
    
    /* Get the ietf-routing module */
    ietf_routing_mod = get_ietf_routing_module(ctx);
    if (!ietf_routing_mod) {
        debug_log(ERROR, "ietf-routing module not found");
        return NULL;
    }
    
    /* Create routes container */
    ret = lyd_new_inner(NULL, ietf_routing_mod, "routing", 0, &routes);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create routing container");
        return NULL;
    }

    /* Get route data from system */
    ret = freebsd_route_enumerate_system(&state, fib);
    if (ret != 0) {
        debug_log(ERROR, "Failed to enumerate routes for FIB %u", fib);
        lyd_free_tree(routes);
        return NULL;
    }

    /* Convert netd routes to libyang data */
    netd_route_t *route;
    TAILQ_FOREACH(route, &state.routes, entries) {
        struct lyd_node *route_node = NULL;
        char dest_str[INET6_ADDRSTRLEN];
        char gw_str[INET6_ADDRSTRLEN];
        char flags_str[16];
        
        /* Convert sockaddr to string representation */
        if (route->destination.ss_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)&route->destination;
            inet_ntop(AF_INET, &sa->sin_addr, dest_str, sizeof(dest_str));
        } else if (route->destination.ss_family == AF_INET6) {
            struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&route->destination;
            inet_ntop(AF_INET6, &sa->sin6_addr, dest_str, sizeof(dest_str));
        }

        if (route->gateway.ss_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)&route->gateway;
            inet_ntop(AF_INET, &sa->sin_addr, gw_str, sizeof(gw_str));
        } else if (route->gateway.ss_family == AF_INET6) {
            struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&route->gateway;
            inet_ntop(AF_INET6, &sa->sin6_addr, gw_str, sizeof(gw_str));
        }

        /* Convert flags to string */
        snprintf(flags_str, sizeof(flags_str), "%d", route->flags);

        /* Create route node */
        ret = lyd_new_inner(routes, ietf_routing_mod, "route", 0, &route_node);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to create route node");
            continue;
        }

        /* Add route attributes */
        lyd_new_term(route_node, ietf_routing_mod, "destination-prefix", dest_str, 0, NULL);
        lyd_new_term(route_node, ietf_routing_mod, "next-hop", gw_str, 0, NULL);
        lyd_new_term(route_node, ietf_routing_mod, "outgoing-interface", route->interface, 0, NULL);
        lyd_new_term(route_node, ietf_routing_mod, "route-preference", flags_str, 0, NULL);
    }

    return routes;
}

/**
 * Handle get-config RPC for routes
 * @param rpc RPC node
 * @param session NETCONF session
 * @return NETCONF reply
 */
struct nc_server_reply *handle_get_config_routes(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    (void)session;

    struct lyd_node *data = get_route_data(yang_ctx, 0); /* Default FIB 0 */
    if (!data) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_OP_FAILED, NULL));
    }

    return nc_server_reply_data(data, NC_WD_UNKNOWN, NC_PARAMTYPE_FREE);
} 