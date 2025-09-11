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

#include <server/include/netconf/server.hpp>
#include <shared/include/exception.hpp>

namespace netd::server::netconf {

  NetconfServer::NetconfServer(netd::shared::TransportType transport_type, 
                               const std::string& bind_address, 
                               int port)
      : transport_type_(transport_type), bind_address_(bind_address), port_(port), running_(false) {
    (void)transport_type;
    (void)bind_address;
    (void)port;
    throw netd::shared::NotImplementedError("NetconfServer constructor not implemented");
  }

  NetconfServer::~NetconfServer() {
    // Destructor can be empty for stubbed implementation
  }

  std::unique_ptr<netd::shared::BaseTransport> NetconfServer::createTransport() {
    throw netd::shared::NotImplementedError("createTransport not implemented");
  }

  bool NetconfServer::start() {
    throw netd::shared::NotImplementedError("NetconfServer start not implemented");
  }

  void NetconfServer::stop() {
    throw netd::shared::NotImplementedError("NetconfServer stop not implemented");
  }

  void NetconfServer::run() {
    throw netd::shared::NotImplementedError("NetconfServer run not implemented");
  }

  void NetconfServer::handleClientSession(std::unique_ptr<netd::shared::netconf::NetconfSession> session) {
    (void)session;
    throw netd::shared::NotImplementedError("handleClientSession not implemented");
  }

} // namespace netd::server::netconf