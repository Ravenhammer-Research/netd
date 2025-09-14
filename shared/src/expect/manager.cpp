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

#include <shared/include/expect/manager.hpp>
#include <shared/include/logger.hpp>
#include <algorithm>
#include <chrono>

namespace netd::shared::expect {

  ExpectManager::ExpectManager() : running_(false) {
    startCleanupThread();
  }

  ExpectManager::~ExpectManager() {
    stopCleanupThread();
  }

  void ExpectManager::addExpect(std::shared_ptr<ExpectBase> expect) {
    if (!expect) {
      return;
    }

    std::lock_guard<std::mutex> lock(expects_mutex_);
    expects_.push_back(expect);
    
    auto& logger = netd::shared::Logger::getInstance();
    logger.debug("ExpectManager - Added expect for message ID: " + expect->getMessageId());
  }

  bool ExpectManager::processResponse(const std::string& message_id,
                                    const std::string& session_id,
                                    lyd_node* response_node) {
    std::lock_guard<std::mutex> lock(expects_mutex_);
    
    // Find matching expect
    auto it = std::find_if(expects_.begin(), expects_.end(),
        [&message_id, &session_id](const std::shared_ptr<ExpectBase>& expect) {
          return expect->matchesMessageId(message_id) && 
                 expect->matchesSessionId(session_id) &&
                 !expect->isExpired();
        });

    if (it != expects_.end()) {
      auto& logger = netd::shared::Logger::getInstance();
      logger.debug("ExpectManager - Processing response for message ID: " + message_id);
      
      // Process the response
      bool handled = (*it)->processResponse(response_node);
      
      // Remove the expect after processing
      expects_.erase(it);
      
      return handled;
    }

    return false;
  }

  bool ExpectManager::removeExpect(const std::string& message_id) {
    std::lock_guard<std::mutex> lock(expects_mutex_);
    
    auto it = std::find_if(expects_.begin(), expects_.end(),
        [&message_id](const std::shared_ptr<ExpectBase>& expect) {
          return expect->matchesMessageId(message_id);
        });

    if (it != expects_.end()) {
      expects_.erase(it);
      return true;
    }

    return false;
  }

  size_t ExpectManager::getActiveExpectCount() const {
    std::lock_guard<std::mutex> lock(expects_mutex_);
    return expects_.size();
  }

  void ExpectManager::startCleanupThread() {
    if (running_) {
      return;
    }

    running_ = true;
    cleanup_thread_ = std::thread(&ExpectManager::cleanupThreadFunction, this);
    
    auto& logger = netd::shared::Logger::getInstance();
    logger.debug("ExpectManager - Started cleanup thread");
  }

  void ExpectManager::stopCleanupThread() {
    if (!running_) {
      return;
    }

    running_ = false;
    cleanup_cv_.notify_all();
    
    if (cleanup_thread_.joinable()) {
      cleanup_thread_.join();
    }
    
    auto& logger = netd::shared::Logger::getInstance();
    logger.debug("ExpectManager - Stopped cleanup thread");
  }

  bool ExpectManager::isRunning() const {
    return running_;
  }

  void ExpectManager::cleanupExpiredExpects() {
    std::lock_guard<std::mutex> lock(expects_mutex_);
    
    auto it = std::remove_if(expects_.begin(), expects_.end(),
        [](const std::shared_ptr<ExpectBase>& expect) {
          return expect->isExpired();
        });

    size_t removed_count = std::distance(it, expects_.end());
    expects_.erase(it, expects_.end());
    
    if (removed_count > 0) {
      auto& logger = netd::shared::Logger::getInstance();
      logger.debug("ExpectManager - Cleaned up " + std::to_string(removed_count) + " expired expects");
    }
  }

  void ExpectManager::cleanupThreadFunction() {
    auto& logger = netd::shared::Logger::getInstance();
    logger.debug("ExpectManager - Cleanup thread started");
    
    while (running_) {
      std::unique_lock<std::mutex> lock(cleanup_mutex_);
      
      // Wait for 1 second or until notified to stop
      cleanup_cv_.wait_for(lock, std::chrono::seconds(1), [this] { return !running_; });
      
      if (running_) {
        cleanupExpiredExpects();
      }
    }
    
    logger.debug("ExpectManager - Cleanup thread stopped");
  }

} // namespace netd::shared::expect
