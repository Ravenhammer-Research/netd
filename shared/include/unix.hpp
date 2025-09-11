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

#ifndef NETD_SERVER_NETCONF_UNIX_HPP
#define NETD_SERVER_NETCONF_UNIX_HPP

#include <string>
#include <shared/include/transport.hpp>

namespace netd::shared {

  class UnixTransport : public BaseTransport {
  private:
    std::string socketPath_;
    bool listening_;
    int server_socket_;

  public:
    UnixTransport();
    ~UnixTransport();

    // BaseTransport interface
    bool start(const std::string &address) override;
    void stop() override;
    bool isListening() const override;
    int acceptConnection() override;
    void closeConnection(int socket_fd) override;
    bool sendData(int socket_fd, const std::string& data) override;
    std::string receiveData(int socket_fd) override;
    const std::string& getAddress() const override;

    // Legacy methods for compatibility
    const std::string &getSocketPath() const;
    int getServerSocket() const;

  private:
    bool createServerSocket();
    bool prepareSocketFile();
    bool checkSocketDirectory();
  };

} // namespace netd::shared

#endif // NETD_SERVER_NETCONF_UNIX_HPP
