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

#include <netconf.h>
#include <netd.h>
#include <debug.h>
#include <libnetconf2/messages_server.h>
#include <libnetconf2/session_server.h>
#include <libyang/tree_data.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include subdirectory headers */
#include <interface/request/request.h>
#include <route/request/request.h>
#include <vrf/request/request.h>

/* Global yang context - set by netconf.c */
extern struct ly_ctx *yang_ctx;

/* Session management */
static bool session_locked = false;
static uint32_t locked_session_id = 0;

struct nc_server_reply *handle_get_rpc(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    (void)session;
    
    /* For now, return the same as get-config */
    return handle_get_config_rpc(rpc, session);
}

struct nc_server_reply *handle_get_config_rpc(struct lyd_node *rpc, struct nc_session *session) {
    /* Check if this is for interfaces, routes, or VRFs */
    struct lyd_node *data = NULL;
    int ret;
    
    /* Try to find interfaces data */
    ret = lyd_find_path(rpc, "ietf-interfaces:interfaces", 0, &data);
    if (ret == LY_SUCCESS && data) {
        return handle_get_config_interfaces(rpc, session);
    }
    
    /* Try to find routing data */
    ret = lyd_find_path(rpc, "ietf-routing:routing", 0, &data);
    if (ret == LY_SUCCESS && data) {
        return handle_get_config_routes(rpc, session);
    }
    
    /* Try to find VRF data */
    ret = lyd_find_path(rpc, "ietf-routing:routing-instance", 0, &data);
    if (ret == LY_SUCCESS && data) {
        return handle_get_config_vrfs(rpc, session);
    }
    
    /* Default: return interfaces data */
    return handle_get_config_interfaces(rpc, session);
}

struct nc_server_reply *handle_edit_config_rpc(struct lyd_node *rpc, struct nc_session *session) {
    /* Check if session is locked */
    if (session_locked && locked_session_id != nc_session_get_id(session)) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_LOCK_DENIED, NULL));
    }
    
    /* Check if this is for interfaces, routes, or VRFs */
    struct lyd_node *data = NULL;
    int ret;
    
    /* Try to find interfaces data */
    ret = lyd_find_path(rpc, "ietf-interfaces:interfaces", 0, &data);
    if (ret == LY_SUCCESS && data) {
        return handle_edit_config_interfaces(rpc, session);
    }
    
    /* Try to find routing data */
    ret = lyd_find_path(rpc, "ietf-routing:routing", 0, &data);
    if (ret == LY_SUCCESS && data) {
        return handle_edit_config_routes(rpc, session);
    }
    
    /* For now, just return OK */
    return nc_server_reply_ok();
}

struct nc_server_reply *handle_commit_rpc(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    (void)session;
    
    /* For now, just return OK */
    return nc_server_reply_ok();
}

struct nc_server_reply *handle_discard_changes_rpc(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    (void)session;
    
    /* For now, just return OK */
    return nc_server_reply_ok();
}

struct nc_server_reply *handle_validate_rpc(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    (void)session;
    
    /* For now, just return OK */
    return nc_server_reply_ok();
}

struct nc_server_reply *handle_lock_rpc(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    
    if (session_locked) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_LOCK_DENIED, NULL));
    }
    
    session_locked = true;
    locked_session_id = nc_session_get_id(session);
    
    return nc_server_reply_ok();
}

struct nc_server_reply *handle_unlock_rpc(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    
    if (!session_locked || locked_session_id != nc_session_get_id(session)) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_LOCK_DENIED, NULL));
    }
    
    session_locked = false;
    locked_session_id = 0;
    
    return nc_server_reply_ok();
}

struct nc_server_reply *handle_close_session_rpc(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    
    /* Release lock if this session holds it */
    if (session_locked && locked_session_id == nc_session_get_id(session)) {
        session_locked = false;
        locked_session_id = 0;
    }
    
    /* Mark session for termination */
    nc_session_set_term_reason(session, NC_SESSION_TERM_CLOSED);
    
    return nc_server_reply_ok();
}

struct nc_server_reply *handle_kill_session_rpc(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    (void)session;
    
    /* For now, return operation not supported */
    return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_OP_NOT_SUPPORTED, NULL));
}

