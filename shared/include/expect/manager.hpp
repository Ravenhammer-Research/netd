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

#ifndef NETD_SHARED_EXPECT_MANAGER_HPP
#define NETD_SHARED_EXPECT_MANAGER_HPP

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <shared/include/expect/base.hpp>
#include <thread>
#include <vector>

namespace netd::shared::expect {

  /**
   * @brief Manager class for handling async NETCONF responses
   * Manages a collection of Expect objects and processes incoming responses
   */
  class ExpectManager {
  public:
    /**
     * @brief Constructor
     */
    ExpectManager();

    /**
     * @brief Destructor - stops the cleanup thread
     */
    ~ExpectManager();

    /**
     * @brief Add an Expect object to be managed
     * @param expect The Expect object to add
     */
    void addExpect(std::shared_ptr<ExpectBase> expect);

    /**
     * @brief Process an incoming response
     * @param message_id The message ID of the response
     * @param session_id The session ID of the response
     * @param response_node The YANG response node
     * @return true if the response was handled by an Expect, false otherwise
     */
    bool processResponse(const std::string &message_id,
                         const std::string &session_id,
                         lyd_node *response_node);

    /**
     * @brief Remove an Expect object by message ID
     * @param message_id The message ID to remove
     * @return true if removed, false if not found
     */
    bool removeExpect(const std::string &message_id);

    /**
     * @brief Get the number of active Expect objects
     * @return The count of active Expect objects
     */
    size_t getActiveExpectCount() const;

    /**
     * @brief Start the cleanup thread (automatically called)
     */
    void startCleanupThread();

    /**
     * @brief Stop the cleanup thread
     */
    void stopCleanupThread();

    /**
     * @brief Check if the manager is running
     * @return true if running, false otherwise
     */
    bool isRunning() const;

  private:
    std::vector<std::shared_ptr<ExpectBase>> expects_;
    mutable std::mutex expects_mutex_;

    std::atomic<bool> running_;
    std::thread cleanup_thread_;
    std::condition_variable cleanup_cv_;
    mutable std::mutex cleanup_mutex_;

    /**
     * @brief Cleanup expired Expect objects
     */
    void cleanupExpiredExpects();

    /**
     * @brief Cleanup thread function
     */
    void cleanupThreadFunction();
  };

} // namespace netd::shared::expect

#endif // NETD_SHARED_EXPECT_MANAGER_HPP
