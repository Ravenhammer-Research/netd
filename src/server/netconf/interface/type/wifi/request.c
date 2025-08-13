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

#include <wifi.h>
#include <system/freebsd/80211/80211.h>
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
 * Get WiFi interface data from FreeBSD system
 * @param ctx libyang context
 * @param iface_name interface name
 * @param iface_node interface node to add WiFi data to
 * @return 0 on success, -1 on failure
 */
int get_wifi_data(struct ly_ctx *ctx, const char *iface_name, struct lyd_node *iface_node) {
    struct lyd_node *wifi_node = NULL;
    int ret;
    
    /* Create WiFi node */
    ret = lyd_new_inner(iface_node, ctx, "netd", "wifi", 0, &wifi_node);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create WiFi node");
        return -1;
    }
    
    /* Get WiFi data from system */
    char regdomain[32];
    char country[8];
    char authmode[32];
    char privacy[32];
    int txpower;
    int bmiss;
    int scanvalid;
    char features[256];
    int bintval;
    char parent[MAX_IFNAME_LEN];
    
    if (freebsd_wireless_show(iface_name, regdomain, sizeof(regdomain),
                             country, sizeof(country), authmode, sizeof(authmode),
                             privacy, sizeof(privacy), &txpower, &bmiss, &scanvalid,
                             features, sizeof(features), &bintval, parent, sizeof(parent)) == 0) {
        /* Set authentication mode */
        ret = lyd_new_term(wifi_node, ctx, "netd", "authmode", authmode, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set WiFi authmode");
        }
        
        /* Set transmit power */
        char txpower_str[16];
        snprintf(txpower_str, sizeof(txpower_str), "%d", txpower);
        ret = lyd_new_term(wifi_node, ctx, "netd", "txpower", txpower_str, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set WiFi txpower");
        }
        
        /* Set country code */
        ret = lyd_new_term(wifi_node, ctx, "netd", "country", country, 0, NULL);
        if (ret != LY_SUCCESS) {
            debug_log(ERROR, "Failed to set WiFi country");
        }
    }
    
    return 0;
}

/**
 * Set WiFi authentication mode
 * @param wifi_name WiFi interface name
 * @param authmode authentication mode (OPEN, WPA, etc.)
 * @return 0 on success, -1 on failure
 */
int wifi_set_authmode(const char *wifi_name, const char *authmode) {
    return freebsd_wireless_set_authmode(wifi_name, authmode);
}

/**
 * Set WiFi transmit power
 * @param wifi_name WiFi interface name
 * @param txpower transmit power in dBm
 * @return 0 on success, -1 on failure
 */
int wifi_set_txpower(const char *wifi_name, int txpower) {
    return freebsd_wireless_set_txpower(wifi_name, txpower);
} 