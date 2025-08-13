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

#include <net.h>
#include <netconf.h>
#include <debug.h>
#include <libyang/libyang.h>
#include <libnetconf2/messages_client.h>

/**
 * Initialize YANG context for client
 * @param client Client state
 * @return 0 on success, -1 on failure
 */
int yang_init_client(net_client_t *client) {
    if (!client) {
        return -1;
    }
    
    debug_log(INFO, "Initializing YANG context for client");
    
    /* Create YANG context with search path for YANG modules */
    LY_ERR ret = ly_ctx_new("./yang", 0, &client->yang_ctx);
    if (ret != LY_SUCCESS || !client->yang_ctx) {
        debug_log(ERROR, "Failed to create YANG context");
        return -1;
    }
    
    /* Load only the essential modules for NETCONF communication */
    const char *modules[] = {
        "ietf-netconf",
        "ietf-interfaces", 
        "ietf-routing",
        "ietf-inet-types",
        "ietf-yang-types"
    };
    
    for (int i = 0; i < (int)(sizeof(modules) / sizeof(modules[0])); i++) {
        struct lys_module *mod = ly_ctx_load_module(client->yang_ctx, modules[i], NULL, NULL);
        if (!mod) {
            debug_log(ERROR, "Failed to load module %s", modules[i]);
            /* Continue anyway, some modules might not be available */
        } else {
            debug_log(DEBUG, "Loaded module: %s", modules[i]);
        }
    }
    
    debug_log(INFO, "YANG context initialized successfully");
    return 0;
}

/**
 * Cleanup YANG context for client
 * @param client Client state
 */
void yang_cleanup_client(net_client_t *client) {
    if (!client) {
        return;
    }
    
    debug_log(INFO, "Cleaning up YANG context for client");
    
    if (client->yang_ctx) {
        ly_ctx_destroy(client->yang_ctx);
        client->yang_ctx = NULL;
    }
}

/**
 * Validate XML data against YANG schema
 * @param client Client state
 * @param xml_data XML data to validate
 * @return 0 on success, -1 on failure
 */
int yang_validate_xml_client(net_client_t *client, const char *xml_data) {
    if (!client || !client->yang_ctx || !xml_data) {
        return -1;
    }
    
    /* Parse XML data */
    struct lyd_node *data;
    LY_ERR ret = lyd_parse_data_mem(client->yang_ctx, xml_data, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &data);
    if (ret != LY_SUCCESS || !data) {
        debug_log(ERROR, "Failed to parse XML data");
        return -1;
    }
    
    lyd_free_all(data);
    return 0;
}

/**
 * Validate RPC XML against YANG schema
 * @param client Client state
 * @param rpc_xml RPC XML to validate
 * @return 0 on success, -1 on failure
 */
int yang_validate_rpc_client(net_client_t *client, const char *rpc_xml) {
    if (!client || !client->yang_ctx || !rpc_xml) {
        return -1;
    }
    
    /* Parse RPC XML */
    struct lyd_node *rpc;
    LY_ERR ret = lyd_parse_data_mem(client->yang_ctx, rpc_xml, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &rpc);
    if (ret != LY_SUCCESS || !rpc) {
        debug_log(ERROR, "Failed to parse RPC XML");
        return -1;
    }
    
    lyd_free_all(rpc);
    return 0;
}

/**
 * Validate response XML against YANG schema
 * @param client Client state
 * @param response_xml Response XML to validate
 * @return 0 on success, -1 on failure
 */
int yang_validate_response_client(net_client_t *client, const char *response_xml) {
    if (!client || !client->yang_ctx || !response_xml) {
        return -1;
    }
    
    /* Parse response XML */
    struct lyd_node *response;
    LY_ERR ret = lyd_parse_data_mem(client->yang_ctx, response_xml, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &response);
    if (ret != LY_SUCCESS || !response) {
        debug_log(ERROR, "Failed to parse response XML");
        return -1;
    }
    
    lyd_free_all(response);
    return 0;
}

/**
 * Validate data XML against YANG schema
 * @param client Client state
 * @param data_xml Data XML to validate
 * @return 0 on success, -1 on failure
 */
int yang_validate_data_client(net_client_t *client, const char *data_xml) {
    if (!client || !client->yang_ctx || !data_xml) {
        return -1;
    }
    
    /* Parse data XML */
    struct lyd_node *data;
    LY_ERR ret = lyd_parse_data_mem(client->yang_ctx, data_xml, LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &data);
    if (ret != LY_SUCCESS || !data) {
        debug_log(ERROR, "Failed to parse data XML");
        return -1;
    }
    
    lyd_free_all(data);
    return 0;
}

/**
 * Convert YANG data to XML string for display
 * @param data YANG data node
 * @return XML string (must be freed by caller) or NULL on error
 */
char *yang_data_to_xml(struct lyd_node *data) {
    if (!data) {
        return NULL;
    }
    
    char *xml_data;
    if (lyd_print_mem(&xml_data, data, LYD_XML, LYD_PRINT_SHRINK) != 0) {
        debug_log(ERROR, "Failed to convert YANG data to XML");
        return NULL;
    }
    
    return xml_data;
}

/**
 * Get interfaces data and convert to XML for display
 * @param client Client state
 * @param interface_type Optional interface type filter
 * @return XML string (must be freed by caller) or NULL on error
 */
char *yang_get_interfaces_xml(net_client_t *client, const char *interface_type) {
    struct lyd_node *data;
    
    if (netconf_get_interfaces_data(client, &data, interface_type) < 0) {
        debug_log(ERROR, "Failed to get interfaces data");
        return NULL;
    }
    
    char *xml_data = yang_data_to_xml(data);
    lyd_free_all(data);
    
    return xml_data;
}

/**
 * Get VRFs data and convert to XML for display
 * @param client Client state
 * @return XML string (must be freed by caller) or NULL on error
 */
char *yang_get_vrfs_xml(net_client_t *client) {
    struct lyd_node *data;
    
    if (netconf_get_vrfs_data(client, &data) < 0) {
        debug_log(ERROR, "Failed to get VRFs data");
        return NULL;
    }
    
    char *xml_data = yang_data_to_xml(data);
    lyd_free_all(data);
    
    return xml_data;
}

/**
 * Get routes data and convert to XML for display
 * @param client Client state
 * @param fib FIB number
 * @param family Address family
 * @return XML string (must be freed by caller) or NULL on error
 */
char *yang_get_routes_xml(net_client_t *client, uint32_t fib, int family) {
    struct lyd_node *data;
    
    if (netconf_get_routes_data(client, &data, fib, family) < 0) {
        debug_log(ERROR, "Failed to get routes data");
        return NULL;
    }
    
    char *xml_data = yang_data_to_xml(data);
    lyd_free_all(data);
    
    return xml_data;
}