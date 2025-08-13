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
#include <system/freebsd/gif/gif.h>
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
 * Get GIF interface data from FreeBSD system
 * @param ctx libyang context
 * @param iface_name interface name
 * @param iface_node interface node to add GIF data to
 * @return 0 on success, -1 on failure
 */
int get_gif_data(struct ly_ctx *ctx, const char *iface_name, struct lyd_node *iface_node) {
    struct lyd_node *gif_node = NULL;
    int ret;
    
    /* Create GIF node */
    ret = lyd_new_inner(iface_node, ctx, "netd", "gif", 0, &gif_node);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create GIF node");
        return -1;
    }
    
    /* Get GIF data from system */
    char local_addr[64];
    char remote_addr[64];
    
    if (freebsd_gif_show(iface_name, local_addr, sizeof(local_addr), 
                        remote_addr, sizeof(remote_addr)) == 0) {
        /* Set local address */
        ret = lyd_new_term(gif_node, ctx, "netd", "local-addr", local_addr, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set GIF local address");
        }
        
        /* Set remote address */
        ret = lyd_new_term(gif_node, ctx, "netd", "remote-addr", remote_addr, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set GIF remote address");
        }
    }
    
    return 0;
}

/**
 * Set GIF tunnel endpoints
 * @param gif_name GIF interface name
 * @param local_addr local tunnel endpoint address
 * @param remote_addr remote tunnel endpoint address
 * @return 0 on success, -1 on failure
 */
int gif_set_endpoints(const char *gif_name, const char *local_addr, const char *remote_addr) {
    return freebsd_gif_set_tunnel(gif_name, local_addr, remote_addr);
} 