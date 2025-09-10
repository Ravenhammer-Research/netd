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
#include <libyang/libyang.h>
#include <server/include/netconf/handlers.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/request/session/close.hpp>
#include <shared/include/response/close.hpp>
#include <shared/include/netconf/session.hpp>

namespace netd::server::netconf::handlers {

  std::unique_ptr<netd::shared::response::CloseResponse>
  RpcHandler::handleCloseSessionRequest(
      std::unique_ptr<netd::shared::request::session::CloseRequest> request) {
    auto &logger = netd::shared::Logger::getInstance();
    auto response = std::make_unique<netd::shared::response::CloseResponse>();

    logger.info("Handling close-session request");

    // Get the session from the request
    auto session = request->getSession();
    if (!session) {
      logger.error("No session found in close-session request");
      response->setProtocolError(netd::shared::marshalling::ErrorTag::OPERATION_FAILED,
                                "No session found in close-session request");
      return response;
    }

    // Close the session gracefully
    session->close();
    
    // Remove session from session manager
    auto &sessionManager = netd::shared::netconf::SessionManager::getInstance();
    sessionManager.removeSession(session->getSessionId());

    logger.info("Session closed: " + std::to_string(session->getSessionId()));

    return response;
  }

} // namespace netd::server::netconf::handlers
