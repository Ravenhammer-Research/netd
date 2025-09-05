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

#ifndef NETD_SERVER_NETCONF_BASE_HPP
#define NETD_SERVER_NETCONF_BASE_HPP

#include <memory>
#include <string>
#include <libyang/libyang.h>
#include <libnetconf2/netconf.h>
#include <libnetconf2/session_server.h>
#include <libnetconf2/messages_server.h>
#include <libnetconf2/server_config.h>
#include <shared/include/yang.hpp>

namespace netd::server::netconf {

	class Server {
	public:
		Server();
		virtual ~Server();

		// Server lifecycle methods
		virtual bool start() = 0;
		virtual void stop() = 0;
		bool isRunning() const;

		// Session management
		struct nc_session* acceptSession();
		void closeSession(struct nc_session* session);

		// RPC handling
		virtual struct nc_server_reply* handleRpc(struct nc_session* session, struct lyd_node* rpc);

		// Specific RPC request handlers
		virtual struct nc_server_reply* handleGetRequest(struct nc_session* session, struct lyd_node* rpc);
		virtual struct nc_server_reply* handleGetConfigRequest(struct nc_session* session, struct lyd_node* rpc);
		virtual struct nc_server_reply* handleEditConfigRequest(struct nc_session* session, struct lyd_node* rpc);
		virtual struct nc_server_reply* handleCopyConfigRequest(struct nc_session* session, struct lyd_node* rpc);
		virtual struct nc_server_reply* handleDeleteConfigRequest(struct nc_session* session, struct lyd_node* rpc);
		virtual struct nc_server_reply* handleLockRequest(struct nc_session* session, struct lyd_node* rpc);
		virtual struct nc_server_reply* handleUnlockRequest(struct nc_session* session, struct lyd_node* rpc);
		virtual struct nc_server_reply* handleDiscardRequest(struct nc_session* session, struct lyd_node* rpc);

	protected:
		bool running_;
		struct nc_pollsession* ps_;
	};

} // namespace netd::netconf

#endif // NETD_SERVER_NETCONF_BASE_HPP
