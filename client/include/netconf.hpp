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

#ifndef NETD_CLIENT_NETCONF_HPP
#define NETD_CLIENT_NETCONF_HPP

#include <shared/include/request/edit.hpp>
#include <shared/include/request/get/base.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/response/get/config.hpp>
#include <libnetconf2/netconf.h>
#include <string>

namespace netd::client {

  // Connection management
  bool connectToServer(const std::string &socketPath = "/var/run/netd.sock");
  void disconnectFromServer();
  bool isConnectedToServer();


  // Client class
  class NetconfClient {
  public:
    NetconfClient();
    ~NetconfClient();
    
    bool connect(const std::string &socketPath = "/tmp/netd.sock");
    void disconnect();
    bool isConnected() const;
    struct nc_session *getSession() const;
    
    netd::shared::response::get::GetConfigResponse sendRequest(struct nc_rpc *rpc);

  private:
    struct nc_session *session_;
    bool connected_;
  };
  
  NetconfClient &getNetconfClient();

} // namespace netd::client

#endif // NETD_CLIENT_NETCONF_HPP