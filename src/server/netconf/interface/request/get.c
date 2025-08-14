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

extern struct ly_ctx *yang_ctx;
extern netd_state_t netd_state;

/* Get the module for ietf-interfaces */
static const struct lys_module *get_ietf_interfaces_module(struct ly_ctx *ctx) {
    return ly_ctx_get_module(ctx, "ietf-interfaces", NULL);
}

/* Convert interface type to IANA if-type string */
static const char *interface_type_to_iana(netd_interface_type_t type) {
    switch (type) {
        case NETD_IF_TYPE_ETHERNET:
            return "iana-if-type:ethernetCsmacd";
        case NETD_IF_TYPE_WIRELESS:
            return "iana-if-type:ieee80211";
        case NETD_IF_TYPE_EPAIR:
            return "iana-if-type:ethernetCsmacd";
        case NETD_IF_TYPE_GIF:
            return "iana-if-type:tunnel";
        case NETD_IF_TYPE_GRE:
            return "iana-if-type:tunnel";
        case NETD_IF_TYPE_LAGG:
            return "iana-if-type:ieee8023adLag";
        case NETD_IF_TYPE_LOOPBACK:
            return "iana-if-type:softwareLoopback";
        case NETD_IF_TYPE_OVPN:
            return "iana-if-type:tunnel";
        case NETD_IF_TYPE_TUN:
            return "iana-if-type:tunnel";
        case NETD_IF_TYPE_TAP:
            return "iana-if-type:ethernetCsmacd";
        case NETD_IF_TYPE_VLAN:
            return "iana-if-type:l2vlan";
        case NETD_IF_TYPE_VXLAN:
            return "iana-if-type:tunnel";
        case NETD_IF_TYPE_BRIDGE:
            return "iana-if-type:bridge";
        case NETD_IF_TYPE_UNKNOWN:
        default:
            return "iana-if-type:other";
    }
}

struct lyd_node *get_interface_data(struct ly_ctx *ctx) {
    struct lyd_node *interfaces = NULL;
    struct lyd_node *iface = NULL;
    int ret;
    const struct lys_module *ietf_interfaces_mod;
    netd_interface_t *interface;
    netd_system_query_t system_query = {0};

    /* Get the ietf-interfaces module */
    ietf_interfaces_mod = get_ietf_interfaces_module(ctx);
    if (!ietf_interfaces_mod) {
        debug_log(ERROR, "ietf-interfaces module not found");
        return NULL;
    }

    /* Call system layer to enumerate interfaces */
    if (freebsd_enumerate_interfaces(&system_query) != 0) {
        debug_log(ERROR, "Failed to enumerate interfaces");
        return NULL;
    }

    /* Create interfaces container */
    ret = lyd_new_inner(NULL, ietf_interfaces_mod, "interfaces", 0, &interfaces);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create interfaces container");
        return NULL;
    }

    /* Iterate through interfaces from system query */
    TAILQ_FOREACH(interface, &system_query.interfaces, entries) {
        /* Create interface node */
        ret = lyd_new_inner(interfaces, ietf_interfaces_mod, "interface", 0, &iface);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to create interface node");
            continue;
        }

        /* Set interface name */
        ret = lyd_new_term(iface, ietf_interfaces_mod, "name", interface->name, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set interface name");
            continue;
        }

        /* Set interface type from system layer */
        const char *type = interface_type_to_iana(interface->type);
        ret = lyd_new_term(iface, ietf_interfaces_mod, "type", type, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set interface type");
            continue;
        }

        /* Set enabled state from system layer - check if interface is up */
        ret = lyd_new_term(iface, ietf_interfaces_mod, "enabled",
                          (interface->flags & NETD_IF_FLAG_UP) ? "true" : "false", 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set interface enabled state");
            continue;
        }

        /* Handle type-specific data using the new request/response pattern */
        if (interface->type == NETD_IF_TYPE_BRIDGE) {
            /* Get bridge-specific data using request handler */
            netd_bridge_t bridge_data;
            if (get_bridge_data(interface->name, &bridge_data) == 0) {
                /* Create bridge-specific response using response handler */
                struct lyd_node *bridge_response = create_bridge_response(ctx, &bridge_data);
                if (bridge_response) {
                    /* Merge the bridge-specific data into the interface node */
                    /* Note: This is a simplified example - in practice you'd need to merge the nodes properly */
                    debug_log(DEBUG, "Successfully added bridge-specific data for %s", interface->name);
                }
                /* Clean up bridge data */
                netd_bridge_members_clear(&bridge_data.members);
            }
        }
        /* Add similar blocks for other interface types (VLAN, LAGG, etc.) */
    }

    return interfaces;
}

/**
 * Handle get-config RPC for interfaces
 * @param rpc RPC node
 * @param session NETCONF session
 * @return NETCONF reply
 */
struct nc_server_reply *handle_get_config_interfaces(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    (void)session;

    struct lyd_node *data = get_interface_data(yang_ctx);
    if (!data) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_OP_FAILED, NULL));
    }

    return nc_server_reply_data(data, NC_WD_UNKNOWN, NC_PARAMTYPE_FREE);
} 