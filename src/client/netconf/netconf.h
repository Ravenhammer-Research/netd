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

/* NETCONF client functions */
int netconf_connect(net_client_t *client);
void netconf_disconnect(net_client_t *client);
int netconf_send_request(net_client_t *client, const char *request, char **response);

/* NETCONF request functions */
int netconf_get_interfaces(net_client_t *client, char **response);
int netconf_get_vrfs(net_client_t *client, char **response);
int netconf_get_routes(net_client_t *client, uint32_t fib, int family, char **response);
int netconf_get_interface_groups(net_client_t *client, char **response);

/* YANG context management */
int yang_init_client(net_client_t *client);
void yang_cleanup_client(net_client_t *client);
int yang_validate_xml_client(net_client_t *client, const char *xml_data);
int yang_validate_rpc_client(net_client_t *client, const char *rpc_xml);
int yang_validate_response_client(net_client_t *client, const char *response_xml);
int yang_validate_data_client(net_client_t *client, const char *data_xml);

#endif /* NETCONF_H */ 