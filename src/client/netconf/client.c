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

#include <net.h>
#include <netconf.h>
#include <libyang/libyang.h>
#include <libnetconf2/netconf.h>
#include <libnetconf2/session_client.h>
#include <libnetconf2/messages_client.h>
#include <libyang/tree_data.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* NETCONF client session */
static struct nc_session *client_session = NULL;

/**
 * Initialize NETCONF client
 * @param client Client state
 * @return 0 on success, -1 on failure
 */
int netconf_client_init(net_client_t *client) {
    if (!client) {
        return -1;
    }
    
    debug_log(INFO, "Initializing NETCONF client");
    
    /* Initialize libnetconf2 client */
    if (nc_client_init() != 0) {
        debug_log(ERROR, "Failed to initialize libnetconf2 client");
        return -1;
    }
    
    client->connected = false;
    debug_log(INFO, "NETCONF client initialized successfully");
    
    return 0;
}

/**
 * Cleanup NETCONF client
 * @param client Client state
 */
void netconf_client_cleanup(net_client_t *client) {
    if (!client) {
        return;
    }
    
    debug_log(INFO, "Cleaning up NETCONF client (stub implementation)");
    
    /* TODO: Cleanup libnetconf2 client */
    /* For now, just mark as not connected */
    client_session = NULL;
    client->connected = false;
    debug_log(INFO, "NETCONF client cleanup completed (stub)");
}

/**
 * Connect to the NETCONF server
 * @param client Client state
 * @return 0 on success, -1 on failure
 */
int netconf_connect(net_client_t *client) {
    if (!client) {
        return -1;
    }
    
    debug_log(INFO, "Connecting to NETCONF server");
    
    /* Connect to server using Unix socket */
    client_session = nc_connect_unix(NETCONF_CLIENT_PATH, client->yang_ctx);
    if (!client_session) {
        debug_log(ERROR, "Failed to connect to NETCONF server: %s", strerror(errno));
        return -1;
    }
    
    client->connected = true;
    debug_log(INFO, "Connected to NETCONF server (session ID: %u)", 
              nc_session_get_id(client_session));
    
    return 0;
}

/**
 * Disconnect from the NETCONF server
 * @param client Client state
 */
void netconf_disconnect(net_client_t *client) {
    if (!client) {
        return;
    }
    
    debug_log(INFO, "Disconnecting from NETCONF server");
    
    if (client_session) {
        nc_session_free(client_session, NULL);
        client_session = NULL;
    }
    
    client->connected = false;
    debug_log(INFO, "Disconnected from NETCONF server");
}

/**
 * Send NETCONF RPC and receive reply
 * @param client Client state
 * @param rpc RPC to send
 * @param reply Reply message to be filled (can be NULL if no reply needed)
 * @return 0 on success, -1 on failure
 */
int netconf_send_rpc(net_client_t *client, struct nc_rpc *rpc, struct nc_msg **reply) {
    int ret;
    
    if (!client || !rpc) {
        return -1;
    }
    
    if (!client->connected || !client_session) {
        debug_log(ERROR, "Not connected to server");
        return -1;
    }
    
    debug_log(DEBUG, "Sending NETCONF RPC");
    
    /* Send RPC */
    uint64_t msgid;
    ret = nc_send_rpc(client_session, rpc, 10000, &msgid);
    if (ret != 0) {
        debug_log(ERROR, "Failed to send RPC: ret=%d, errno=%d (%s)", ret, errno, strerror(errno));
        return -1;
    }
    
    /* If reply is requested, try to receive it */
    if (reply) {
        struct lyd_node *envp = NULL;
        struct lyd_node *op = NULL;
        
        /* Receive reply */
        NC_MSG_TYPE msg_type = nc_recv_reply(client_session, rpc, msgid, 10000, &envp, &op);
        if (msg_type == NC_MSG_ERROR) {
            debug_log(ERROR, "Received error reply from server");
            return -1;
        } else if (msg_type == NC_MSG_REPLY) {
            debug_log(DEBUG, "Received reply from server");
            if (op) {
                debug_log(DEBUG, "Reply contains data");
            } else {
                debug_log(DEBUG, "Reply is OK (no data)");
            }
            *reply = NULL; /* For now, just mark as received */
        } else {
            debug_log(ERROR, "Unexpected reply type: %d", msg_type);
            return -1;
        }
    }
    
    debug_log(DEBUG, "NETCONF RPC sent successfully");
    return 0;
}

/**
 * Send get-config RPC
 * @param client Client state
 * @param source Source datastore
 * @param filter Filter to apply
 * @param reply Reply message to be filled
 * @return 0 on success, -1 on failure
 */
int netconf_send_get_config(net_client_t *client, NC_DATASTORE source, 
                           struct lyd_node *filter, struct nc_msg **reply) {
    struct nc_rpc *rpc;
    int ret;
    
    if (!client || !reply) {
        return -1;
    }
    
    /* Create get-config RPC with filter if provided */
    const char *filter_str = NULL;
    if (filter) {
        /* Convert filter to string for RPC */
        char *filter_xml;
        if (lyd_print_mem(&filter_xml, filter, LYD_XML, LYD_PRINT_SHRINK) == 0) {
            filter_str = filter_xml;
        }
    }
    
    rpc = nc_rpc_getconfig(source, filter_str, NC_WD_UNKNOWN, NC_PARAMTYPE_CONST);
    if (!rpc) {
        debug_log(ERROR, "Failed to create get-config RPC");
        return -1;
    }
    
    /* Send RPC */
    ret = netconf_send_rpc(client, rpc, reply);
    
    /* Free RPC */
    nc_rpc_free(rpc);
    
    return ret;
}

/**
 * Send edit-config RPC
 * @param client Client state
 * @param target Target datastore
 * @param config Configuration data
 * @param reply Reply message to be filled
 * @return 0 on success, -1 on failure
 */
int netconf_send_edit_config(net_client_t *client, NC_DATASTORE target,
                            struct lyd_node *config, struct nc_msg **reply) {
    struct nc_rpc *rpc;
    int ret;
    
    if (!client || !config || !reply) {
        return -1;
    }
    
    /* Create edit-config RPC */
    rpc = nc_rpc_edit(target, NC_RPC_EDIT_DFLTOP_MERGE, NC_RPC_EDIT_TESTOPT_SET, 
                     NC_RPC_EDIT_ERROPT_STOP, NULL, NC_PARAMTYPE_CONST);
    if (!rpc) {
        debug_log(ERROR, "Failed to create edit-config RPC");
            return -1;
        }
        
    /* Send RPC */
    ret = netconf_send_rpc(client, rpc, reply);
    
    /* Free RPC */
    nc_rpc_free(rpc);
    
    return ret;
}

/**
 * Send get RPC
 * @param client Client state
 * @param filter Filter to apply
 * @param reply Reply message to be filled
 * @return 0 on success, -1 on failure
 */
int netconf_send_get(net_client_t *client, struct lyd_node *filter, 
                    struct nc_msg **reply) {
    struct nc_rpc *rpc;
    int ret;
    
    if (!client || !reply) {
            return -1;
        }
        
    /* Create get RPC with filter if provided */
    const char *filter_str = NULL;
    if (filter) {
        /* Convert filter to string for RPC */
        char *filter_xml;
        if (lyd_print_mem(&filter_xml, filter, LYD_XML, LYD_PRINT_SHRINK) == 0) {
            filter_str = filter_xml;
        }
    }
    
    rpc = nc_rpc_get(filter_str, NC_WD_UNKNOWN, NC_PARAMTYPE_CONST);
    if (!rpc) {
        debug_log(ERROR, "Failed to create get RPC");
        return -1;
    }
    
    /* Send RPC */
    ret = netconf_send_rpc(client, rpc, reply);
    
    /* Free RPC */
    nc_rpc_free(rpc);
    
    return ret;
}

/**
 * Send commit RPC
 * @param client Client state
 * @param reply Reply message to be filled
 * @return 0 on success, -1 on failure
 */
int netconf_send_commit(net_client_t *client, struct nc_msg **reply) {
    struct nc_rpc *rpc;
    int ret;
    
    if (!client || !reply) {
        return -1;
    }
    
    /* Create commit RPC */
    rpc = nc_rpc_commit(0, 0, NULL, NULL, NC_PARAMTYPE_CONST);
    if (!rpc) {
        debug_log(ERROR, "Failed to create commit RPC");
        return -1;
    }
    
    /* Send RPC */
    ret = netconf_send_rpc(client, rpc, reply);
    
    /* Free RPC */
    nc_rpc_free(rpc);
    
    return ret;
}

/**
 * Send validate RPC
 * @param client Client state
 * @param source Source datastore
 * @param reply Reply message to be filled
 * @return 0 on success, -1 on failure
 */
int netconf_send_validate(net_client_t *client, NC_DATASTORE source, 
                         struct nc_msg **reply) {
    struct nc_rpc *rpc;
    int ret;
    
    if (!client || !reply) {
        return -1;
    }
    
    /* Create validate RPC */
    rpc = nc_rpc_validate(source, NULL, NC_PARAMTYPE_CONST);
    if (!rpc) {
        debug_log(ERROR, "Failed to create validate RPC");
        return -1;
    }
    
    /* Send RPC */
    ret = netconf_send_rpc(client, rpc, reply);
    
    /* Free RPC */
    nc_rpc_free(rpc);
    
    return ret;
}

/**
 * Send lock RPC
 * @param client Client state
 * @param target Target datastore
 * @param reply Reply message to be filled
 * @return 0 on success, -1 on failure
 */
int netconf_send_lock(net_client_t *client, NC_DATASTORE target, 
                     struct nc_msg **reply) {
    struct nc_rpc *rpc;
    int ret;
    
    if (!client || !reply) {
        return -1;
    }
    
    /* Create lock RPC */
    rpc = nc_rpc_lock(target);
    if (!rpc) {
        debug_log(ERROR, "Failed to create lock RPC");
        return -1;
    }
    
    /* Send RPC */
    ret = netconf_send_rpc(client, rpc, reply);
    
    /* Free RPC */
    nc_rpc_free(rpc);
    
    return ret;
}

/**
 * Send unlock RPC
 * @param client Client state
 * @param target Target datastore
 * @param reply Reply message to be filled
 * @return 0 on success, -1 on failure
 */
int netconf_send_unlock(net_client_t *client, NC_DATASTORE target, 
                       struct nc_msg **reply) {
    struct nc_rpc *rpc;
    int ret;
    
    if (!client || !reply) {
        return -1;
    }
    
    /* Create unlock RPC */
    rpc = nc_rpc_unlock(target);
    if (!rpc) {
        debug_log(ERROR, "Failed to create unlock RPC");
        return -1;
    }
    
    /* Send RPC */
    ret = netconf_send_rpc(client, rpc, reply);
    
    /* Free RPC */
    nc_rpc_free(rpc);
    
    return ret;
}

/**
 * Send kill-session RPC
 * @param client Client state
 * @param session_id Session ID to kill
 * @param reply Reply message to be filled
 * @return 0 on success, -1 on failure
 */
int netconf_send_kill_session(net_client_t *client, uint32_t session_id, 
                             struct nc_msg **reply) {
    struct nc_rpc *rpc;
    int ret;
    
    if (!client || !reply) {
        return -1;
    }
    
    /* Create kill-session RPC */
    rpc = nc_rpc_kill(session_id);
    if (!rpc) {
        debug_log(ERROR, "Failed to create kill-session RPC");
        return -1;
    }
    
    /* Send RPC */
    ret = netconf_send_rpc(client, rpc, reply);
    
    /* Free RPC */
    nc_rpc_free(rpc);
    
    return ret;
}

/**
 * Send discard-changes RPC
 * @param client Client state
 * @param reply Reply message to be filled
 * @return 0 on success, -1 on failure
 */
int netconf_send_discard_changes(net_client_t *client, struct nc_msg **reply) {
    struct nc_rpc *rpc;
    int ret;
    
    if (!client || !reply) {
        return -1;
    }
    
    /* Create discard-changes RPC */
    rpc = nc_rpc_discard();
    if (!rpc) {
        debug_log(ERROR, "Failed to create discard-changes RPC");
        return -1;
    }
    
    /* Send RPC */
    ret = netconf_send_rpc(client, rpc, reply);
    
    /* Free RPC */
    nc_rpc_free(rpc);
    
    return ret;
}

/* Data operations implementation */

int netconf_get_interfaces_data(net_client_t *client, struct lyd_node **data, 
                               const char *interface_type) {
    struct nc_rpc *rpc;
    struct nc_msg *reply;
    int ret;
    
    if (!client || !data) {
        return -1;
    }
    
    /* Create get RPC for interfaces - always include interfaces filter */
    const char *filter_str = "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\"/>";
    
    debug_log(DEBUG, "Creating get RPC with filter: %s", filter_str);
    rpc = nc_rpc_get(filter_str, NC_WD_UNKNOWN, NC_PARAMTYPE_CONST);
    if (!rpc) {
        debug_log(ERROR, "Failed to create get RPC for interfaces");
        return -1;
    }
    debug_log(DEBUG, "Successfully created get RPC");
    
    /* Send RPC */
    ret = netconf_send_rpc(client, rpc, &reply);
    nc_rpc_free(rpc);
    
    if (ret < 0) {
        return ret;
    }
    
    /* For now, create dummy data since we don't have full reply handling */
    *data = NULL;
    return 0;
}

int netconf_get_vrfs_data(net_client_t *client, struct lyd_node **data) {
    struct nc_rpc *rpc;
    struct nc_msg *reply;
    int ret;
    
    if (!client || !data) {
        return -1;
    }
    
    /* Create get RPC for VRFs with specific filter */
    const char *filter_str = "<routing xmlns=\"urn:ietf:params:xml:ns:yang:ietf-routing\">"
                            "<routing-instance>"
                            "<name/>"
                            "<description/>"
                            "</routing-instance>"
                            "</routing>";
    
    rpc = nc_rpc_get(filter_str, NC_WD_UNKNOWN, NC_PARAMTYPE_CONST);
    if (!rpc) {
        debug_log(ERROR, "Failed to create get RPC for VRFs");
        return -1;
    }
    
    /* Send RPC */
    ret = netconf_send_rpc(client, rpc, &reply);
    nc_rpc_free(rpc);
    
    if (ret < 0) {
        return ret;
    }
    
    /* For now, create dummy data since we don't have full reply handling */
    *data = NULL;
    return 0;
}

int netconf_get_routes_data(net_client_t *client, struct lyd_node **data, 
                           uint32_t fib, int family) {
    struct nc_rpc *rpc;
    struct nc_msg *reply;
    int ret;
    
    if (!client || !data) {
        return -1;
    }
    
    /* Create get RPC for routes with FIB and family filters */
    const char *filter_str = NULL;
    char filter_buf[512];
    
    if (family == AF_INET) {
        snprintf(filter_buf, sizeof(filter_buf), 
                "<routing xmlns=\"urn:ietf:params:xml:ns:yang:ietf-routing\">"
                "<routing-instance>"
                "<name>%u</name>"
                "<ribs>"
                "<rib>"
                "<name>ipv4-unicast</name>"
                "<routes>"
                "<route/>"
                "</routes>"
                "</rib>"
                "</ribs>"
                "</routing-instance>"
                "</routing>", fib);
    } else if (family == AF_INET6) {
        snprintf(filter_buf, sizeof(filter_buf), 
                "<routing xmlns=\"urn:ietf:params:xml:ns:yang:ietf-routing\">"
                "<routing-instance>"
                "<name>%u</name>"
                "<ribs>"
                "<rib>"
                "<name>ipv6-unicast</name>"
                "<routes>"
                "<route/>"
                "</routes>"
                "</rib>"
                "</ribs>"
                "</routing-instance>"
                "</routing>", fib);
    } else {
        /* Default to both IPv4 and IPv6 */
        snprintf(filter_buf, sizeof(filter_buf), 
                "<routing xmlns=\"urn:ietf:params:xml:ns:yang:ietf-routing\">"
                "<routing-instance>"
                "<name>%u</name>"
                "<ribs>"
                "<rib>"
                "<routes>"
                "<route/>"
                "</routes>"
                "</rib>"
                "</ribs>"
                "</routing-instance>"
                "</routing>", fib);
    }
    filter_str = filter_buf;
    
    rpc = nc_rpc_get(filter_str, NC_WD_UNKNOWN, NC_PARAMTYPE_CONST);
    if (!rpc) {
        debug_log(ERROR, "Failed to create get RPC for routes");
        return -1;
    }
    
    /* Send RPC */
    ret = netconf_send_rpc(client, rpc, &reply);
    nc_rpc_free(rpc);
    
    if (ret < 0) {
        return ret;
    }
    
    /* For now, create dummy data since we don't have full reply handling */
    *data = NULL;
    return 0;
}

int netconf_apply_config_changes(net_client_t *client, struct lyd_node *config) {
    struct nc_rpc *rpc;
    struct nc_msg *reply;
    int ret;
    
    if (!client || !config) {
        return -1;
    }
    
    /* Create edit-config RPC */
    rpc = nc_rpc_edit(NC_DATASTORE_RUNNING, NC_RPC_EDIT_DFLTOP_MERGE, 
                     NC_RPC_EDIT_TESTOPT_SET, NC_RPC_EDIT_ERROPT_STOP,
                     NULL, NC_PARAMTYPE_CONST);
    if (!rpc) {
        debug_log(ERROR, "Failed to create edit-config RPC");
        return -1;
    }
    
    /* Send RPC */
    ret = netconf_send_rpc(client, rpc, &reply);
    nc_rpc_free(rpc);
    
    if (ret < 0) {
        return ret;
    }
    
    return 0;
} 

int netconf_handle_error_reply(struct nc_msg *reply, char **error_message) {
    if (!reply || !error_message) {
        return -1;
    }
    
    /* For now, just return a generic error message since we don't have full error handling */
    *error_message = strdup("NETCONF error occurred");
    
    return 0;
}

 