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
#include <system/freebsd/80211/80211.h>
#include <netconf/netconf.h>
#include <netd.h>
#include <debug.h>
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Get WiFi interface data from FreeBSD system
 * @param iface_name interface name
 * @param wifi_data pointer to netd_wifi_t structure to populate
 * @return 0 on success, -1 on failure
 */
int get_wifi_data(const char *iface_name, netd_wifi_t *wifi_data) {
    if (!iface_name || !wifi_data) {
        debug_log(ERROR, "Invalid parameters for WiFi data acquisition");
        return -1;
    }
    
    /* Initialize the WiFi structure */
    memset(wifi_data, 0, sizeof(netd_wifi_t));
    strlcpy(wifi_data->base.name, iface_name, sizeof(wifi_data->base.name));
    wifi_data->base.type = NETD_IF_TYPE_WIFI;
    
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
        strlcpy(wifi_data->regdomain, regdomain, sizeof(wifi_data->regdomain));
        strlcpy(wifi_data->country, country, sizeof(wifi_data->country));
        strlcpy(wifi_data->authmode, authmode, sizeof(wifi_data->authmode));
        strlcpy(wifi_data->privacy, privacy, sizeof(wifi_data->privacy));
        wifi_data->txpower = txpower;
        wifi_data->bmiss = bmiss;
        wifi_data->scanvalid = scanvalid;
        strlcpy(wifi_data->features, features, sizeof(wifi_data->features));
        wifi_data->bintval = bintval;
        strlcpy(wifi_data->parent, parent, sizeof(wifi_data->parent));
        
        debug_log(DEBUG, "Successfully acquired WiFi data for %s: authmode=%s, txpower=%d, country=%s", 
                  iface_name, authmode, txpower, country);
        return 0;
    } else {
        debug_log(ERROR, "Failed to get WiFi data for %s", iface_name);
        return -1;
    }
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