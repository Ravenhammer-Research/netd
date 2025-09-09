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

#include <libnetconf2/messages_server.h>
#include <libnetconf2/netconf.h>
#include <libnetconf2/server_config.h>
#include <libnetconf2/session_server.h>
#include <memory>
#include <server/include/netconf/base.hpp>
#include <server/include/netconf/handlers.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/request/close.hpp>
#include <shared/include/request/commit.hpp>
#include <shared/include/request/copy.hpp>
#include <shared/include/request/delete.hpp>
#include <shared/include/request/discard.hpp>
#include <shared/include/request/edit.hpp>
#include <shared/include/request/get/base.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/request/hello.hpp>
#include <shared/include/request/kill.hpp>
#include <shared/include/request/lock.hpp>
#include <shared/include/request/unlock.hpp>
#include <shared/include/request/validate.hpp>
#include <signal.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <cstring>

namespace netd::server::netconf {

  using netd::server::netconf::handlers::RpcHandler;
  using netd::shared::Logger;
  using netd::shared::Yang;

  // Helper function to check if running as root
  static bool isRunningAsRoot() {
    return geteuid() == 0;
  }

  // Helper function to check and clean up existing socket file
  static bool prepareSocketFile(const std::string &socketPath) {
    auto &logger = Logger::getInstance();
    
    // Check if socket file already exists
    struct stat st;
    if (stat(socketPath.c_str(), &st) == 0) {
      // Check if it's actually a socket file
      if (!S_ISSOCK(st.st_mode)) {
        logger.error("Socket path exists but is not a socket file: " + socketPath);
        return false;
      }
      
      // Try to remove existing socket file
      if (unlink(socketPath.c_str()) != 0) {
        logger.error("Failed to remove existing socket file " + socketPath + ": " + std::string(strerror(errno)));
        return false;
      }
      
      logger.info("Removed existing socket file: " + socketPath);
    }
    
    return true;
  }

  // Helper function to check socket directory permissions
  static bool checkSocketDirectory(const std::string &socketPath) {
    auto &logger = Logger::getInstance();
    
    // Extract directory from socket path
    size_t lastSlash = socketPath.find_last_of('/');
    if (lastSlash == std::string::npos) {
      logger.error("Invalid socket path (no directory): " + socketPath);
      return false;
    }
    
    std::string dirPath = socketPath.substr(0, lastSlash);
    
    // Check if directory exists and is writable
    struct stat st;
    if (stat(dirPath.c_str(), &st) != 0) {
      logger.error("Socket directory does not exist: " + dirPath);
      return false;
    }
    
    if (!S_ISDIR(st.st_mode)) {
      logger.error("Socket path is not a directory: " + dirPath);
      return false;
    }
    
    // Check write permissions
    if (access(dirPath.c_str(), W_OK) != 0) {
      logger.error("No write permission to socket directory: " + dirPath + " (" + std::string(strerror(errno)) + ")");
      return false;
    }
    
    return true;
  }

  // Forward declaration
  class NetconfServer;

  // Global server instance
  static std::unique_ptr<NetconfServer> g_server;

  class NetconfServer : public netd::server::netconf::Server {
  private:
    std::string socketPath_;

    // Static wrapper for the RPC callback
    static struct nc_server_reply *rpcCallback(struct lyd_node *rpc,
                                               struct nc_session *session) {
      auto &logger = netd::shared::Logger::getInstance();
      
      // Print the incoming RPC request
      char *xmlStr = nullptr;
      if (lyd_print_mem(&xmlStr, rpc, LYD_XML, LYD_PRINT_WITHSIBLINGS) == LY_SUCCESS) {
        logger.debug("Incoming RPC request: " + std::string(xmlStr));
        free(xmlStr);
      }
      
      if (!g_server) {
        throw netd::shared::ArgumentError("Server instance not available");
      }

      if (!session || !rpc) {
        throw netd::shared::ArgumentError("Invalid session or RPC parameters");
      }

      // Get the RPC operation name
      const char *rpcName = LYD_NAME(rpc);
      if (!rpcName) {
        throw netd::shared::ArgumentError("Failed to get RPC operation name");
      }

      try {
        // Dispatch to appropriate handler based on RPC name
        if (strcmp(rpcName, "get") == 0) {
          return g_server->handleGetRequest(session, rpc);
        } else if (strcmp(rpcName, "get-config") == 0) {
          return g_server->handleGetConfigRequest(session, rpc);
        } else if (strcmp(rpcName, "edit-config") == 0) {
          return g_server->handleEditConfigRequest(session, rpc);
        } else if (strcmp(rpcName, "copy-config") == 0) {
          return g_server->handleCopyConfigRequest(session, rpc);
        } else if (strcmp(rpcName, "delete-config") == 0) {
          return g_server->handleDeleteConfigRequest(session, rpc);
        } else if (strcmp(rpcName, "lock") == 0) {
          return g_server->handleLockRequest(session, rpc);
        } else if (strcmp(rpcName, "unlock") == 0) {
          return g_server->handleUnlockRequest(session, rpc);
        } else if (strcmp(rpcName, "discard-changes") == 0) {
          return g_server->handleDiscardRequest(session, rpc);
        } else if (strcmp(rpcName, "close-session") == 0) {
          return g_server->handleCloseSessionRequest(session, rpc);
        } else if (strcmp(rpcName, "kill-session") == 0) {
          return g_server->handleKillSessionRequest(session, rpc);
        } else if (strcmp(rpcName, "validate") == 0) {
          return g_server->handleValidateRequest(session, rpc);
        } else if (strcmp(rpcName, "hello") == 0) {
          return g_server->handleHelloRequest(session, rpc);
        } else if (strcmp(rpcName, "commit") == 0) {
          return g_server->handleCommitRequest(session, rpc);
        } else {
          // Unknown RPC
          throw netd::shared::NotImplementedError("Unknown RPC operation: " +
                                                  std::string(rpcName));
        }
      } catch (const netd::shared::ArgumentError &e) {
        return nc_server_reply_err(
            nc_err(netd::shared::Yang::getInstance().getContext(),
                   NC_ERR_INVALID_VALUE, NC_ERR_TYPE_APP, e.what()));
      } catch (const netd::shared::NotImplementedError &e) {
        return nc_server_reply_err(
            nc_err(netd::shared::Yang::getInstance().getContext(),
                   NC_ERR_OP_NOT_SUPPORTED, NC_ERR_TYPE_APP, e.what()));
      } catch (const std::runtime_error &e) {
        return nc_server_reply_err(
            nc_err(netd::shared::Yang::getInstance().getContext(),
                   NC_ERR_OP_FAILED, NC_ERR_TYPE_APP, e.what()));
      } catch (const std::exception &e) {
        return nc_server_reply_err(
            nc_err(netd::shared::Yang::getInstance().getContext(),
                   NC_ERR_OP_FAILED, NC_ERR_TYPE_APP, e.what()));
      }
    }

  public:
    NetconfServer(const std::string &socketPath) : socketPath_(socketPath) { 
      Yang::getInstance(); 
    }

    ~NetconfServer() { stop(); }

    bool start() override {
      auto &logger = Logger::getInstance();

      // Check if running as root (required for UNIX socket binding)
      if (!isRunningAsRoot()) {
        logger.error("NETCONF server must be run as root to bind to UNIX socket");
        return false;
      }

      // Check socket directory permissions
      if (!checkSocketDirectory(socketPath_)) {
        logger.error("Socket directory permission check failed");
        return false;
      }

      // Prepare socket file (remove existing if present)
      if (!prepareSocketFile(socketPath_)) {
        logger.error("Failed to prepare socket file");
        return false;
      }

      // Get the context from the base class
      struct ly_ctx *serverCtx = Yang::getInstance().getContext();

      if (!serverCtx) {
        logger.error("No context available");
        return false;
      }

      // Implement the base NETCONF modules
      if (nc_server_init_ctx(&serverCtx) != 0) {
        logger.error("Failed to initialize context");
        return false;
      }

      // Load all required modules for configuration
      if (nc_server_config_load_modules(&serverCtx) != 0) {
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

      // Add Unix socket endpoint using the provided socket path
      if (nc_server_add_endpt_unix_socket_listen("netd", socketPath_.c_str(), 0666,
                                                 -1, -1) != 0) {
        logger.error("Failed to add Unix socket endpoint: " + socketPath_);
        nc_ps_free(ps_);
        nc_server_destroy();
        return false;
      }

      // Register the global RPC callback
      nc_set_global_rpc_clb(rpcCallback);

      logger.info("NETCONF server started successfully on socket: " + socketPath_);
      running_ = true;
      return true;
    }

    void stop() override {
      if (!isRunning()) {
        return; // Already stopped, no need to throw exception
      }
        
      auto &logger = Logger::getInstance();
      running_ = false;

      // Clean up socket file
      if (!socketPath_.empty()) {
        if (unlink(socketPath_.c_str()) == 0) {
          logger.info("Removed socket file: " + socketPath_);
        } else if (errno != ENOENT) {
          logger.warning("Failed to remove socket file " + socketPath_ + ": " + std::string(strerror(errno)));
        }
      }

      nc_server_destroy();
      logger.info("NETCONF server stopped");
    }

    struct nc_server_reply *handleGetRequest(struct nc_session *session,
                                             struct lyd_node *rpc) override {
      auto request = std::make_unique<netd::shared::request::get::GetRequest>(
          session, rpc);

      auto response = RpcHandler::handleGetRequest(std::move(request));

      return response->toNetconfReply(session);
    }

    struct nc_server_reply *
    handleGetConfigRequest(struct nc_session *session,
                           struct lyd_node *rpc) override {
      auto request =
          std::make_unique<netd::shared::request::get::GetConfigRequest>(
              session, rpc);

      auto response = RpcHandler::handleGetConfigRequest(std::move(request));

      // Log the response as XML before converting to NETCONF reply
      auto &logger = netd::shared::Logger::getInstance();
      auto &yang = netd::shared::Yang::getInstance();
      lyd_node *responseNode = response->toYang(yang.getContext());
      if (responseNode) {
        char *xmlStr = nullptr;
        if (lyd_print_mem(&xmlStr, responseNode, LYD_XML, LYD_PRINT_WITHSIBLINGS) == LY_SUCCESS) {
          logger.debug("Outgoing RPC reply: " + std::string(xmlStr));
          free(xmlStr);
        }
        lyd_free_tree(responseNode);
      }

      return response->toNetconfReply(session);
    }

    struct nc_server_reply *
    handleEditConfigRequest(struct nc_session *session,
                            struct lyd_node *rpc) override {
      auto request = std::make_unique<netd::shared::request::EditConfigRequest>(
          session, rpc);

      auto response = RpcHandler::handleEditConfigRequest(std::move(request));

      return response->toNetconfReply(session);
    }

    struct nc_server_reply *
    handleCopyConfigRequest(struct nc_session *session,
                            struct lyd_node *rpc) override {
      auto request = std::make_unique<netd::shared::request::CopyConfigRequest>(
          session, rpc);

      auto response = RpcHandler::handleCopyConfigRequest(std::move(request));

      return response->toNetconfReply(session);
    }

    struct nc_server_reply *
    handleDeleteConfigRequest(struct nc_session *session,
                              struct lyd_node *rpc) override {
      auto request =
          std::make_unique<netd::shared::request::DeleteConfigRequest>(session,
                                                                       rpc);

      auto response = RpcHandler::handleDeleteConfigRequest(std::move(request));

      return response->toNetconfReply(session);
    }

    struct nc_server_reply *handleLockRequest(struct nc_session *session,
                                              struct lyd_node *rpc) override {
      auto request =
          std::make_unique<netd::shared::request::LockRequest>(session, rpc);

      auto response = RpcHandler::handleLockRequest(std::move(request));

      return response->toNetconfReply(session);
    }

    struct nc_server_reply *handleUnlockRequest(struct nc_session *session,
                                                struct lyd_node *rpc) override {
      auto request =
          std::make_unique<netd::shared::request::UnlockRequest>(session, rpc);

      auto response = RpcHandler::handleUnlockRequest(std::move(request));

      return response->toNetconfReply(session);
    }

    struct nc_server_reply *
    handleDiscardRequest(struct nc_session *session,
                         struct lyd_node *rpc) override {
      auto request =
          std::make_unique<netd::shared::request::DiscardRequest>(session, rpc);

      auto response = RpcHandler::handleDiscardRequest(std::move(request));

      return response->toNetconfReply(session);
    }

    struct nc_server_reply *
    handleCloseSessionRequest(struct nc_session *session,
                              struct lyd_node *rpc) override {
      auto request =
          std::make_unique<netd::shared::request::CloseRequest>(session, rpc);

      auto response = RpcHandler::handleCloseSessionRequest(std::move(request));

      return response->toNetconfReply(session);
    }

    struct nc_server_reply *
    handleKillSessionRequest(struct nc_session *session,
                             struct lyd_node *rpc) override {
      auto request =
          std::make_unique<netd::shared::request::KillRequest>(session, rpc);

      auto response = RpcHandler::handleKillSessionRequest(std::move(request));

      return response->toNetconfReply(session);
    }

    struct nc_server_reply *
    handleValidateRequest(struct nc_session *session,
                          struct lyd_node *rpc) override {
      auto request = std::make_unique<netd::shared::request::ValidateRequest>(
          session, rpc);

      auto response = RpcHandler::handleValidateRequest(std::move(request));

      return response->toNetconfReply(session);
    }

    struct nc_server_reply *handleHelloRequest(struct nc_session *session,
                                               struct lyd_node *rpc) override {
      auto request =
          std::make_unique<netd::shared::request::HelloRequest>(session, rpc);

      auto response = RpcHandler::handleHelloRequest(std::move(request));

      return response->toNetconfReply(session);
    }

    struct nc_server_reply *handleCommitRequest(struct nc_session *session,
                                                struct lyd_node *rpc) override {
      auto request =
          std::make_unique<netd::shared::request::CommitRequest>(session, rpc);

      auto response = RpcHandler::handleCommitRequest(std::move(request));

      return response->toNetconfReply(session);
    }

    void run() {
      auto &logger = Logger::getInstance();
      logger.info("Starting server main loop");

      while (isRunning()) {
        int no_new_sessions = 0;

        // Try to accept new NETCONF sessions
        struct nc_session *newSession = acceptSession();

        if (newSession) {
          logger.info("New session accepted");
          if (nc_ps_add_session(ps_, newSession) != 0) {
            logger.error("Failed to add session to poll");
            closeSession(newSession);
          }
        } else {
          no_new_sessions = 1;
        }

        // Poll all sessions and process events
        struct nc_session *session = nullptr;
        int ret = nc_ps_poll(ps_, 0, &session);

        if (ret & NC_PSPOLL_ERROR) {
          logger.error("Polling error occurred");
          break;
        }

        if (ret & NC_PSPOLL_SESSION_TERM) {
          logger.info("Session terminated");
          if (session) {
            nc_ps_del_session(ps_, session);
            closeSession(session);
          }
        }

        if (ret & NC_PSPOLL_SESSION_ERROR) {
          logger.error("Session error occurred");
          if (session) {
            nc_ps_del_session(ps_, session);
            closeSession(session);
          }
        }

        // Prevent active waiting by sleeping when no activity
        if (no_new_sessions &&
            (ret & (NC_PSPOLL_TIMEOUT | NC_PSPOLL_NOSESSIONS))) {
          usleep(10000); // 10ms sleep
        }
      }
    }
  };

  bool startNetconfServer(const std::string &socketPath) {
    g_server = std::make_unique<NetconfServer>(socketPath);
    return g_server->start();
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

} // namespace netd::server::netconf
