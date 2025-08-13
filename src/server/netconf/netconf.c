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

/* Global server state */
static struct ly_ctx *yang_ctx = NULL;
static struct nc_pollsession *ps = NULL;

/* Session management */
static bool session_locked = false;
static uint32_t locked_session_id = 0;

/**
 * Global RPC callback for handling NETCONF RPCs
 */
static struct nc_server_reply *rpc_callback(struct lyd_node *rpc, struct nc_session *session) {
    const char *rpc_name;
    struct lyd_node *child;
    
    /* Get the RPC name */
    child = lyd_child(rpc);
    if (!child) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_OP_FAILED, NULL));
    }
    rpc_name = child->schema->name;
    
    /* Route to appropriate handler based on RPC name */
    if (strcmp(rpc_name, "get") == 0) {
        return handle_get_rpc(child, session);
    } else if (strcmp(rpc_name, "get-config") == 0) {
        return handle_get_config_rpc(child, session);
    } else if (strcmp(rpc_name, "edit-config") == 0) {
        return handle_edit_config_rpc(child, session);
    } else if (strcmp(rpc_name, "commit") == 0) {
        return handle_commit_rpc(child, session);
    } else if (strcmp(rpc_name, "discard-changes") == 0) {
        return handle_discard_changes_rpc(child, session);
    } else if (strcmp(rpc_name, "validate") == 0) {
        return handle_validate_rpc(child, session);
    } else if (strcmp(rpc_name, "lock") == 0) {
        return handle_lock_rpc(child, session);
    } else if (strcmp(rpc_name, "unlock") == 0) {
        return handle_unlock_rpc(child, session);
    } else if (strcmp(rpc_name, "close-session") == 0) {
        return handle_close_session_rpc(child, session);
    } else if (strcmp(rpc_name, "kill-session") == 0) {
        return handle_kill_session_rpc(child, session);
    } else {
        /* Unknown RPC */
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_OP_NOT_SUPPORTED, NULL));
    }
}

int netconf_server_init(void) {
    int ret;
    
    /* Initialize libnetconf2 server */
    if (nc_server_init() != 0) {
        debug_log(ERROR, "Failed to initialize libnetconf2 server");
        return -1;
    }
    
    /* Initialize libyang context */
    if (nc_server_init_ctx(&yang_ctx) != 0) {
        debug_log(ERROR, "Failed to initialize libyang context");
        nc_server_destroy();
        return -1;
    }
    
    /* Load YANG modules */
    if (yang_load_modules(yang_ctx) != 0) {
        debug_log(ERROR, "Failed to load YANG modules");
        ly_ctx_destroy(yang_ctx);
        nc_server_destroy();
        return -1;
    }
    
    /* Set global RPC callback */
    nc_set_global_rpc_clb(rpc_callback);
    
    /* Create Unix socket endpoint */
    ret = nc_server_add_endpt_unix_socket_listen("netd", NETD_SOCKET_PATH, 0660, -1, -1);
    if (ret != 0) {
        debug_log(ERROR, "Failed to create Unix socket endpoint: %s", NETD_SOCKET_PATH);
        ly_ctx_destroy(yang_ctx);
        nc_server_destroy();
        return -1;
    }
    
    /* Create poll session */
    ps = nc_ps_new();
    if (!ps) {
        debug_log(ERROR, "Failed to create poll session");
        nc_server_del_endpt_unix_socket("netd");
        ly_ctx_destroy(yang_ctx);
        nc_server_destroy();
        return -1;
    }
    
    return 0;
}

int netconf_server_poll(void) {
    struct nc_session *session;
    NC_MSG_TYPE msgtype;
    int ret;
    
    /* Accept new sessions */
    msgtype = nc_accept(100, yang_ctx, &session);
    if (msgtype == NC_MSG_HELLO) {
        /* New session established */
        if (nc_ps_add_session(ps, session) != 0) {
            debug_log(ERROR, "Failed to add session to poll session");
            nc_session_free(session, NULL);
            return -1;
        }
    } else if (msgtype == NC_MSG_ERROR) {
        debug_log(ERROR, "Error accepting new session");
        return -1;
    }
    /* NC_MSG_WOULDBLOCK is normal, just continue */
    
    /* Poll existing sessions */
    ret = nc_ps_poll(ps, 100, &session);
    if (ret & NC_PSPOLL_ERROR) {
        debug_log(ERROR, "Error in poll session");
        return -1;
    }
    
    if (ret & NC_PSPOLL_SESSION_TERM) {
        /* Session terminated, remove it */
        nc_ps_del_session(ps, session);
        nc_session_free(session, NULL);
    }
    
    return 0;
}

void netconf_server_cleanup(void) {
    if (ps) {
        nc_ps_clear(ps, 1, NULL);
        nc_ps_free(ps);
        ps = NULL;
    }
    
    nc_server_del_endpt_unix_socket("netd");
    
    if (yang_ctx) {
        ly_ctx_destroy(yang_ctx);
        yang_ctx = NULL;
    }
    
    nc_server_destroy();
}

/**
 * Get the global yang context
 */
struct ly_ctx *get_yang_ctx(void) {
    return yang_ctx;
}

/**
 * Handle lock RPC
 */
struct nc_server_reply *handle_lock_rpc(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    
    if (session_locked) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_LOCK_DENIED, NULL));
    }
    
    session_locked = true;
    locked_session_id = nc_session_get_id(session);
    
    return nc_server_reply_ok();
}

/**
 * Handle unlock RPC
 */
struct nc_server_reply *handle_unlock_rpc(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    
    if (!session_locked) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_OP_FAILED, NULL));
    }
    
    if (locked_session_id != nc_session_get_id(session)) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_LOCK_DENIED, NULL));
    }
    
    session_locked = false;
    locked_session_id = 0;
    
    return nc_server_reply_ok();
}

/**
 * Handle close-session RPC
 */
struct nc_server_reply *handle_close_session_rpc(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    
    /* Release lock if this session holds it */
    if (session_locked && locked_session_id == nc_session_get_id(session)) {
        session_locked = false;
        locked_session_id = 0;
    }
    
    /* Close the session */
    nc_session_set_term_reason(session, NC_SESSION_TERM_CLOSED);
    
    return nc_server_reply_ok();
}

/**
 * Handle kill-session RPC
 */
struct nc_server_reply *handle_kill_session_rpc(struct lyd_node *rpc, struct nc_session *session) {
    struct lyd_node *session_id_node;
    uint32_t target_session_id;
    
    /* Find session-id in the RPC */
    if (lyd_find_path(rpc, "session-id", 0, &session_id_node) != LY_SUCCESS || !session_id_node) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_MISSING_ELEM, NULL));
    }
    
    target_session_id = atoi(lyd_get_value(session_id_node));
    
    /* Find the target session */
    struct nc_session *target_session = nc_ps_find_session(ps, NULL, NULL);
    while (target_session) {
        if (nc_session_get_id(target_session) == target_session_id) {
            break;
        }
        target_session = nc_ps_get_session(ps, 0); /* Get next session */
    }
    
    if (!target_session) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_INVALID_VALUE, NULL));
    }
    
    /* Release lock if the killed session holds it */
    if (session_locked && locked_session_id == target_session_id) {
        session_locked = false;
        locked_session_id = 0;
    }
    
    /* Kill the session */
    nc_session_set_term_reason(target_session, NC_SESSION_TERM_KILLED);
    nc_session_set_killed_by(target_session, nc_session_get_id(session));
    
    return nc_server_reply_ok();
}

/**
 * Handle commit RPC
 */
struct nc_server_reply *handle_commit_rpc(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    
    /* Check if session has lock */
    if (!session_locked || locked_session_id != nc_session_get_id(session)) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_LOCK_DENIED, NULL));
    }
    
    /* For now, just return OK since we don't have a candidate datastore */
    /* In a full implementation, this would apply pending changes */
    debug_log(INFO, "Commit requested by session %u", nc_session_get_id(session));
    
    return nc_server_reply_ok();
}

/**
 * Handle discard-changes RPC
 */
struct nc_server_reply *handle_discard_changes_rpc(struct lyd_node *rpc, struct nc_session *session) {
    (void)rpc;
    
    /* Check if session has lock */
    if (!session_locked || locked_session_id != nc_session_get_id(session)) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_LOCK_DENIED, NULL));
    }
    
    /* For now, just return OK since we don't have a candidate datastore */
    /* In a full implementation, this would discard pending changes */
    debug_log(INFO, "Discard changes requested by session %u", nc_session_get_id(session));
    
    return nc_server_reply_ok();
}

/**
 * Handle validate RPC
 */
struct nc_server_reply *handle_validate_rpc(struct lyd_node *rpc, struct nc_session *session) {
    struct lyd_node *source_node, *config_data;
    const char *source;
    
    /* Find source datastore */
    if (lyd_find_path(rpc, "source", 0, &source_node) != LY_SUCCESS || !source_node) {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_MISSING_ELEM, NULL));
    }
    
    source = lyd_get_value(source_node);
    
    /* Find config data if present */
    lyd_find_path(rpc, "config", 0, &config_data);
    
    if (strcmp(source, "running") == 0) {
        /* Validate running datastore - always succeeds for now */
        debug_log(INFO, "Validate running datastore requested by session %u", nc_session_get_id(session));
        return nc_server_reply_ok();
    } else if (strcmp(source, "candidate") == 0) {
        /* Validate candidate datastore - not supported yet */
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_OP_NOT_SUPPORTED, NULL));
    } else if (strcmp(source, "startup") == 0) {
        /* Validate startup datastore - not supported yet */
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_OP_NOT_SUPPORTED, NULL));
    } else {
        return nc_server_reply_err(nc_err(yang_ctx, NC_ERR_INVALID_VALUE, NULL));
    }
}

