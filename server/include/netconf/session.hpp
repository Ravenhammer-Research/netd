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

#ifndef NETD_SERVER_NETCONF_SESSION_HPP
#define NETD_SERVER_NETCONF_SESSION_HPP

#include <memory>
#include <mutex>
#include <shared/include/netconf/session.hpp>
#include <vector>

namespace netd::server::netconf {

  // Session manager for tracking active server sessions
  class SessionManager {
  public:
    static SessionManager &getInstance();

    // Session management
    void
    addSession(std::unique_ptr<netd::shared::netconf::NetconfSession> session);
    void removeSession(int session_id);
    netd::shared::netconf::NetconfSession *getSession(int session_id);
    std::vector<netd::shared::netconf::NetconfSession *> getAllSessions();
    std::vector<netd::shared::netconf::NetconfSession *>
    getSessionsByTransportType(netd::shared::TransportType transport_type);
    void closeAllSessions();
    void
    closeSessionsByTransportType(netd::shared::TransportType transport_type);

    // Session statistics
    size_t getSessionCount() const;
    size_t getSessionsByTransportTypeCount(
        netd::shared::TransportType transport_type) const;

    // Session lookup by client info (for server sessions)
    netd::shared::netconf::NetconfSession *
    findSessionByClient(const std::string &username, const std::string &host);
    std::vector<netd::shared::netconf::NetconfSession *>
    findSessionsByClient(const std::string &username);

    // Unix socket specific - find session by user ID
    netd::shared::netconf::NetconfSession *findSessionByUserId(uid_t user_id);

  private:
    SessionManager() = default;
    std::vector<std::unique_ptr<netd::shared::netconf::NetconfSession>>
        sessions_;
    mutable std::mutex sessions_mutex_;
  };

} // namespace netd::server::netconf

#endif // NETD_SERVER_NETCONF_SESSION_HPP
