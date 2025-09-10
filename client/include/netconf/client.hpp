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

#include <shared/include/netconf/session.hpp>
#include <shared/include/netconf/unix.hpp>
#include <memory>
#include <string>
#include <atomic>

namespace netd::client::netconf {

  class NetconfClient {
  public:
    NetconfClient();
    ~NetconfClient();

    // Connection management
    bool connect(const std::string& socket_path);
    void disconnect();
    bool isConnected() const;
    
    // Raw socket communication methods
    bool sendData(const void* data, size_t len);
    std::string receiveData();
    std::string readXmlMessage();
    
  private:
    std::unique_ptr<netd::shared::netconf::UnixTransport> transport_;
    std::atomic<bool> connected_;
    int socket_fd_;
  };

} // namespace netd::client::netconf

#endif // NETD_CLIENT_NETCONF_CLIENT_HPP
