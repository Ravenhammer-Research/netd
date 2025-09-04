/*
 * Copyright (c) 2024 Paige Thompson / Ravenhammer Research (paige@paige.bio)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <shared/include/logger.hpp>
#include <shared/include/request.hpp>
#include <shared/include/response.hpp>
#include <shared/include/yang.hpp>
#include <libnetconf2/session_client.h>
#include <libnetconf2/messages_client.h>
#include <libyang/libyang.h>
#include <cstring>
#include <stdexcept>

namespace netd {

class NetconfClient {
public:
    NetconfClient() : session_(nullptr), connected_(false) {
        yang_ = createYang();
    }
    
    ~NetconfClient() {
        if (connected_) {
            disconnect();
        }
    }

    bool connect(const std::string& socketPath = "/tmp/netd.sock") {
        auto& logger = Logger::getInstance();
        
        // Use the shared YANG context
        struct ly_ctx* ctx = yang_->getContext();

        // Connect to NETCONF server via Unix socket
        session_ = nc_connect_unix(socketPath.c_str(), ctx);
        if (!session_) {
            logger.error("Failed to connect to NETD server at " + socketPath);
            return false;
        }

        connected_ = true;
        logger.info("Connected to NETD server at " + socketPath);
        return true;
    }

    void disconnect() {
        if (session_) {
            nc_session_free(session_, nullptr);
            session_ = nullptr;
        }
        connected_ = false;
    }

    bool isConnected() const {
        return connected_;
    }

    // Send a NETCONF request and receive response
    std::string sendRequest(const std::string& request) {
        if (!connected_) {
            throw std::runtime_error("Not connected to server");
        }

        auto& logger = Logger::getInstance();
        
        // TODO: Parse request string to nc_rpc and send via nc_send_rpc
        // For now, return a placeholder response
        logger.debug("Sending request: " + request);
        return "NETCONF response placeholder";
    }

    // Convenience methods for common NETCONF operations
    netd::Response getConfig(const std::string& source = "running") {
        if (!connected_) {
            throw std::runtime_error("Not connected to server");
        }

        // Convert source string to NC_DATASTORE
        NC_DATASTORE ncSource = NC_DATASTORE_RUNNING;
        if (source == "candidate") {
            ncSource = NC_DATASTORE_CANDIDATE;
        } else if (source == "startup") {
            ncSource = NC_DATASTORE_STARTUP;
        }

        // Create get-config RPC
        struct nc_rpc* rpc = nc_rpc_getconfig(ncSource, nullptr, NC_WD_UNKNOWN, NC_PARAMTYPE_CONST);
        if (!rpc) {
            return netd::Response("1", "Failed to create get-config RPC", false);
        }

        // Send RPC and get message ID
        uint64_t msgid;
        NC_MSG_TYPE msgtype = nc_send_rpc(session_, rpc, 1000, &msgid);
        if (msgtype != NC_MSG_RPC) {
            nc_rpc_free(rpc);
            return netd::Response("1", "Failed to send get-config RPC", false);
        }

        // Receive reply
        struct lyd_node* envp = nullptr;
        struct lyd_node* op = nullptr;
        msgtype = nc_recv_reply(session_, rpc, msgid, 1000, &envp, &op);
        
        nc_rpc_free(rpc);
        
        if (msgtype == NC_MSG_REPLY) {
            // Success - extract data from op
            std::string data = op ? "Configuration data" : "";
            lyd_free_tree(envp);
            lyd_free_tree(op);
            return netd::Response(std::to_string(msgid), data, true);
        } else {
            // Error
            lyd_free_tree(envp);
            lyd_free_tree(op);
            return netd::Response(std::to_string(msgid), "get-config failed", false);
        }
    }

    netd::Response editConfig(const std::string& target = "candidate", const std::string& config = "") {
        if (!connected_) {
            throw std::runtime_error("Not connected to server");
        }

        // Convert target string to NC_DATASTORE
        NC_DATASTORE ncTarget = NC_DATASTORE_CANDIDATE;
        if (target == "running") {
            ncTarget = NC_DATASTORE_RUNNING;
        } else if (target == "startup") {
            ncTarget = NC_DATASTORE_STARTUP;
        }

        // TODO: Parse config string to lyd_node
        // For now, create empty config
        struct lyd_node* configData = nullptr;

        // Create edit-config RPC
        struct nc_rpc* rpc = nc_rpc_edit(ncTarget, NC_RPC_EDIT_DFLTOP_MERGE, NC_RPC_EDIT_TESTOPT_SET, NC_RPC_EDIT_ERROPT_STOP, nullptr, NC_PARAMTYPE_CONST);
        if (!rpc) {
            return netd::Response("1", "Failed to create edit-config RPC", false);
        }

        // Send RPC and get message ID
        uint64_t msgid;
        NC_MSG_TYPE msgtype = nc_send_rpc(session_, rpc, 1000, &msgid);
        if (msgtype != NC_MSG_RPC) {
            nc_rpc_free(rpc);
            return netd::Response("1", "Failed to send edit-config RPC", false);
        }

        // Receive reply
        struct lyd_node* envp = nullptr;
        struct lyd_node* op = nullptr;
        msgtype = nc_recv_reply(session_, rpc, msgid, 1000, &envp, &op);
        
        nc_rpc_free(rpc);
        
        if (msgtype == NC_MSG_REPLY) {
            // Success
            lyd_free_tree(envp);
            lyd_free_tree(op);
            return netd::Response(std::to_string(msgid), "", true);
        } else {
            // Error
            lyd_free_tree(envp);
            lyd_free_tree(op);
            return netd::Response(std::to_string(msgid), "edit-config failed", false);
        }
    }

    netd::Response commit() {
        if (!connected_) {
            throw std::runtime_error("Not connected to server");
        }

        // Create commit RPC
        struct nc_rpc* rpc = nc_rpc_commit(0, 0, nullptr, nullptr, NC_PARAMTYPE_CONST);
        if (!rpc) {
            return netd::Response("1", "Failed to create commit RPC", false);
        }

        // Send RPC and get message ID
        uint64_t msgid;
        NC_MSG_TYPE msgtype = nc_send_rpc(session_, rpc, 1000, &msgid);
        if (msgtype != NC_MSG_RPC) {
            nc_rpc_free(rpc);
            return netd::Response("1", "Failed to send commit RPC", false);
        }

        // Receive reply
        struct lyd_node* envp = nullptr;
        struct lyd_node* op = nullptr;
        msgtype = nc_recv_reply(session_, rpc, msgid, 1000, &envp, &op);
        
        nc_rpc_free(rpc);
        
        if (msgtype == NC_MSG_REPLY) {
            // Success
            lyd_free_tree(envp);
            lyd_free_tree(op);
            return netd::Response(std::to_string(msgid), "", true);
        } else {
            // Error
            lyd_free_tree(envp);
            lyd_free_tree(op);
            return netd::Response(std::to_string(msgid), "commit failed", false);
        }
    }

private:
    struct nc_session* session_;
    std::unique_ptr<YangAbstract> yang_;
    bool connected_;
};

// Global NETCONF client instance
static NetconfClient g_netconfClient;

// Public interface functions
bool connectToServer(const std::string& socketPath) {
    return g_netconfClient.connect(socketPath);
}

void disconnectFromServer() {
    g_netconfClient.disconnect();
}

bool isConnectedToServer() {
    return g_netconfClient.isConnected();
}

std::string sendNetconfRequest(const std::string& request) {
    return g_netconfClient.sendRequest(request);
}

netd::Response getConfig(const std::string& source) {
    return g_netconfClient.getConfig(source);
}

netd::Response editConfig(const std::string& target, const std::string& config) {
    return g_netconfClient.editConfig(target, config);
}

netd::Response commit() {
    return g_netconfClient.commit();
}

} // namespace netd
