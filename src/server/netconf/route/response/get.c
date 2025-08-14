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
#include <arpa/inet.h>

/**
 * Create route response data from populated system query structure
 * @param ctx libyang context
 * @param system_query populated netd_system_query_t structure
 * @return libyang data tree with route response data, NULL on error
 */
struct lyd_node *create_route_response(struct ly_ctx *ctx, const netd_system_query_t *system_query) {
    struct lyd_node *routes = NULL;
    const struct lys_module *ietf_routing_mod;
    int ret;

    if (!system_query) {
        debug_log(ERROR, "Invalid system query data for route response creation");
        return NULL;
    }

    /* Get the ietf-routing module */
    ietf_routing_mod = ly_ctx_get_module_implemented(ctx, "ietf-routing");
    if (!ietf_routing_mod) {
        debug_log(ERROR, "Failed to get ietf-routing module");
        return NULL;
    }

    /* Create routes container */
    ret = lyd_new_inner(NULL, ietf_routing_mod, "routing", 0, &routes);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create routing container");
        return NULL;
    }

    /* Convert netd routes to libyang data */
    netd_route_t *route;
    TAILQ_FOREACH(route, &system_query->routes, entries) {
        struct lyd_node *route_node = NULL;
        char dest_str[INET6_ADDRSTRLEN];
        char gw_str[INET6_ADDRSTRLEN];
        char flags_str[16];
        
        /* Convert netd_address to string representation */
        if (route->destination.family == AF_INET) {
            inet_ntop(AF_INET, route->destination.address, dest_str, sizeof(dest_str));
        } else if (route->destination.family == AF_INET6) {
            inet_ntop(AF_INET6, route->destination.address, dest_str, sizeof(dest_str));
        } else {
            strlcpy(dest_str, "unknown", sizeof(dest_str));
        }

        if (route->gateway.family == AF_INET) {
            inet_ntop(AF_INET, route->gateway.address, gw_str, sizeof(gw_str));
        } else if (route->gateway.family == AF_INET6) {
            inet_ntop(AF_INET6, route->gateway.address, gw_str, sizeof(gw_str));
        } else {
            strlcpy(gw_str, "unknown", sizeof(gw_str));
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
        ret = lyd_new_term(route_node, ietf_routing_mod, "destination-prefix", dest_str, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set destination-prefix");
        }

        ret = lyd_new_term(route_node, ietf_routing_mod, "next-hop", gw_str, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set next-hop");
        }

        if (route->interface) {
            ret = lyd_new_term(route_node, ietf_routing_mod, "outgoing-interface", route->interface->name, 0, NULL);
            if (ret != LY_SUCCESS) {
                debug_log(ERROR, "Failed to set outgoing-interface");
            }
        }

        ret = lyd_new_term(route_node, ietf_routing_mod, "route-preference", flags_str, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set route-preference");
        }
    }

    return routes;
} 