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
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

#ifndef NETCONF_H
#define NETCONF_H

#include <net.h>
#include <utils.h>
#include <libnetconf2/netconf.h>
#include <libnetconf2/session_client.h>
#include <libnetconf2/messages_client.h>

/* Forward declarations for libnetconf2 types */
struct nc_msg;
struct nc_rpc;
struct lyd_node;

/* NETCONF client configuration */
#define NETCONF_CLIENT_PORT 830
#define NETCONF_CLIENT_PATH "/var/run/netd.sock"

/* NETCONF client session management */
int netconf_client_init(net_client_t *client);
void netconf_client_cleanup(net_client_t *client);
int netconf_connect(net_client_t *client);
void netconf_disconnect(net_client_t *client);

/* NETCONF message handling */
int netconf_send_rpc(net_client_t *client, struct nc_rpc *rpc, struct nc_msg **reply);
int netconf_send_get_config(net_client_t *client, NC_DATASTORE source, 
                           struct lyd_node *filter, struct nc_msg **reply);
int netconf_send_edit_config(net_client_t *client, NC_DATASTORE target,
                            struct lyd_node *config, struct nc_msg **reply);
int netconf_send_get(net_client_t *client, struct lyd_node *filter, 
                    struct nc_msg **reply);
int netconf_send_commit(net_client_t *client, struct nc_msg **reply);
int netconf_send_validate(net_client_t *client, NC_DATASTORE source, 
                         struct nc_msg **reply);
int netconf_send_lock(net_client_t *client, NC_DATASTORE target, 
                     struct nc_msg **reply);
int netconf_send_unlock(net_client_t *client, NC_DATASTORE target, 
                       struct nc_msg **reply);
int netconf_send_kill_session(net_client_t *client, uint32_t session_id, 
                             struct nc_msg **reply);
int netconf_send_discard_changes(net_client_t *client, struct nc_msg **reply);

/* NETCONF data operations */
int netconf_get_interfaces_data(net_client_t *client, struct lyd_node **data, 
                               const char *interface_type);
int netconf_get_vrfs_data(net_client_t *client, struct lyd_node **data);
int netconf_get_routes_data(net_client_t *client, struct lyd_node **data, 
                           uint32_t fib, int family);
int netconf_apply_config_changes(net_client_t *client, struct lyd_node *config);

/* NETCONF error handling */
int netconf_handle_error_reply(struct nc_msg *reply, char **error_message);

/* Command execution functions */
int execute_command(net_client_t *client, const command_t *cmd);
int execute_set_command(net_client_t *client, const command_t *cmd);
int execute_show_command(net_client_t *client, const command_t *cmd);
int execute_delete_command(net_client_t *client, const command_t *cmd);
int execute_save_command(net_client_t *client, const command_t *cmd);

/* Transaction management functions */
int transaction_begin(void);
int transaction_add_command(const char *command);
int transaction_commit(net_client_t *client);
int transaction_rollback(void);
bool is_transaction_active(void);
int get_transaction_command_count(void);



/* YANG context management */
int yang_init_client(net_client_t *client);
void yang_cleanup_client(net_client_t *client);
int yang_validate_xml_client(net_client_t *client, const char *xml_data);
int yang_validate_rpc_client(net_client_t *client, const char *rpc_xml);
int yang_validate_response_client(net_client_t *client, const char *response_xml);
int yang_validate_data_client(net_client_t *client, const char *data_xml);

/* YANG data conversion functions */
char *yang_data_to_xml(struct lyd_node *data);
char *yang_get_interfaces_xml(net_client_t *client, const char *interface_type);
char *yang_get_vrfs_xml(net_client_t *client);
char *yang_get_routes_xml(net_client_t *client, uint32_t fib, int family);

#endif /* NETCONF_H */ 