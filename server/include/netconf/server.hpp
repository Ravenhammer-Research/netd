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

#ifndef NETD_SERVER_NETCONF_SERVER_HPP
#define NETD_SERVER_NETCONF_SERVER_HPP

#include <memory>
#include <server/include/netconf/base.hpp>
#include <server/include/netconf/handlers.hpp>
#include <server/include/netconf/rpc.hpp>
#include <shared/include/netconf/session.hpp>
#include <shared/include/transport.hpp>
#include <shared/include/yang.hpp>
#include <string>
#include <thread>
#include <vector>

namespace netd::server::netconf {

  class NetconfServer : public Server {
  public:
    NetconfServer(netd::shared::TransportType transport_type,
                  const std::string &bind_address, int port);
    ~NetconfServer();

    bool start();
    void stop();
    void run();
    bool isListening() const;

  private:
    netd::shared::TransportType transport_type_ [[maybe_unused]];
    std::string bind_address_ [[maybe_unused]];
    int port_ [[maybe_unused]];
    std::unique_ptr<netd::shared::BaseTransport> transport_;
    std::vector<std::thread> session_threads_;

    // Server management
    std::unique_ptr<netd::shared::BaseTransport> createTransport();
    void accept();
    netd::shared::netconf::NetconfSession *
    handleClientSession(const netd::shared::ClientSocket &client_socket);
    void rpcReceive(netd::shared::RpcRxStream &rpc_stream,
                    netd::shared::netconf::NetconfSession *session);
    void processRpcRequest(netd::shared::netconf::NetconfSession &session,
                           const std::string &data);
    void sendErrorResponse(int client_socket, const std::string &error_message);
  };

} // namespace netd::server::netconf

#endif // NETD_SERVER_NETCONF_SERVER_HPP