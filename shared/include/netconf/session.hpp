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
#include <unordered_map>
#include <atomic>
#include <mutex>

namespace netd::shared::netconf {

  enum class SocketType {
    UNIX,
    HTTP,
    SCTP,
    HTTPS,
    SCTPS
  };

  enum class SessionState {
    INITIALIZING,
    HELLO_SENT,
    HELLO_RECEIVED,
    ACTIVE,
    CLOSING,
    CLOSED
  };

  class NetconfSession {
  public:
    NetconfSession(ly_ctx* ctx, int socket = -1, SocketType socket_type = SocketType::UNIX);
    ~NetconfSession();

    // Session management
    bool isConnected() const;
    void close();
    void setState(SessionState state);
    SessionState getState() const { return state_; }
    
    // Session properties (using socket as ID)
    int getSessionId() const { return socket_; }
    const std::vector<std::string>& getCapabilities() const { return capabilities_; }
    void setCapabilities(const std::vector<std::string>& caps) { capabilities_ = caps; }
    
    // YANG context access
    ly_ctx* getContext() const { return ctx_; }
    
    // Message ID management
    uint64_t getNextMessageId() { return ++message_id_counter_; }
    
    
    // Socket management
    int getSocket() const { return socket_; }
    SocketType getSocketType() const { return socket_type_; }

  private:
    ly_ctx* ctx_;
    SessionState state_;
    std::vector<std::string> capabilities_;
    std::atomic<uint64_t> message_id_counter_;
    bool connected_;
    int socket_;
    SocketType socket_type_;
  };


} // namespace netd::shared::netconf

#endif // NETD_NETCONF_SESSION_HPP
