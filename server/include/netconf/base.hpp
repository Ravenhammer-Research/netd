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

#ifndef NETD_SERVER_NETCONF_BASE_HPP
#define NETD_SERVER_NETCONF_BASE_HPP

#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#ifdef HAVE_LLDP
#include <shared/include/lldp/client.hpp>
#endif

namespace netd::server::netconf {

  /**
   * @brief Base mixin class for NETCONF server functionality
   *
   * This class serves as a mixin that can be inherited by other classes
   * to provide NETCONF server capabilities through multiple inheritance.
   * Includes LLDP functionality when available.
   */
  class Server {
  public:
    Server() = default;
    virtual ~Server() = default;

    /**
     * @brief Initialize LLDP client and configuration
     * @return true if LLDP initialization successful or not available, false on
     * error
     */
    virtual bool initializeLLDP();

    /**
     * @brief Cleanup LLDP client
     */
    virtual void cleanupLLDP();

  protected:
#ifdef HAVE_LLDP
    std::unique_ptr<netd::shared::lldp::Client> lldp_client_;
#endif
  };

} // namespace netd::server::netconf

#endif // NETD_SERVER_NETCONF_BASE_HPP