#include <libnetconf2/session_server.h>
#include <libnetconf2/messages_server.h>
#include <libnetconf2/netconf.h>
#include <libnetconf2/server_config.h>
#include <shared/include/yang.hpp>
#include <shared/include/request.hpp>
#include <shared/include/response.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/exception.hpp>
#include <memory>
#include <string>
#include <signal.h>
#include <unistd.h>

namespace netd {

class NetconfServer {
private:
    std::unique_ptr<YangAbstract> yang_;
    struct ly_ctx* serverCtx_;
    struct nc_pollsession* ps_;
    bool running_;
    std::string socketPath_;

public:
    NetconfServer() : serverCtx_(nullptr), ps_(nullptr), running_(false) {
        yang_ = createYang();
    }

    ~NetconfServer() {
        stop();
    }

    bool start(const std::string& socketPath) {
        auto& logger = Logger::getInstance();
        
        socketPath_ = socketPath;
        
        // Create a libyang context
        // Use the existing YANG context from yang_ object
        serverCtx_ = yang_->getContext();

        // Implement the base NETCONF modules
        if (nc_server_init_ctx(&serverCtx_) != 0) {
            logger.error("Failed to initialize context");
            return false;
        }

        // Load all required modules for configuration
        if (nc_server_config_load_modules(&serverCtx_) != 0) {
            logger.error("Failed to load server configuration modules");
            return false;
        }

        // Initialize the server
        if (nc_server_init() != 0) {
            logger.error("Failed to initialize libnetconf2 server");
            return false;
        }

        // Create pollsession for managing sessions
        ps_ = nc_ps_new();
        if (!ps_) {
            logger.error("Failed to create pollsession");
            nc_server_destroy();
            return false;
        }

        // Add Unix socket endpoint using the old API (this version doesn't have the new endpoint functions)
        if (nc_server_add_endpt_unix_socket_listen("netd", socketPath.c_str(), 0666, -1, -1) != 0) {
            logger.error("Failed to add Unix socket endpoint: " + socketPath);
            nc_ps_free(ps_);
            nc_server_destroy();
            return false;
        }

        // Set up RPC callbacks
        nc_set_global_rpc_clb(globalRpcCallback);

        running_ = true;
        logger.info("NETCONF server started on " + socketPath);
        return true;
    }

    void stop() {
        if (!running_) return;
        
        auto& logger = Logger::getInstance();
        running_ = false;

        if (ps_) {
            nc_ps_clear(ps_, 1, nullptr);  // Free all sessions
            nc_ps_free(ps_);
            ps_ = nullptr;
        }

        nc_server_destroy();
        // Don't destroy serverCtx_ since it's owned by yang_ object
        serverCtx_ = nullptr;
        logger.info("NETCONF server stopped");
    }

    void run() {
        auto& logger = Logger::getInstance();
        logger.info("Starting server main loop");
        
        while (running_) {
            int no_new_sessions = 0;
            
            // Try to accept new NETCONF sessions on all configured endpoints
            struct nc_session* newSession = nullptr;
            NC_MSG_TYPE msgType = nc_accept(0, serverCtx_, &newSession);  // Non-blocking
            
            switch (msgType) {
            case NC_MSG_HELLO:
                logger.info("New session accepted");
                if (nc_ps_add_session(ps_, newSession) != 0) {
                    logger.error("Failed to add session to poll");
                    nc_session_free(newSession, nullptr);
                }
                break;
                
            case NC_MSG_WOULDBLOCK:
                no_new_sessions = 1;
                break;
                
            case NC_MSG_BAD_HELLO:
                logger.error("Bad hello message received");
                if (newSession) {
                    nc_session_free(newSession, nullptr);
                }
                break;
                
            case NC_MSG_ERROR:
                logger.error("Error accepting new session");
                if (newSession) {
                    nc_session_free(newSession, nullptr);
                }
                break;
                
            default:
                logger.debug("nc_accept returned: " + std::to_string(msgType));
                break;
            }
            
            // Poll all sessions and process events
            struct nc_session* session = nullptr;
            int ret = nc_ps_poll(ps_, 0, &session);  // Non-blocking
            
            if (ret & NC_PSPOLL_ERROR) {
                logger.error("Polling error occurred");
                break;
            }
            
            if (ret & NC_PSPOLL_SESSION_TERM) {
                logger.info("Session terminated");
                if (session) {
                    nc_ps_del_session(ps_, session);
                    nc_session_free(session, nullptr);
                }
            }
            
            if (ret & NC_PSPOLL_SESSION_ERROR) {
                logger.error("Session error occurred");
                if (session) {
                    nc_ps_del_session(ps_, session);
                    nc_session_free(session, nullptr);
                }
            }
            
            // Prevent active waiting by sleeping when no activity
            if (no_new_sessions && (ret & (NC_PSPOLL_TIMEOUT | NC_PSPOLL_NOSESSIONS))) {
                usleep(10000);  // 10ms sleep
            }
        }
    }

private:
    static struct nc_server_reply* globalRpcCallback(struct lyd_node* rpc, struct nc_session* session) {
        auto& logger = Logger::getInstance();
        
        try {
            // Parse the RPC request
            Request request = Request::fromYang(nc_session_get_ctx(session), rpc);
            logger.info("Processing RPC: " + request.getType() + " (data: " + request.getData() + ")");
        
        // Create appropriate response based on request type
        if (request.getType() == "get") {
            return handleGetRequest(request, session);
        } else if (request.getType() == "get-config") {
            return handleGetConfigRequest(request, session);
        } else if (request.getType() == "edit-config") {
            return handleEditConfigRequest(request, session);
        } else if (request.getType() == "close-session") {
            return nc_server_reply_ok();
        } else {
            // Unsupported operation
            logger.warning("Unsupported RPC type: " + request.getType());
            return nc_server_reply_ok(); // Just return OK for now
        }
        
        } catch (const netd::NotImplementedError& e) {
            logger.error("NotImplementedError: " + std::string(e.what()));
            return nc_server_reply_ok(); // Return OK for now, but log the error
        } catch (const std::exception& e) {
            logger.error("Exception in RPC callback: " + std::string(e.what()));
            return nc_server_reply_ok(); // Return OK for now, but log the error
        }
    }

    static struct nc_server_reply* handleGetRequest(const Request& request, struct nc_session* session) {
        auto& logger = Logger::getInstance();
        
        // For now, return a simple OK response
        // TODO: Implement actual data retrieval based on request filters
        logger.info("Handling get request");
        return nc_server_reply_ok();
    }

    static struct nc_server_reply* handleGetConfigRequest(const Request& request, struct nc_session* session) {
        auto& logger = Logger::getInstance();
        
        // For now, return a simple OK response
        // TODO: Implement actual configuration retrieval
        logger.info("Handling get-config request");
        return nc_server_reply_ok();
    }

    static struct nc_server_reply* handleEditConfigRequest(const Request& request, struct nc_session* session) {
        auto& logger = Logger::getInstance();
        
        // For now, return a simple OK response
        // TODO: Implement actual configuration editing
        logger.info("Handling edit-config request");
        return nc_server_reply_ok();
    }
};

// Global server instance
static std::unique_ptr<NetconfServer> g_server;

bool startNetconfServer(const std::string& socketPath) {
    g_server = std::make_unique<NetconfServer>();
    return g_server->start(socketPath);
}

void stopNetconfServer() {
    if (g_server) {
        g_server->stop();
        g_server.reset();
    }
}

void runNetconfServer() {
    if (g_server) {
        g_server->run();
    }
}

} // namespace netd
