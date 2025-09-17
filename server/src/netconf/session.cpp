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

#include <algorithm>
#include <server/include/netconf/session.hpp>
#include <shared/include/logger.hpp>

namespace netd::server::netconf {

  // SessionManager implementation
  SessionManager &SessionManager::getInstance() {
    static SessionManager instance;
    return instance;
  }

  void SessionManager::addSession(
      std::unique_ptr<netd::shared::netconf::NetconfSession> session) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_.push_back(std::move(session));

    auto &logger = netd::shared::Logger::getInstance();
    logger.debug("Added session to manager. Total sessions: " +
                 std::to_string(sessions_.size()));
  }

  void SessionManager::removeSession(int session_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    auto it = std::find_if(
        sessions_.begin(), sessions_.end(),
        [session_id](
            const std::unique_ptr<netd::shared::netconf::NetconfSession>
                &session) { return session->getSessionId() == session_id; });

    if (it != sessions_.end()) {
      (*it)->close();
      sessions_.erase(it);

      auto &logger = netd::shared::Logger::getInstance();
      logger.debug("Removed session " + std::to_string(session_id) +
                   ". Total sessions: " + std::to_string(sessions_.size()));
    }
  }

  netd::shared::netconf::NetconfSession *
  SessionManager::getSession(int session_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    auto it = std::find_if(
        sessions_.begin(), sessions_.end(),
        [session_id](
            const std::unique_ptr<netd::shared::netconf::NetconfSession>
                &session) { return session->getSessionId() == session_id; });

    return (it != sessions_.end()) ? it->get() : nullptr;
  }

  std::vector<netd::shared::netconf::NetconfSession *>
  SessionManager::getAllSessions() {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    std::vector<netd::shared::netconf::NetconfSession *> result;
    for (const auto &session : sessions_) {
      result.push_back(session.get());
    }

    return result;
  }

  void SessionManager::closeAllSessions() {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    for (auto &session : sessions_) {
      session->close();
    }
    sessions_.clear();

    auto &logger = netd::shared::Logger::getInstance();
    logger.debug("Closed all NETCONF sessions");
  }

  std::vector<netd::shared::netconf::NetconfSession *>
  SessionManager::getSessionsByTransportType(
      netd::shared::TransportType transport_type) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    std::vector<netd::shared::netconf::NetconfSession *> result;
    for (const auto &session : sessions_) {
      if (session->getTransportType() == transport_type) {
        result.push_back(session.get());
      }
    }

    return result;
  }

  void SessionManager::closeSessionsByTransportType(
      netd::shared::TransportType transport_type) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    auto it = sessions_.begin();
    while (it != sessions_.end()) {
      if ((*it)->getTransportType() == transport_type) {
        (*it)->close();
        it = sessions_.erase(it);
      } else {
        ++it;
      }
    }
  }

  size_t SessionManager::getSessionsByTransportTypeCount(
      netd::shared::TransportType transport_type) const {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    size_t count = 0;
    for (const auto &session : sessions_) {
      if (session->getTransportType() == transport_type) {
        count++;
      }
    }
    return count;
  }

  netd::shared::netconf::NetconfSession *
  SessionManager::findSessionByUserId(uid_t user_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    for (const auto &session : sessions_) {
      if (session->getUserId() == user_id) {
        return session.get();
      }
    }

    return nullptr;
  }

} // namespace netd::server::netconf
