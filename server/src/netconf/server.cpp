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

#include <libnetconf2/session_server.h>
#include <libnetconf2/messages_server.h>
#include <libnetconf2/netconf.h>
#include <libnetconf2/server_config.h>
#include <server/include/netconf/base.hpp>
#include <server/include/netconf/handlers.hpp>
#include <shared/include/logger.hpp>
#include <memory>
#include <string>
#include <signal.h>
#include <unistd.h>

namespace netd::server::netconf {

	using netd::shared::Logger;
    using netd::shared::Yang;
	using netd::server::netconf::handlers::RpcHandler;

	class NetconfServer : public netd::server::netconf::Server {
	private:
		std::string socketPath_;

	public:
		NetconfServer() {
			Yang::getInstance();
		}

		~NetconfServer() {
			stop();
		}

		bool start() override {
			auto& logger = Logger::getInstance();
			
			// Get the context from the base class
			struct ly_ctx* serverCtx = Yang::getInstance().getContext();
			
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

			// Add Unix socket endpoint
			if (nc_server_add_endpt_unix_socket_listen("netd", "/tmp/netd.sock", 0666, -1, -1) != 0) {
				logger.error("Failed to add Unix socket endpoint");
				nc_ps_free(ps_);
				nc_server_destroy();
				return false;
			}

			running_ = true;
			logger.info("NETCONF server started");
			return true;
		}

		void stop() override {
			if (!isRunning()) return;
			
			auto& logger = Logger::getInstance();
			running_ = false;

			nc_server_destroy();

			logger.info("NETCONF server stopped");
		}

		struct nc_server_reply* handleGetRequest(struct nc_session* session, struct lyd_node* rpc) override {
			return RpcHandler::handleGetRequest(session, rpc);
		}

		struct nc_server_reply* handleGetConfigRequest(struct nc_session* session, struct lyd_node* rpc) override {
			return RpcHandler::handleGetConfigRequest(session, rpc);
		}

		struct nc_server_reply* handleEditConfigRequest(struct nc_session* session, struct lyd_node* rpc) override {
			return RpcHandler::handleEditConfigRequest(session, rpc);
		}

		struct nc_server_reply* handleCopyConfigRequest(struct nc_session* session, struct lyd_node* rpc) override {
			return RpcHandler::handleCopyConfigRequest(session, rpc);
		}

		struct nc_server_reply* handleDeleteConfigRequest(struct nc_session* session, struct lyd_node* rpc) override {
			return RpcHandler::handleDeleteConfigRequest(session, rpc);
		}

		struct nc_server_reply* handleLockRequest(struct nc_session* session, struct lyd_node* rpc) override {
			return RpcHandler::handleLockRequest(session, rpc);
		}

		struct nc_server_reply* handleUnlockRequest(struct nc_session* session, struct lyd_node* rpc) override {
			return RpcHandler::handleUnlockRequest(session, rpc);
		}

		struct nc_server_reply* handleDiscardRequest(struct nc_session* session, struct lyd_node* rpc) override {
			return RpcHandler::handleDiscardRequest(session, rpc);
		}

		void run() {
			auto& logger = Logger::getInstance();
			logger.info("Starting server main loop");
			
			while (isRunning()) {
				int no_new_sessions = 0;
				
				// Try to accept new NETCONF sessions
				struct nc_session* newSession = acceptSession();
				
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
				struct nc_session* session = nullptr;
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
				if (no_new_sessions && (ret & (NC_PSPOLL_TIMEOUT | NC_PSPOLL_NOSESSIONS))) {
					usleep(10000);  // 10ms sleep
				}
			}
		}
	};

	// Global server instance
	static std::unique_ptr<NetconfServer> g_server;

	bool startNetconfServer(const std::string& socketPath) {
		g_server = std::make_unique<NetconfServer>();
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
