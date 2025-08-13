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

#ifndef NETCONF_H
#define NETCONF_H

#include <libnetconf2/messages_server.h>
#include <libnetconf2/session_server.h>
#include <libyang/tree_data.h>

/* Forward declarations */
struct ly_ctx;

/**
 * Initialize the NETCONF server
 * @return 0 on success, -1 on failure
 */
int netconf_server_init(void);

/**
 * Poll for NETCONF messages and handle them
 * @return 0 on success, -1 on failure
 */
int netconf_server_poll(void);

/**
 * Clean up NETCONF server resources
 */
void netconf_server_cleanup(void);

/**
 * Load required YANG modules into the context
 * @param ctx libyang context
 * @return 0 on success, -1 on failure
 */
int yang_load_modules(struct ly_ctx *ctx);

/**
 * Get the global yang context
 * @return libyang context
 */
struct ly_ctx *get_yang_ctx(void);

/* RPC handler function declarations */
struct nc_server_reply *handle_get_rpc(struct lyd_node *rpc, struct nc_session *session);
struct nc_server_reply *handle_get_config_rpc(struct lyd_node *rpc, struct nc_session *session);
struct nc_server_reply *handle_edit_config_rpc(struct lyd_node *rpc, struct nc_session *session);
struct nc_server_reply *handle_commit_rpc(struct lyd_node *rpc, struct nc_session *session);
struct nc_server_reply *handle_discard_changes_rpc(struct lyd_node *rpc, struct nc_session *session);
struct nc_server_reply *handle_validate_rpc(struct lyd_node *rpc, struct nc_session *session);
struct nc_server_reply *handle_lock_rpc(struct lyd_node *rpc, struct nc_session *session);
struct nc_server_reply *handle_unlock_rpc(struct lyd_node *rpc, struct nc_session *session);
struct nc_server_reply *handle_close_session_rpc(struct lyd_node *rpc, struct nc_session *session);
struct nc_server_reply *handle_kill_session_rpc(struct lyd_node *rpc, struct nc_session *session);

#endif /* NETCONF_H */


