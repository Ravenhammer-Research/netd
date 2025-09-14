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

#ifndef NETD_SERVER_NETCONF_TRANSPORT_HPP
#define NETD_SERVER_NETCONF_TRANSPORT_HPP

#include <string>
#include <memory>
#include <iostream>
#include <functional>

namespace netd::shared {

  enum class TransportType {
    UNIX,
    SCTP,
    HTTP,
    SCTPS,
    HTTPS
  };

  class BaseTransport {
  public:
    BaseTransport() = default;
    virtual ~BaseTransport() = default;

    // Factory method for creating transports
    static std::unique_ptr<BaseTransport> create(TransportType type);
    
    // Address formatting for different transport types
    static std::string formatAddress(TransportType type, const std::string& bind_address, int port);

    // Transport lifecycle
    virtual bool start(const std::string& address) = 0;
    virtual void stop() = 0;
    virtual bool isListening() const = 0;

    // Connection management
    virtual int acceptConnection() = 0;
    virtual void closeConnection(int socket_fd) = 0;

    // Client-side methods
    virtual bool connect(const std::string& address) = 0;
    virtual void disconnect() = 0;
    virtual int getSocket() const = 0;

    // Communication
    virtual bool sendData(int socket_fd, const std::string& data) = 0;
    virtual std::string receiveData(int socket_fd) = 0;
    virtual bool hasData(int socket_fd) = 0;
    
    // Cancellation support
    virtual void cancelOperation(int socket_fd) = 0;

    // Transport properties
    virtual const std::string& getAddress() const = 0;
  };

} // namespace netd::shared

#endif // NETD_SERVER_NETCONF_TRANSPORT_HPP
