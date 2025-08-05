/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "netd.h"
#include <libyang/libyang.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Initialize YANG context
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int yang_init(netd_state_t *state)
{
    struct lys_module *mod;
    const char *modules[] = {
        "ietf-netconf",
        "ietf-interfaces",
        "ietf-routing", 
        "ietf-routing-types",
        "ietf-inet-types",
        "netd"
    };
    int i;
    
    if (!state) {
        return -1;
    }

    /* Create YANG context with search paths */
    if (ly_ctx_new("./yang", 0, &state->yang_ctx) != LY_SUCCESS) {
        debug_log(DEBUG_ERROR, "Failed to create YANG context");
        return -1;
    }
    
    if (!state->yang_ctx) {
        debug_log(DEBUG_ERROR, "Failed to create YANG context");
        return -1;
    }

    /* Load required modules */
    for (i = 0; i < (int)(sizeof(modules) / sizeof(modules[0])); i++) {
        mod = ly_ctx_load_module(state->yang_ctx, modules[i], NULL, NULL);
        if (!mod) {
            const struct ly_err_item *err = ly_err_last(state->yang_ctx);
            if (err) {
                debug_log(DEBUG_ERROR, "Failed to load module %s: %s", modules[i], err->msg);
            } else {
                debug_log(DEBUG_ERROR, "Failed to load module %s", modules[i]);
            }
            goto error;
        }
        debug_log(DEBUG_DEBUG, "Successfully loaded module: %s", modules[i]);
    }

    debug_log(DEBUG_INFO, "YANG context initialized successfully with %d modules", 
              (int)(sizeof(modules) / sizeof(modules[0])));
    return 0;

error:
    yang_cleanup(state);
    return -1;
}

/**
 * Cleanup YANG context
 * @param state Server state
 */
void yang_cleanup(netd_state_t *state)
{
    if (state && state->yang_ctx) {
        ly_ctx_destroy(state->yang_ctx);
        state->yang_ctx = NULL;
        debug_log(DEBUG_INFO, "YANG context cleaned up");
    }
}

/**
 * Validate XML data against YANG schema
 * @param state Server state
 * @param xml_data XML data string to validate
 * @return 0 on success, -1 on failure
 */
int yang_validate_xml(netd_state_t *state, const char *xml_data)
{
    struct lyd_node *tree = NULL;
    LY_ERR result;
    char *error_msg = NULL;
    
    if (!state || !state->yang_ctx || !xml_data) {
        return -1;
    }

    debug_log(DEBUG_DEBUG, "YANG validation requested for XML data (length: %zu)", strlen(xml_data));
    
    /* Parse and validate XML data using libyang */
    result = lyd_parse_data_mem(state->yang_ctx, xml_data, LYD_XML, 
                               LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree);
    
    if (result != LY_SUCCESS) {
        error_msg = yang_get_validation_error(state->yang_ctx);
        if (error_msg) {
            debug_log(DEBUG_ERROR, "YANG validation failed: %s", error_msg);
            free(error_msg);
        } else {
            debug_log(DEBUG_ERROR, "YANG validation failed with unknown error");
        }
        
        /* Clean up any partial tree */
        if (tree) {
            lyd_free_all(tree);
        }
        return -1;
    }
    
    /* Additional validation for the parsed tree */
    result = lyd_validate_all(&tree, state->yang_ctx, LYD_VALIDATE_PRESENT, NULL);
    if (result != LY_SUCCESS) {
        error_msg = yang_get_validation_error(state->yang_ctx);
        if (error_msg) {
            debug_log(DEBUG_ERROR, "YANG tree validation failed: %s", error_msg);
            free(error_msg);
        }
        
        if (tree) {
            lyd_free_all(tree);
        }
        return -1;
    }
    
    /* Validate leafref references if tree exists */
    if (tree) {
        if (yang_validate_leafrefs(state, tree) < 0) {
            debug_log(DEBUG_ERROR, "Leafref validation failed");
            lyd_free_all(tree);
            return -1;
        }
    }
    
    /* Clean up the validated tree */
    if (tree) {
        lyd_free_all(tree);
    }
    
    debug_log(DEBUG_DEBUG, "YANG validation completed successfully");
    return 0;
}

/**
 * Validate configuration XML before applying
 * @param state Server state
 * @param xml_config XML configuration string
 * @return 0 on success, -1 on failure
 */
int yang_validate_config(netd_state_t *state, const char *xml_config)
{
    if (!state || !state->yang_ctx || !xml_config) {
        return -1;
    }

    /* Validate the XML against YANG schema */
    if (yang_validate_xml(state, xml_config) < 0) {
        debug_log(DEBUG_ERROR, "Configuration XML validation failed");
        return -1;
    }

    debug_log(DEBUG_INFO, "Configuration XML validation completed");
    return 0;
}

/**
 * Validate NETCONF RPC operation
 * @param state Server state
 * @param rpc_xml NETCONF RPC XML string
 * @return 0 on success, -1 on failure
 */
int yang_validate_rpc(netd_state_t *state, const char *rpc_xml)
{
    struct lyd_node *tree = NULL;
    struct lyd_node *op = NULL;
    struct ly_in *in = NULL;
    LY_ERR result;
    char *error_msg = NULL;
    
    if (!state || !state->yang_ctx || !rpc_xml) {
        return -1;
    }

    /* Create input handle for the XML data */
    result = ly_in_new_memory(rpc_xml, &in);
    if (result != LY_SUCCESS) {
        return -1;
    }
    
    /* Parse and validate NETCONF RPC using libyang */
    result = lyd_parse_op(state->yang_ctx, NULL, in, LYD_XML, LYD_TYPE_RPC_NETCONF, &tree, &op);
    ly_in_free(in, 0);
    if (result != LY_SUCCESS) {
        error_msg = yang_get_validation_error(state->yang_ctx);
        if (error_msg) {
            debug_log(DEBUG_ERROR, "YANG RPC validation failed: %s", error_msg);
            free(error_msg);
        } else {
            debug_log(DEBUG_ERROR, "YANG RPC validation failed with unknown error");
        }
        if (tree) {
            lyd_free_all(tree);
        }
        return -1;
    }
    
    /* Validate the RPC operation */
    result = lyd_validate_op(op, NULL, LYD_TYPE_RPC_YANG, NULL);
    if (result != LY_SUCCESS) {
        error_msg = yang_get_validation_error(state->yang_ctx);
        if (error_msg) {
            debug_log(DEBUG_ERROR, "YANG RPC operation validation failed: %s", error_msg);
            free(error_msg);
        }
        if (tree) {
            lyd_free_all(tree);
        }
        return -1;
    }
    
    /* Clean up the validated tree */
    if (tree) {
        lyd_free_all(tree);
    }
    
    return 0;
}

/**
 * Validate leafref references in data tree
 * @param state Server state
 * @param data_tree Data tree to validate
 * @return 0 on success, -1 on failure
 */
int yang_validate_leafrefs(netd_state_t *state, struct lyd_node *data_tree)
{
    LY_ERR result;
    
    if (!state || !state->yang_ctx || !data_tree) {
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Validating leafref references in data tree");
    
    /* Link leafref nodes to their targets */
    result = lyd_leafref_link_node_tree(data_tree);
    if (result != LY_SUCCESS) {
        const struct ly_err_item *err = ly_err_last(state->yang_ctx);
        if (err) {
            debug_log(DEBUG_ERROR, "Leafref validation failed: %s", err->msg);
            if (err->data_path) {
                debug_log(DEBUG_ERROR, "Data path: %s", err->data_path);
            }
            if (err->schema_path) {
                debug_log(DEBUG_ERROR, "Schema path: %s", err->schema_path);
            }
        }
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Leafref validation completed successfully");
    return 0;
}

/**
 * Get detailed validation error information
 * @param ctx YANG context
 * @return Error message string (caller must free)
 */
char *yang_get_validation_error(const struct ly_ctx *ctx)
{
    const struct ly_err_item *err;
    char *error_msg = NULL;
    size_t msg_len = 0;
    
    if (!ctx) {
        return NULL;
    }
    
    err = ly_err_last(ctx);
    if (!err) {
        return NULL;
    }
    
    /* Calculate required buffer size */
    msg_len = strlen(err->msg) + 1;
    if (err->data_path) {
        msg_len += strlen("Data path: ") + strlen(err->data_path) + 2;
    }
    if (err->schema_path) {
        msg_len += strlen("Schema path: ") + strlen(err->schema_path) + 2;
    }
    
    /* Allocate and format error message */
    error_msg = malloc(msg_len);
    if (!error_msg) {
        return NULL;
    }
    
    snprintf(error_msg, msg_len, "%s", err->msg);
    if (err->data_path) {
        strcat(error_msg, "\nData path: ");
        strcat(error_msg, err->data_path);
    }
    if (err->schema_path) {
        strcat(error_msg, "\nSchema path: ");
        strcat(error_msg, err->schema_path);
    }
    
    return error_msg;
}

/**
 * Validate netd-specific operations
 * @param state Server state
 * @param operation Operation name
 * @param data Operation data
 * @return 0 on success, -1 on failure
 */
int yang_validate_netd_operation(netd_state_t *state, const char *operation, const char *data)
{
    struct lyd_node *tree = NULL;
    LY_ERR result;
    char *error_msg = NULL;
    char xml_data[4096];
    
    if (!state || !state->yang_ctx || !operation || !data) {
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Validating netd operation: %s", operation);
    
    /* Create XML for the operation */
    snprintf(xml_data, sizeof(xml_data),
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
        "  <%s xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
        "    %s\n"
        "  </%s>\n"
        "</rpc>",
        operation, data, operation);
    
    /* Parse and validate the operation */
    result = lyd_parse_data_mem(state->yang_ctx, xml_data, LYD_XML, 
                               LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree);
    
    if (result != LY_SUCCESS) {
        error_msg = yang_get_validation_error(state->yang_ctx);
        if (error_msg) {
            debug_log(DEBUG_ERROR, "Netd operation validation failed: %s", error_msg);
            free(error_msg);
        } else {
            debug_log(DEBUG_ERROR, "Netd operation validation failed with unknown error");
        }
        
        if (tree) {
            lyd_free_all(tree);
        }
        return -1;
    }
    
    /* Clean up the validated tree */
    if (tree) {
        lyd_free_all(tree);
    }
    
    debug_log(DEBUG_DEBUG, "Netd operation validation completed successfully");
    return 0;
}

/**
 * Check if a YANG module is loaded
 * @param state Server state
 * @param module_name Module name to check
 * @return true if loaded, false otherwise
 */
bool yang_module_loaded(netd_state_t *state, const char *module_name)
{
    struct lys_module *mod;
    
    if (!state || !state->yang_ctx || !module_name) {
        return false;
    }
    
    mod = ly_ctx_get_module(state->yang_ctx, module_name, NULL);
    return (mod != NULL);
} 