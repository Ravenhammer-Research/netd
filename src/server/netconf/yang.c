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

#include <netd.h>
#include <netconf.h>
#include <libyang/libyang.h>
#include <libyang/log.h>
#include <libyang/tree_data.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/**
 * Mount point extension callback for schema mount
 * @param ext Extension instance
 * @param user_data User data (server state)
 * @param ext_data Extension data to be set
 * @param ext_data_free Flag indicating if ext_data should be freed
 * @return LY_SUCCESS on success, error code on failure
 */
static LY_ERR mount_point_callback(const struct lysc_ext_instance *ext,
                                   void *user_data, void **ext_data,
                                   ly_bool *ext_data_free) {
  (void)ext;
  (void)user_data;
  (void)ext_data;
  (void)ext_data_free;

  debug_log(DEBUG, "Mount point callback not implemented");

  /* Return NULL to disable schema mount validation */
  *ext_data = NULL;
  *ext_data_free = 0;

  return LY_SUCCESS;
}

/**
 * Initialize YANG context
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int yang_init(netd_state_t *state) {
  //(void)ly_set_log_clb(yang_log_callback);
  (void)ly_log_level(LY_LLDBG);

  struct lys_module *mod;
  const char *modules[] = {"ietf-netconf",
                           "ietf-interfaces",
                           "iana-if-type",
                           "ietf-ip",
                           "ietf-routing",
                           "ietf-routing-types",
                           "ietf-inet-types",
                           "ietf-yang-schema-mount",
                           "ietf-yang-library",
                           "ietf-datastores",
                           "ietf-yang-types",
                           "frr-vrf",
                           "netd"};
  int i;

  if (!state) {
    debug_log(ERROR, "Invalid state parameter for YANG initialization");
    return -1;
  }

  debug_log(INFO, "Initializing YANG context with %d modules",
            (int)(sizeof(modules) / sizeof(modules[0])));

  /* Create YANG context with search paths */
  debug_log(DEBUG, "Creating YANG context with search path './yang'");
  if (ly_ctx_new("./yang", 0, &state->yang_ctx) != LY_SUCCESS) {
    debug_log(ERROR, "Failed to create YANG context");
    return -1;
  }

  if (!state->yang_ctx) {
    debug_log(ERROR, "YANG context creation returned NULL");
    return -1;
  }

  debug_log(DEBUG, "YANG context created successfully");

  /* Load required modules */
  int loaded_count = 0;
  for (i = 0; i < (int)(sizeof(modules) / sizeof(modules[0])); i++) {
    debug_log(DEBUG, "Loading module %d/%d: %s", i + 1,
              (int)(sizeof(modules) / sizeof(modules[0])), modules[i]);

    /* Enable writable-running feature for ietf-netconf module */
    if (strcmp(modules[i], "ietf-netconf") == 0) {
      const char *features[] = {"writable-running", NULL};
      mod = ly_ctx_load_module(state->yang_ctx, modules[i], NULL, features);
    } else {
      mod = ly_ctx_load_module(state->yang_ctx, modules[i], NULL, NULL);
    }
    if (!mod) {
      const struct ly_err_item *err = ly_err_last(state->yang_ctx);
      if (err) {
        debug_log(ERROR, "Failed to load module %s: %s", modules[i],
                  err->msg);
        if (err->data_path) {
          debug_log(ERROR, "Data path: %s", err->data_path);
        }
        if (err->schema_path) {
          debug_log(ERROR, "Schema path: %s", err->schema_path);
        }
      } else {
        debug_log(ERROR,
                  "Failed to load module %s (no error details available)",
                  modules[i]);
      }
      goto error;
    }
    loaded_count++;
    debug_log(DEBUG, "Successfully loaded module: %s (revision: %s)",
              modules[i], mod->revision ? mod->revision : "none");
  }

  /* Set up extension data callback for mount points */
  debug_log(DEBUG, "Setting up extension data callback for mount points");
  ly_ctx_set_ext_data_clb(state->yang_ctx, mount_point_callback, state);

  debug_log(INFO, "YANG context initialized successfully with %d modules",
            loaded_count);
  return 0;

error:
  debug_log(ERROR, "YANG context initialization failed, cleaning up");
  yang_cleanup(state);
  return -1;
}

/**
 * Cleanup YANG context
 * @param state Server state
 */
void yang_cleanup(netd_state_t *state) {
  if (state && state->yang_ctx) {
    debug_log(DEBUG, "Cleaning up YANG context");

    /* Clear any error messages that might be holding references */
    ly_err_clean(state->yang_ctx, NULL);

    /* Destroy the YANG context */
    ly_ctx_destroy(state->yang_ctx);
    state->yang_ctx = NULL;
    debug_log(INFO, "YANG context cleaned up successfully");
  } else {
    debug_log(DEBUG, "No YANG context to clean up");
  }
}

/**
 * Validate XML data against YANG schema
 * @param state Server state
 * @param xml_data XML data string to validate
 * @return 0 on success, -1 on failure
 */
int yang_validate_xml(netd_state_t *state, const char *xml_data) {
  struct lyd_node *tree = NULL;
  LY_ERR result;
  char *error_msg = NULL;

  if (!state || !state->yang_ctx || !xml_data) {
    debug_log(ERROR,
              "Invalid parameters for XML validation: state=%p, ctx=%p, xml=%p",
              state, state ? state->yang_ctx : NULL, xml_data);
    return -1;
  }

  debug_log(DEBUG, "YANG validation requested for XML data (length: %zu)",
            strlen(xml_data));

  /* Parse and validate XML data using libyang */
  debug_log(DEBUG, "Parsing XML data with libyang");
  result = lyd_parse_data_mem(state->yang_ctx, xml_data, LYD_XML,
                              LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree);

  if (result != LY_SUCCESS) {
    error_msg = yang_get_validation_error(state->yang_ctx);
    if (error_msg) {
      debug_log(ERROR, "YANG validation failed: %s", error_msg);
      free(error_msg);
    } else {
      debug_log(ERROR, "YANG validation failed with unknown error");
    }

    /* Clean up any partial tree */
    if (tree) {
      debug_log(DEBUG,
                "Cleaning up partial tree after validation failure");
      lyd_free_all(tree);
    }
    return -1;
  }

  debug_log(DEBUG, "XML parsing completed successfully, validating tree");

  /* Additional validation for the parsed tree */
  result = lyd_validate_all(&tree, state->yang_ctx, LYD_VALIDATE_PRESENT, NULL);
  if (result != LY_SUCCESS) {
    error_msg = yang_get_validation_error(state->yang_ctx);
    if (error_msg) {
      debug_log(ERROR, "YANG tree validation failed: %s", error_msg);
      free(error_msg);
    }

    if (tree) {
      debug_log(DEBUG, "Cleaning up tree after validation failure");
      lyd_free_all(tree);
    }
    return -1;
  }

  debug_log(DEBUG, "Tree validation completed successfully");

  /* Clean up the validated tree */
  if (tree) {
    debug_log(DEBUG, "Cleaning up validated tree");
    lyd_free_all(tree);
  }

  debug_log(DEBUG, "YANG validation completed successfully");
  return 0;
}

/**
 * Validate configuration XML before applying
 * @param state Server state
 * @param xml_config XML configuration string
 * @return 0 on success, -1 on failure
 */
int yang_validate_config(netd_state_t *state, const char *xml_config) {
  if (!state || !state->yang_ctx || !xml_config) {
    debug_log(ERROR,
              "Invalid parameters for configuration validation: state=%p, "
              "ctx=%p, config=%p",
              state, state ? state->yang_ctx : NULL, xml_config);
    return -1;
  }

  debug_log(DEBUG, "Validating configuration XML (length: %zu)",
            strlen(xml_config));

  /* Validate the XML against YANG schema */
  if (yang_validate_xml(state, xml_config) < 0) {
    debug_log(ERROR, "Configuration XML validation failed");
    return -1;
  }

  debug_log(INFO, "Configuration XML validation completed successfully");
  return 0;
}

/**
 * Validate NETCONF RPC operation
 * @param state Server state
 * @param rpc_xml NETCONF RPC XML string
 * @return 0 on success, -1 on failure
 */
int yang_validate_rpc(netd_state_t *state, const char *rpc_xml) {
  struct lyd_node *tree = NULL;
  struct lyd_node *op = NULL;
  struct ly_in *in = NULL;
  LY_ERR result;
  char *error_msg = NULL;

  if (!state || !state->yang_ctx || !rpc_xml) {
    debug_log(ERROR,
              "Invalid parameters for RPC validation: state=%p, ctx=%p, rpc=%p",
              state, state ? state->yang_ctx : NULL, rpc_xml);
    return -1;
  }

  debug_log(DEBUG, "Validating NETCONF RPC (length: %zu)",
            strlen(rpc_xml));

  /* Create input handle for the XML data */
  debug_log(DEBUG, "Creating input handle for RPC XML");
  result = ly_in_new_memory(rpc_xml, &in);
  if (result != LY_SUCCESS) {
    debug_log(ERROR, "Failed to create input handle for RPC XML");
    return -1;
  }

  /* Parse and validate NETCONF RPC using libyang */
  debug_log(DEBUG, "Parsing NETCONF RPC with libyang");
  result = lyd_parse_op(state->yang_ctx, NULL, in, LYD_XML,
                        LYD_TYPE_RPC_NETCONF, &tree, &op);
  ly_in_free(in, 0);
  if (result != LY_SUCCESS) {
    error_msg = yang_get_validation_error(state->yang_ctx);
    if (error_msg) {
      debug_log(ERROR, "YANG RPC validation failed: %s", error_msg);
      free(error_msg);
    } else {
      debug_log(ERROR, "YANG RPC validation failed with unknown error");
    }
    if (tree) {
      debug_log(DEBUG, "Cleaning up tree after RPC parsing failure");
      lyd_free_all(tree);
    }
    return -1;
  }

  debug_log(DEBUG,
            "RPC parsing completed successfully, validating operation");

  /* Validate the RPC operation */
  result = lyd_validate_op(op, NULL, LYD_TYPE_RPC_YANG, NULL);
  if (result != LY_SUCCESS) {
    error_msg = yang_get_validation_error(state->yang_ctx);
    if (error_msg) {
      debug_log(ERROR, "YANG RPC operation validation failed: %s",
                error_msg);
      free(error_msg);
    }
    if (tree) {
      debug_log(DEBUG,
                "Cleaning up tree after RPC operation validation failure");
      lyd_free_all(tree);
    }
    return -1;
  }

  /* Clean up the validated tree */
  if (tree) {
    debug_log(DEBUG, "Cleaning up validated RPC tree");
    lyd_free_all(tree);
  }

  debug_log(DEBUG, "NETCONF RPC validation completed successfully");
  return 0;
}

/**
 * Get detailed validation error information
 * @param ctx YANG context
 * @return Error message string (caller must free)
 */
char *yang_get_validation_error(const struct ly_ctx *ctx) {
  const struct ly_err_item *err;
  char *error_msg = NULL;
  size_t msg_len = 0;

  if (!ctx) {
    debug_log(DEBUG, "NULL context provided to get validation error");
    return NULL;
  }

  err = ly_err_last(ctx);
  if (!err) {
    debug_log(DEBUG, "No error found in YANG context");
    return NULL;
  }

  debug_log(DEBUG, "Retrieving validation error: %s", err->msg);

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
    debug_log(ERROR, "Failed to allocate memory for error message");
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

  debug_log(DEBUG, "Formatted error message: %s", error_msg);
  return error_msg;
}

/**
 * Validate netd-specific operations
 * @param state Server state
 * @param operation Operation name
 * @param data Operation data
 * @return 0 on success, -1 on failure
 */
int yang_validate_netd_operation(netd_state_t *state, const char *operation,
                                 const char *data) {
  struct lyd_node *tree = NULL;
  LY_ERR result;
  char *error_msg = NULL;
  char xml_data[4096];

  if (!state || !state->yang_ctx || !operation || !data) {
    debug_log(ERROR,
              "Invalid parameters for netd operation validation: state=%p, "
              "ctx=%p, op=%s, data=%s",
              state, state ? state->yang_ctx : NULL,
              operation ? operation : "NULL", data ? data : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Validating netd operation: %s", operation);

  /* Create XML for the operation */
  snprintf(xml_data, sizeof(xml_data),
           "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           "<rpc message-id=\"1\" "
           "xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
           "  <%s xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
           "    %s\n"
           "  </%s>\n"
           "</rpc>",
           operation, data, operation);

  debug_log(DEBUG, "Generated XML for operation validation (length: %zu)",
            strlen(xml_data));

  /* Parse and validate the operation */
  result = lyd_parse_data_mem(state->yang_ctx, xml_data, LYD_XML,
                              LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree);

  if (result != LY_SUCCESS) {
    error_msg = yang_get_validation_error(state->yang_ctx);
    if (error_msg) {
      debug_log(ERROR, "Netd operation validation failed: %s", error_msg);
      free(error_msg);
    } else {
      debug_log(ERROR,
                "Netd operation validation failed with unknown error");
    }

    if (tree) {
      debug_log(DEBUG,
                "Cleaning up tree after netd operation validation failure");
      lyd_free_all(tree);
    }
    return -1;
  }

  /* Clean up the validated tree */
  if (tree) {
    debug_log(DEBUG, "Cleaning up validated netd operation tree");
    lyd_free_all(tree);
  }

  debug_log(DEBUG, "Netd operation validation completed successfully");
  return 0;
}

/**
 * Check if a YANG module is loaded
 * @param state Server state
 * @param module_name Module name to check
 * @return true if loaded, false otherwise
 */
bool yang_module_loaded(netd_state_t *state, const char *module_name) {
  struct lys_module *mod;

  if (!state || !state->yang_ctx || !module_name) {
    debug_log(DEBUG,
              "Invalid parameters for module check: state=%p, ctx=%p, name=%s",
              state, state ? state->yang_ctx : NULL,
              module_name ? module_name : "NULL");
    return false;
  }

  mod = ly_ctx_get_module(state->yang_ctx, module_name, NULL);
  bool loaded = (mod != NULL);
  debug_log(DEBUG, "Module %s is %s", module_name,
            loaded ? "loaded" : "not loaded");
  return loaded;
}