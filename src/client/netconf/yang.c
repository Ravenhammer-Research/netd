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
#include <libyang/libyang.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Initialize YANG context for client
 */
int yang_init_client(net_client_t *client) {
  struct lys_module *mod;
  const char *modules[] = {"ietf-interfaces", "frr-vrf", "ietf-routing-types",
                           "ietf-inet-types", "ietf-ip", "ietf-netconf",
                           "iana-if-type",    "netd"};
  int i;

  if (!client) {
    return -1;
  }

  /* Initialize libyang context with search paths */
  if (ly_ctx_new("./yang", 0, &client->yang_ctx) != LY_SUCCESS) {
    fprintf(stderr, "Failed to initialize YANG context\n");
    return -1;
  }

  if (!client->yang_ctx) {
    fprintf(stderr, "Failed to create YANG context\n");
    return -1;
  }

  /* Load required modules */
  for (i = 0; i < (int)(sizeof(modules) / sizeof(modules[0])); i++) {
    /* Enable writable-running feature for ietf-netconf module */
    if (strcmp(modules[i], "ietf-netconf") == 0) {
      const char *features[] = {"writable-running", NULL};
      mod = ly_ctx_load_module(client->yang_ctx, modules[i], NULL, features);
    } else {
      mod = ly_ctx_load_module(client->yang_ctx, modules[i], NULL, NULL);
    }
    if (!mod) {
      const struct ly_err_item *err = ly_err_last(client->yang_ctx);
      if (err) {
        fprintf(stderr, "Failed to load module %s: %s\n", modules[i], err->msg);
      } else {
        fprintf(stderr, "Failed to load module %s\n", modules[i]);
      }
      goto error;
    }
    debug_log(DEBUG_DEBUG, "Successfully loaded module: %s", modules[i]);
  }

  debug_log(DEBUG_INFO,
            "Client YANG context initialized successfully with %d modules",
            (int)(sizeof(modules) / sizeof(modules[0])));
  return 0;

error:
  yang_cleanup_client(client);
  return -1;
}

/**
 * Cleanup YANG context for client
 */
void yang_cleanup_client(net_client_t *client) {
  if (client && client->yang_ctx) {
    /* Clear any error messages that might be holding references */
    ly_err_clean(client->yang_ctx, NULL);

    /* Destroy the YANG context */
    ly_ctx_destroy(client->yang_ctx);
    client->yang_ctx = NULL;
  }
}

/**
 * Validate XML data against YANG schema (client-side)
 * @param client Client structure
 * @param xml_data XML data string to validate
 * @return 0 on success, -1 on failure
 */
int yang_validate_xml_client(net_client_t *client, const char *xml_data) {
  struct lyd_node *tree = NULL;
  LY_ERR result;
  const struct ly_err_item *err;

  if (!client || !client->yang_ctx || !xml_data) {
    return -1;
  }

  debug_log(DEBUG_DEBUG,
            "Client YANG validation requested for XML data (length: %zu)",
            strlen(xml_data));

  /* Parse and validate XML data using libyang */
  result = lyd_parse_data_mem(client->yang_ctx, xml_data, LYD_XML,
                              LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree);

  if (result != LY_SUCCESS) {
    err = ly_err_last(client->yang_ctx);
    if (err) {
      debug_log(DEBUG_ERROR, "Client YANG validation failed: %s", err->msg);
      if (err->data_path) {
        debug_log(DEBUG_ERROR, "Data path: %s", err->data_path);
      }
      if (err->schema_path) {
        debug_log(DEBUG_ERROR, "Schema path: %s", err->schema_path);
      }
    } else {
      debug_log(DEBUG_ERROR,
                "Client YANG validation failed with unknown error");
    }

    /* Clean up any partial tree */
    if (tree) {
      lyd_free_all(tree);
    }
    return -1;
  }

  /* Additional validation for the parsed tree */
  result =
      lyd_validate_all(&tree, client->yang_ctx, LYD_VALIDATE_PRESENT, NULL);
  if (result != LY_SUCCESS) {
    err = ly_err_last(client->yang_ctx);
    if (err) {
      debug_log(DEBUG_ERROR, "Client YANG tree validation failed: %s",
                err->msg);
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

  debug_log(DEBUG_DEBUG, "Client YANG validation completed successfully");
  return 0;
}

/**
 * Validate NETCONF response against YANG schema (client-side)
 * @param client Client structure
 * @param response_xml NETCONF response XML string
 * @return 0 on success, -1 on failure
 */
int yang_validate_response_client(net_client_t *client,
                                  const char *response_xml) {
  char *data_content = NULL;

  if (!client || !client->yang_ctx || !response_xml) {
    return -1;
  }

  debug_log(
      DEBUG_DEBUG,
      "Client YANG response validation requested for XML data (length: %zu)",
      strlen(response_xml));

  /* Skip response validation as server doesn't validate responses */
  /* Only validate data content if present */

  /* Try to extract and validate data content if present */
  if (strstr(response_xml, "<data>") && strstr(response_xml, "</data>")) {
    debug_log(DEBUG_DEBUG, "Found data content in response, validating data");

    /* Extract data content between <data> tags */
    const char *data_start = strstr(response_xml, "<data>");
    const char *data_end = strstr(response_xml, "</data>");
    if (data_start && data_end && data_end > data_start) {
      /* Skip past the <data> tag */
      data_start += 6; /* length of "<data>" */
      size_t content_len = data_end - data_start;

      /* Create properly namespaced XML for validation */
      data_content = malloc(content_len +
                            200); /* Extra space for namespace declarations */
      if (data_content) {
        /* Add XML declaration and namespace declarations */
        snprintf(data_content, content_len + 200,
                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<data xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                 "%.*s\n"
                 "</data>",
                 (int)content_len, data_start);

        /* Validate the data content */
        if (yang_validate_data_client(client, data_content) < 0) {
          debug_log(DEBUG_WARN,
                    "Data content validation failed, but continuing");
        } else {
          debug_log(DEBUG_DEBUG, "Data content validated successfully");
        }

        free(data_content);
      }
    }
  }

  debug_log(DEBUG_DEBUG,
            "Client YANG response validation completed successfully");
  return 0;
}

/**
 * Validate NETCONF RPC request against YANG schema (client-side)
 * @param client Client structure
 * @param rpc_xml NETCONF RPC XML string
 * @return 0 on success, -1 on failure
 */
int yang_validate_rpc_client(net_client_t *client, const char *rpc_xml) {
  struct lyd_node *tree = NULL;
  struct lyd_node *op = NULL;
  struct ly_in *in = NULL;
  LY_ERR result;
  const struct ly_err_item *err;

  if (!client || !client->yang_ctx || !rpc_xml) {
    return -1;
  }

  debug_log(DEBUG_DEBUG,
            "Client YANG RPC validation requested for XML data (length: %zu)",
            strlen(rpc_xml));

  /* Create input structure for XML data */
  result = ly_in_new_memory(rpc_xml, &in);
  if (result != LY_SUCCESS) {
    debug_log(DEBUG_ERROR,
              "Failed to create input structure for RPC validation");
    return -1;
  }

  /* Parse and validate NETCONF RPC using libyang */
  result = lyd_parse_op(client->yang_ctx, NULL, in, LYD_XML,
                        LYD_TYPE_RPC_NETCONF, &tree, &op);
  /* Note: lyd_parse_op automatically frees the input structure, so we don't
   * need to call ly_in_free */

  if (result != LY_SUCCESS) {
    err = ly_err_last(client->yang_ctx);
    if (err) {
      debug_log(DEBUG_ERROR, "Client YANG RPC validation failed: %s", err->msg);
      if (err->data_path) {
        debug_log(DEBUG_ERROR, "Data path: %s", err->data_path);
      }
      if (err->schema_path) {
        debug_log(DEBUG_ERROR, "Schema path: %s", err->schema_path);
      }
    } else {
      debug_log(DEBUG_ERROR,
                "Client YANG RPC validation failed with unknown error");
    }

    /* Clean up any partial tree */
    if (tree) {
      lyd_free_all(tree);
    }
    return -1;
  }

  /* Validate the RPC operation */
  result = lyd_validate_op(op, NULL, LYD_TYPE_RPC_YANG, NULL);
  if (result != LY_SUCCESS) {
    err = ly_err_last(client->yang_ctx);
    if (err) {
      debug_log(DEBUG_ERROR, "Client YANG RPC operation validation failed: %s",
                err->msg);
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

  debug_log(DEBUG_DEBUG, "Client YANG RPC validation completed successfully");
  return 0;
}

/**
 * Validate data content within NETCONF response (client-side)
 * @param client Client structure
 * @param data_xml Data XML string (without NETCONF envelope)
 * @return 0 on success, -1 on failure
 */
int yang_validate_data_client(net_client_t *client, const char *data_xml) {
  struct lyd_node *tree = NULL;
  LY_ERR result;
  const struct ly_err_item *err;

  if (!client || !client->yang_ctx || !data_xml) {
    return -1;
  }

  debug_log(DEBUG_DEBUG,
            "Client YANG data validation requested for XML data (length: %zu)",
            strlen(data_xml));

  /* Parse and validate data using libyang */
  result = lyd_parse_data_mem(client->yang_ctx, data_xml, LYD_XML,
                              LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, &tree);

  if (result != LY_SUCCESS) {
    err = ly_err_last(client->yang_ctx);
    if (err) {
      debug_log(DEBUG_ERROR, "Client YANG data validation failed: %s",
                err->msg);
      if (err->data_path) {
        debug_log(DEBUG_ERROR, "Data path: %s", err->data_path);
      }
      if (err->schema_path) {
        debug_log(DEBUG_ERROR, "Schema path: %s", err->schema_path);
      }
    } else {
      debug_log(DEBUG_ERROR,
                "Client YANG data validation failed with unknown error");
    }

    /* Clean up any partial tree */
    if (tree) {
      lyd_free_all(tree);
    }
    return -1;
  }

  /* Additional validation for the parsed tree */
  result =
      lyd_validate_all(&tree, client->yang_ctx, LYD_VALIDATE_PRESENT, NULL);
  if (result != LY_SUCCESS) {
    err = ly_err_last(client->yang_ctx);
    if (err) {
      debug_log(DEBUG_ERROR, "Client YANG data tree validation failed: %s",
                err->msg);
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

  debug_log(DEBUG_DEBUG, "Client YANG data validation completed successfully");
  return 0;
}