/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistribributions of source code must retain the above copyright notice,
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

#ifndef NETCONF_H
#define NETCONF_H

#define NETCONF_RESPONSE_BUFFER_SIZE (128 * 1024 * 1024) /* 128MB */

/* Forward declarations */
struct netd_state;

/* Main NETCONF request handler */
int netconf_handle_request(netd_state_t *state, const char *request,
                          char **response);

/* Request type checking functions */
bool is_get_interfaces_request(const char *request);
bool is_get_vrfs_request(const char *request);
bool is_get_vrf_routes_request(const char *request);
bool is_save_request(const char *request);
bool is_commit_request(const char *request);
bool is_edit_config_request(const char *request);

/* Edit config handling */
int handle_edit_config(netd_state_t *state, const char *request,
                       const char *message_id, char **response);

/* Request handling functions */
int handle_commit_request(netd_state_t *state, const char *request,
                          const char *message_id, char **response);
int handle_save_request(netd_state_t *state, const char *request,
                         const char *message_id, char **response);

/* Response creation functions */
char *create_success_response(const char *message_id);
char *create_error_response(const char *message_id, const char *error_type,
                            const char *error_message);

/* Utility functions */
int prepare_response(char *response, const char *format, ...);

/* YANG/Netconf functions */
int yang_init(netd_state_t *state);
void yang_cleanup(netd_state_t *state);
int yang_validate_xml(netd_state_t *state, const char *xml_data);
int yang_validate_config(netd_state_t *state, const char *xml_config);
int yang_validate_rpc(netd_state_t *state, const char *rpc_xml);
int yang_validate_leafrefs(netd_state_t *state, struct lyd_node *data_tree);
char *yang_get_validation_error(const struct ly_ctx *ctx);
int yang_validate_netd_operation(netd_state_t *state, const char *operation,
                                 const char *data);
bool yang_module_loaded(netd_state_t *state, const char *module_name);
void yang_log_callback(LY_LOG_LEVEL level, const char *msg,
                       const char *data_path, const char *schema_path,
                       uint64_t line);
int netconf_handle_request(netd_state_t *state, const char *request,
                           char **response);


#endif /* NETCONF_H */ 