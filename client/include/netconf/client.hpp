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

#ifndef NETD_CLIENT_NETCONF_CLIENT_HPP
#define NETD_CLIENT_NETCONF_CLIENT_HPP

#include <memory>
#include <string>
#include <shared/include/transport.hpp>
#include <shared/include/netconf/session.hpp>
#include <client/include/netconf/base.hpp>

namespace netd::client::netconf {

  class NetconfClient : public NetconfBase {
  public:
    /**
     * @brief Constructs a NetconfClient for the specified server
     * @param transport_type Type of transport to use
     * @param server_address Address of the server (path for UNIX, hostname for others)
     * @param port Port number (ignored for UNIX transport)
     */
    NetconfClient(netd::shared::TransportType transport_type, 
                  const std::string& server_address, 
                  int port = 0);
    
    /**
     * @brief Destructor - automatically disconnects if connected
     */
    ~NetconfClient();

    /**
     * @brief Connects to the NETCONF server
     * @return true if connection successful, false otherwise
     * @throws TransportError if connection fails
     */
    bool connect();

    /**
     * @brief Disconnects from the NETCONF server
     * @param close_session Whether to close the NETCONF session (true) or just disconnect transport (false)
     */
    void disconnect(bool close_session = true);

    /**
     * @brief Checks if the client is connected to the server
     * @return true if connected, false otherwise
     */
    bool isConnected() const;


    /**
     * @brief Gets the current NETCONF session
     * @return Pointer to the session, or nullptr if not connected
     */
    netd::shared::netconf::NetconfSession* getSession() const;

    /**
     * @brief Handles the server session in a loop, processing incoming RPC messages
     * This method runs continuously until the session is disconnected
     */
    void handleServerSession();

  private:
    netd::shared::TransportType transport_type_;
    std::string server_address_;
    int port_;
    bool connected_;
    std::unique_ptr<netd::shared::BaseTransport> transport_;
    std::unique_ptr<netd::shared::netconf::NetconfSession> session_;
  };

} // namespace netd::client::netconf

#endif // NETD_CLIENT_NETCONF_CLIENT_HPP
