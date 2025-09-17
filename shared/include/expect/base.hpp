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

#ifndef NETD_SHARED_EXPECT_BASE_HPP
#define NETD_SHARED_EXPECT_BASE_HPP

#include <chrono>
#include <functional>
#include <libyang/libyang.h>
#include <memory>
#include <string>

namespace netd::shared::expect {

  /**
   * @brief Base class for all Expect objects
   * Handles async response waiting with callbacks and TTL
   */
  class ExpectBase {
  public:
    /**
     * @brief Constructor
     * @param message_id The NETCONF message ID to expect
     * @param session_id The NETCONF session ID
     * @param ttl Time to live in seconds (default: 8 seconds)
     */
    ExpectBase(const std::string &message_id, const std::string &session_id,
               std::chrono::seconds ttl = std::chrono::seconds(8));

    /**
     * @brief Virtual destructor
     */
    virtual ~ExpectBase() = default;

    /**
     * @brief Check if this expect matches the given message ID
     * @param message_id The message ID to check
     * @return true if matches, false otherwise
     */
    bool matchesMessageId(const std::string &message_id) const;

    /**
     * @brief Check if this expect matches the given session ID
     * @param session_id The session ID to check
     * @return true if matches, false otherwise
     */
    bool matchesSessionId(const std::string &session_id) const;

    /**
     * @brief Check if this expect has expired
     * @return true if expired, false otherwise
     */
    bool isExpired() const;

    /**
     * @brief Get the creation time
     * @return The time when this expect was created
     */
    std::chrono::steady_clock::time_point getCreationTime() const;

    /**
     * @brief Get the TTL
     * @return The time to live in seconds
     */
    std::chrono::seconds getTtl() const;

    /**
     * @brief Get the message ID
     * @return The expected message ID
     */
    const std::string &getMessageId() const;

    /**
     * @brief Get the session ID
     * @return The expected session ID
     */
    const std::string &getSessionId() const;

    /**
     * @brief Process a response (to be implemented by derived classes)
     * @param response_node The YANG response node
     * @return true if the response was handled, false otherwise
     */
    virtual bool processResponse(lyd_node *response_node) = 0;

  private:
    std::string message_id_;
    std::string session_id_;
    std::chrono::seconds ttl_;
    std::chrono::steady_clock::time_point creation_time_;
  };

  /**
   * @brief Template class for typed Expect objects
   * @tparam ResponseType The type of response data expected
   */
  template <typename ResponseType> class Expect : public ExpectBase {
  public:
    using CallbackType = std::function<void(const ResponseType &)>;

    /**
     * @brief Constructor
     * @param callback The callback function to call when response is received
     * @param message_id The NETCONF message ID to expect
     * @param session_id The NETCONF session ID
     * @param ttl Time to live in seconds (default: 8 seconds)
     */
    Expect(CallbackType callback, const std::string &message_id,
           const std::string &session_id,
           std::chrono::seconds ttl = std::chrono::seconds(8))
        : ExpectBase(message_id, session_id, ttl), callback_(callback) {}

    /**
     * @brief Process a response
     * @param response_node The YANG response node
     * @return true if the response was handled, false otherwise
     */
    bool processResponse(lyd_node *response_node) override {
      if (!response_node) {
        return false;
      }

      try {
        // Convert YANG node to ResponseType
        ResponseType response_data = convertFromYang(response_node);

        // Call the callback with the converted response
        if (callback_) {
          callback_(response_data);
        }

        return true;
      } catch (const std::exception &e) {
        // Log error and return false
        return false;
      }
    }

  private:
    CallbackType callback_;

    /**
     * @brief Convert YANG node to ResponseType
     * This method needs to be specialized for each ResponseType
     * @param response_node The YANG response node
     * @return The converted response data
     */
    ResponseType convertFromYang(lyd_node *response_node) {
      // Default implementation - should be specialized
      (void)response_node; // Suppress unused parameter warning
      throw std::runtime_error(
          "convertFromYang not implemented for this response type");
    }
  };

} // namespace netd::shared::expect

#endif // NETD_SHARED_EXPECT_BASE_HPP
