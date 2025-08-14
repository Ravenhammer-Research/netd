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

#include <80211.h>
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

/**
 * Create WiFi response data from populated WiFi structure
 * @param ctx libyang context
 * @param wifi_data populated netd_wifi_t structure
 * @return libyang data tree with WiFi response data, NULL on error
 */
struct lyd_node *create_wifi_response(struct ly_ctx *ctx, const netd_wifi_t *wifi_data) {
    struct lyd_node *response = NULL;
    struct lyd_node *iface_node = NULL;
    struct lyd_node *wifi_node = NULL;
    const struct lys_module *ietf_interfaces_mod;
    const struct lys_module *netd_mod;
    int ret;

    if (!wifi_data) {
        debug_log(ERROR, "Invalid WiFi data for response creation");
        return NULL;
    }

    /* Get the required modules */
    ietf_interfaces_mod = ly_ctx_get_module_implemented(ctx, "ietf-interfaces");
    if (!ietf_interfaces_mod) {
        debug_log(ERROR, "Failed to get ietf-interfaces module");
        return NULL;
    }
    
    netd_mod = ly_ctx_get_module_implemented(ctx, "netd");
    if (!netd_mod) {
        debug_log(ERROR, "Failed to get netd module");
        return NULL;
    }

    /* Create interfaces container */
    ret = lyd_new_inner(NULL, ietf_interfaces_mod, "interfaces", 0, &response);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create interfaces response container");
        return NULL;
    }

    /* Create interface node */
    ret = lyd_new_inner(response, ietf_interfaces_mod, "interface", 0, &iface_node);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create interface node");
        return NULL;
    }

    /* Set interface basic properties */
    ret = lyd_new_term(iface_node, ietf_interfaces_mod, "name", wifi_data->base.name, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set interface name");
    }
    
    ret = lyd_new_term(iface_node, ietf_interfaces_mod, "type", "iana-if-type:ieee80211", 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set interface type");
    }
    
    ret = lyd_new_term(iface_node, ietf_interfaces_mod, "enabled", "true", 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set interface enabled");
    }
    
    ret = lyd_new_term(iface_node, ietf_interfaces_mod, "oper-status", "up", 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set interface oper-status");
    }

    /* Create WiFi-specific node */
    ret = lyd_new_inner(iface_node, netd_mod, "wifi", 0, &wifi_node);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create WiFi node");
        return response; /* Return partial response */
    }

    /* Add WiFi-specific properties */
    ret = lyd_new_term(wifi_node, netd_mod, "authmode", wifi_data->authmode, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set WiFi authmode");
    }

    char txpower_str[16];
    snprintf(txpower_str, sizeof(txpower_str), "%d", wifi_data->txpower);
    ret = lyd_new_term(wifi_node, netd_mod, "txpower", txpower_str, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set WiFi txpower");
    }

    ret = lyd_new_term(wifi_node, netd_mod, "country", wifi_data->country, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set WiFi country");
    }

    ret = lyd_new_term(wifi_node, netd_mod, "regdomain", wifi_data->regdomain, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set WiFi regdomain");
    }

    ret = lyd_new_term(wifi_node, netd_mod, "privacy", wifi_data->privacy, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set WiFi privacy");
    }

    char bmiss_str[16];
    snprintf(bmiss_str, sizeof(bmiss_str), "%d", wifi_data->bmiss);
    ret = lyd_new_term(wifi_node, netd_mod, "bmiss", bmiss_str, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set WiFi bmiss");
    }

    char scanvalid_str[16];
    snprintf(scanvalid_str, sizeof(scanvalid_str), "%d", wifi_data->scanvalid);
    ret = lyd_new_term(wifi_node, netd_mod, "scanvalid", scanvalid_str, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set WiFi scanvalid");
    }

    ret = lyd_new_term(wifi_node, netd_mod, "features", wifi_data->features, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set WiFi features");
    }

    char bintval_str[16];
    snprintf(bintval_str, sizeof(bintval_str), "%d", wifi_data->bintval);
    ret = lyd_new_term(wifi_node, netd_mod, "bintval", bintval_str, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set WiFi bintval");
    }

    ret = lyd_new_term(wifi_node, netd_mod, "parent", wifi_data->parent, 0, NULL);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set WiFi parent");
    }

    return response;
} 