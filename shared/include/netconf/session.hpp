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

#ifndef NETD_NETCONF_SESSION_HPP
#define NETD_NETCONF_SESSION_HPP

#include <libyang/libyang.h>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>

namespace netd::shared::netconf {

  class NetconfSession {
  public:
    NetconfSession(ly_ctx* ctx);
    ~NetconfSession();

    // Session management
    bool isConnected() const;
    void close();
    
    // Session properties
    uint64_t getSessionId() const { return session_id_; }
    const std::string& getCapabilities() const { return capabilities_; }
    void setCapabilities(const std::string& caps) { capabilities_ = caps; }
    
    // YANG context access
    ly_ctx* getContext() const { return ctx_; }
    
    // Message ID management
    uint64_t getNextMessageId() { return ++message_id_counter_; }
    

  private:
    uint64_t session_id_;
    ly_ctx* ctx_;
    std::string capabilities_;
    std::atomic<uint64_t> message_id_counter_;
    bool connected_;
    
    static std::atomic<uint64_t> next_session_id_;
  };

  // Session manager for tracking active sessions
  class SessionManager {
  public:
    static SessionManager& getInstance();
    
    void addSession(std::unique_ptr<NetconfSession> session);
    void removeSession(uint64_t session_id);
    NetconfSession* getSession(uint64_t session_id);
    std::vector<NetconfSession*> getAllSessions();
    void closeAllSessions();
    
  private:
    SessionManager() = default;
    std::vector<std::unique_ptr<NetconfSession>> sessions_;
    std::mutex sessions_mutex_;
  };

} // namespace netd::shared::netconf

#endif // NETD_NETCONF_SESSION_HPP
