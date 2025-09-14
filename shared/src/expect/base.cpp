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

#include <shared/include/expect/base.hpp>
#include <shared/include/logger.hpp>

namespace netd::shared::expect {

  ExpectBase::ExpectBase(const std::string& message_id, 
                         const std::string& session_id, 
                         std::chrono::seconds ttl)
      : message_id_(message_id), session_id_(session_id), ttl_(ttl), 
        creation_time_(std::chrono::steady_clock::now()) {
  }

  bool ExpectBase::matchesMessageId(const std::string& message_id) const {
    return message_id_ == message_id;
  }

  bool ExpectBase::matchesSessionId(const std::string& session_id) const {
    return session_id_ == session_id;
  }

  bool ExpectBase::isExpired() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - creation_time_);
    return elapsed >= ttl_;
  }

  std::chrono::steady_clock::time_point ExpectBase::getCreationTime() const {
    return creation_time_;
  }

  std::chrono::seconds ExpectBase::getTtl() const {
    return ttl_;
  }

  const std::string& ExpectBase::getMessageId() const {
    return message_id_;
  }

  const std::string& ExpectBase::getSessionId() const {
    return session_id_;
  }

} // namespace netd::shared::expect
