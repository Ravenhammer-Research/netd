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
#include <system/freebsd/interface/interface.h>
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
 * Set interface enabled state
 * @param iface_name interface name
 * @param enabled enabled state
 * @return 0 on success, -1 on failure
 */
int set_interface_enabled(const char *iface_name, bool enabled) {
    debug_log(INFO, "Setting interface %s enabled=%d", iface_name, enabled);
    return 0;
}

/**
 * Set interface description
 * @param iface_name interface name
 * @param description interface description
 * @return 0 on success, -1 on failure
 */
int set_interface_description(const char *iface_name, const char *description) {
    debug_log(INFO, "Setting interface %s description=%s", iface_name, description);
    return 0;
}

/**
 * Set interface FIB (VRF)
 * @param iface_name interface name
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
int set_interface_fib(const char *iface_name, uint32_t fib) {
    return freebsd_interface_set_fib(iface_name, fib);
}

/**
 * Set interface MTU
 * @param iface_name interface name
 * @param mtu MTU value
 * @return 0 on success, -1 on failure
 */
int set_interface_mtu(const char *iface_name, int mtu) {
    return freebsd_interface_set_mtu(iface_name, mtu);
}

/**
 * Handle edit-config RPC for interfaces
 * @param rpc RPC node
 * @param session NETCONF session
 * @return NETCONF reply
 */
struct nc_server_reply *handle_edit_config_interfaces(struct lyd_node *rpc, struct nc_session *session) {
    (void)session;
    
    struct lyd_node *config = NULL;
    struct lyd_node *iface_node = NULL;
    const char *iface_name = NULL;
    int ret;

    ret = lyd_find_path(rpc, "ietf-interfaces:interfaces", 0, &config);
    if (ret != LY_SUCCESS || !config) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_INVALID_VALUE, NULL));
    }

    ret = lyd_find_path(config, "ietf-interfaces:interface", 0, &iface_node);
    if (ret != LY_SUCCESS || !iface_node) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_INVALID_VALUE, NULL));
    }

    struct lyd_node_term *term;
    lyd_find_path(iface_node, "ietf-interfaces:name", 0, (struct lyd_node **)&term);
    if (term) iface_name = lyd_get_value((struct lyd_node *)term);

    if (!iface_name) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_INVALID_VALUE, NULL));
    }

    lyd_find_path(iface_node, "ietf-interfaces:enabled", 0, (struct lyd_node **)&term);
    if (term) {
        const char *enabled_str = lyd_get_value((struct lyd_node *)term);
        bool enabled = (enabled_str && strcmp(enabled_str, "true") == 0);
        ret = set_interface_enabled(iface_name, enabled);
        if (ret != 0) {
            return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_OP_FAILED, NULL));
        }
    }

    lyd_find_path(iface_node, "ietf-interfaces:description", 0, (struct lyd_node **)&term);
    if (term) {
        const char *description = lyd_get_value((struct lyd_node *)term);
        ret = set_interface_description(iface_name, description);
        if (ret != 0) {
            return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_OP_FAILED, NULL));
        }
    }

    lyd_find_path(iface_node, "ietf-ip:ipv4/ietf-ip:address", 0, (struct lyd_node **)&term);
    if (term) {
        const char *address = lyd_get_value((struct lyd_node *)term);
        debug_log(INFO, "Setting interface %s IPv4 address %s", iface_name, address);
    }

    lyd_find_path(iface_node, "ietf-ip:ipv6/ietf-ip:address", 0, (struct lyd_node **)&term);
    if (term) {
        const char *address = lyd_get_value((struct lyd_node *)term);
        debug_log(INFO, "Setting interface %s IPv6 address %s", iface_name, address);
    }

    return nc_server_reply_ok();
} 