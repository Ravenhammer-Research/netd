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

#ifndef NETD_SHARED_EXPECT_INTERFACE_HPP
#define NETD_SHARED_EXPECT_INTERFACE_HPP

#include <libyang/libyang.h>
#include <shared/include/expect/base.hpp>
#include <string>
#include <vector>

namespace netd::shared::expect {

  /**
   * @brief Response data structure for interface list
   */
  struct InterfaceResponse {
    std::vector<std::string> interface_names;
  };

  /**
   * @brief Specialized Expect class for interface responses
   */
  class InterfaceExpect : public Expect<InterfaceResponse> {
  public:
    using CallbackType = std::function<void(const InterfaceResponse &)>;

    /**
     * @brief Constructor
     * @param callback The callback function to call when interface response is
     * received
     * @param message_id The NETCONF message ID to expect
     * @param session_id The NETCONF session ID
     * @param ttl Time to live in seconds (default: 8 seconds)
     */
    InterfaceExpect(CallbackType callback, const std::string &message_id,
                    const std::string &session_id,
                    std::chrono::seconds ttl = std::chrono::seconds(8))
        : Expect<InterfaceResponse>(callback, message_id, session_id, ttl) {}

  private:
    /**
     * @brief Convert YANG node to InterfaceResponse
     * @param response_node The YANG response node
     * @return The converted interface response data
     */
    InterfaceResponse convertFromYang(lyd_node *response_node);
  };

} // namespace netd::shared::expect

#endif // NETD_SHARED_EXPECT_INTERFACE_HPP
