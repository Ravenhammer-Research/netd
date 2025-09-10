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

#include <shared/include/netconf/session.hpp>
#include <shared/include/logger.hpp>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <cstring>
#include <algorithm>

namespace netd::shared::netconf {

  // Static member initialization
  std::atomic<uint64_t> NetconfSession::next_session_id_{1};

  NetconfSession::NetconfSession(ly_ctx* ctx)
      : ctx_(ctx), message_id_counter_(0), connected_(true) {
    session_id_ = next_session_id_++;
    
    auto &logger = Logger::getInstance();
    logger.info("Created new NETCONF session with ID: " + std::to_string(session_id_));
  }

  NetconfSession::~NetconfSession() {
    close();
  }

  bool NetconfSession::isConnected() const {
    return connected_;
  }

  void NetconfSession::close() {
    if (connected_) {
      connected_ = false;
      
      auto &logger = Logger::getInstance();
      logger.info("Closed NETCONF session ID: " + std::to_string(session_id_));
    }
  }


  // SessionManager implementation
  SessionManager& SessionManager::getInstance() {
    static SessionManager instance;
    return instance;
  }

  void SessionManager::addSession(std::unique_ptr<NetconfSession> session) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_.push_back(std::move(session));
    
    auto &logger = Logger::getInstance();
    logger.info("Added session to manager. Total sessions: " + std::to_string(sessions_.size()));
  }

  void SessionManager::removeSession(uint64_t session_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    
    auto it = std::find_if(sessions_.begin(), sessions_.end(),
                          [session_id](const std::unique_ptr<NetconfSession>& session) {
                            return session->getSessionId() == session_id;
                          });
    
    if (it != sessions_.end()) {
      (*it)->close();
      sessions_.erase(it);
      
      auto &logger = Logger::getInstance();
      logger.info("Removed session " + std::to_string(session_id) + 
                 ". Total sessions: " + std::to_string(sessions_.size()));
    }
  }

  NetconfSession* SessionManager::getSession(uint64_t session_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    
    auto it = std::find_if(sessions_.begin(), sessions_.end(),
                          [session_id](const std::unique_ptr<NetconfSession>& session) {
                            return session->getSessionId() == session_id;
                          });
    
    return (it != sessions_.end()) ? it->get() : nullptr;
  }

  std::vector<NetconfSession*> SessionManager::getAllSessions() {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    
    std::vector<NetconfSession*> result;
    for (const auto& session : sessions_) {
      result.push_back(session.get());
    }
    
    return result;
  }

  void SessionManager::closeAllSessions() {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    
    for (auto& session : sessions_) {
      session->close();
    }
    sessions_.clear();
    
    auto &logger = Logger::getInstance();
    logger.info("Closed all NETCONF sessions");
  }

} // namespace netd::shared::netconf
