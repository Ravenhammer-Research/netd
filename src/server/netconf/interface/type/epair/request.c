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

#include <epair.h>
#include <system/freebsd/epair/epair.h>
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
 * Get EPAIR interface data from FreeBSD system
 * @param ctx libyang context
 * @param iface_name interface name
 * @param iface_node interface node to add EPAIR data to
 * @return 0 on success, -1 on failure
 */
int get_epair_data(struct ly_ctx *ctx, const char *iface_name, struct lyd_node *iface_node) {
    struct lyd_node *epair_node = NULL;
    int ret;
    
    /* Create EPAIR node */
    ret = lyd_new_inner(iface_node, ctx, "netd", "epair", 0, &epair_node);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create EPAIR node");
        return -1;
    }
    
    /* Get EPAIR data from system */
    char peer_name[MAX_IFNAME_LEN];
    
    if (freebsd_epair_show(iface_name, peer_name, sizeof(peer_name)) == 0) {
        /* Set peer name */
        ret = lyd_new_term(epair_node, ctx, "netd", "peer", peer_name, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set EPAIR peer name");
        }
    }
    
    return 0;
}

/**
 * Create EPAIR interface
 * @param epair_name EPAIR interface name
 * @return 0 on success, -1 on failure
 */
int epair_create(const char *epair_name) {
    return freebsd_epair_create(epair_name);
}

/**
 * Delete EPAIR interface
 * @param epair_name EPAIR interface name
 * @return 0 on success, -1 on failure
 */
int epair_delete(const char *epair_name) {
    return freebsd_epair_delete(epair_name);
} 