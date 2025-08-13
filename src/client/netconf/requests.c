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

#include <netconf/netconf.h>
#include <libnetconf2/messages_client.h>
#include <libyang/tree_data.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Get interfaces from the server
 */
int netconf_get_interfaces(net_client_t *client, const char *type_filter, char **response) {
    struct lyd_node *data = NULL;
    int ret;
    
    if (!client || !response) {
        return -1;
    }
    
    /* Send get request for interfaces */
    ret = netconf_get_interfaces_data(client, &data, type_filter);
    if (ret < 0) {
        return -1;
    }
    
    /* Convert data to XML response */
    if (data) {
        *response = yang_data_to_xml(data);
    } else {
        *response = NULL;
    }
    
    return 0;
}

/**
 * Get VRFs from the server
 */
int netconf_get_vrfs(net_client_t *client, char **response) {
    struct lyd_node *data = NULL;
    int ret;
    
    if (!client || !response) {
        return -1;
    }
    
    /* Send get request for VRFs */
    ret = netconf_get_vrfs_data(client, &data);
    if (ret < 0) {
        return -1;
    }
    
    /* Convert data to XML response */
    if (data) {
        *response = yang_data_to_xml(data);
    } else {
        *response = NULL;
    }
    
    return 0;
}

/**
 * Get routes from the server
 */
int netconf_get_routes(net_client_t *client, uint32_t fib, int family, char **response) {
    struct lyd_node *data = NULL;
    int ret;
    
    if (!client || !response) {
        return -1;
    }
    
    /* Send get request for routes */
    ret = netconf_get_routes_data(client, &data, fib, family);
    if (ret < 0) {
        return -1;
    }
    
    /* Convert data to XML response */
    if (data) {
        *response = yang_data_to_xml(data);
    } else {
        *response = NULL;
    }
    
    return 0;
}

/**
 * Get interface groups from the server
 */
int netconf_get_interface_groups(net_client_t *client, char **response) {
    struct lyd_node *data = NULL;
    int ret;
    
    if (!client || !response) {
        return -1;
    }
    
    /* Send get request for interface groups */
    ret = netconf_get_interfaces_data(client, &data, "group");
    if (ret < 0) {
        return -1;
    }
    
    /* Convert data to XML response */
    if (data) {
        *response = yang_data_to_xml(data);
    } else {
        *response = NULL;
    }
    
    return 0;
} 