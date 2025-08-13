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

/* Global yang context - set by netconf.c */
extern struct ly_ctx *yang_ctx;

/**
 * Add route
 * @param fib FIB number (VRF)
 * @param destination destination network
 * @param gateway gateway address
 * @param interface interface name
 * @param flags route flags
 * @return 0 on success, -1 on failure
 */
int route_add(uint32_t fib, const char *destination, const char *gateway, 
              const char *interface, int flags) {
    return freebsd_route_add(fib, destination, gateway, interface, flags);
}

/**
 * Delete route
 * @param fib FIB number (VRF)
 * @param destination destination network
 * @return 0 on success, -1 on failure
 */
int route_delete(uint32_t fib, const char *destination) {
    return freebsd_route_delete(fib, destination);
}

/**
 * Handle edit-config RPC for routes
 * @param rpc RPC node
 * @param session NETCONF session
 * @return NETCONF reply
 */
struct nc_server_reply *handle_edit_config_routes(struct lyd_node *rpc, struct nc_session *session) {
    (void)session;
    
    struct lyd_node *config = NULL;
    struct lyd_node *route_node = NULL;
    const char *destination = NULL, *gateway = NULL, *interface = NULL;
    uint32_t fib = 0;
    int flags = 0;
    int ret;

    ret = lyd_find_path(rpc, "ietf-routing:routing", 0, &config);
    if (ret != LY_SUCCESS || !config) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_INVALID_VALUE, NULL));
    }

    ret = lyd_find_path(config, "ietf-routing:route", 0, &route_node);
    if (ret != LY_SUCCESS || !route_node) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_INVALID_VALUE, NULL));
    }

    struct lyd_node_term *term;
    lyd_find_path(route_node, "ietf-routing:destination-prefix", 0, (struct lyd_node **)&term);
    if (term) destination = lyd_get_value((struct lyd_node *)term);

    lyd_find_path(route_node, "ietf-routing:next-hop", 0, (struct lyd_node **)&term);
    if (term) gateway = lyd_get_value((struct lyd_node *)term);

    lyd_find_path(route_node, "ietf-routing:outgoing-interface", 0, (struct lyd_node **)&term);
    if (term) interface = lyd_get_value((struct lyd_node *)term);

    lyd_find_path(route_node, "ietf-routing:route-preference", 0, (struct lyd_node **)&term);
    if (term) {
        const char *flags_str = lyd_get_value((struct lyd_node *)term);
        if (flags_str) {
            flags = atoi(flags_str);
        }
    }

    if (!destination) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_INVALID_VALUE, NULL));
    }

    ret = route_add(fib, destination, gateway, interface, flags);
    if (ret != 0) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_OP_FAILED, NULL));
    }

    return nc_server_reply_ok();
} 