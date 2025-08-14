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
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

/**
 * Get route data from FreeBSD system
 * @param fib FIB number (VRF)
 * @param system_query pointer to netd_system_query_t structure to populate
 * @return 0 on success, -1 on failure
 */
int get_route_data(uint32_t fib, netd_system_query_t *system_query) {
    if (!system_query) {
        debug_log(ERROR, "Invalid parameters for route data acquisition");
        return -1;
    }
    
    /* Initialize the system query structure */
    memset(system_query, 0, sizeof(netd_system_query_t));
    
    /* Get route data from system */
    int ret = freebsd_route_enumerate_system(system_query, fib);
    if (ret != 0) {
        debug_log(ERROR, "Failed to enumerate routes for FIB %u", fib);
        return -1;
    }
    
    debug_log(DEBUG, "Successfully acquired route data for FIB %u with %d routes", 
              fib, system_query->routes.count);
    return 0;
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

    netd_system_query_t system_query = {0};
    if (get_route_data(0, &system_query) != 0) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_OP_FAILED, NULL));
    }

    return nc_server_reply_data(NULL, NC_WD_UNKNOWN, NC_PARAMTYPE_FREE);
} 