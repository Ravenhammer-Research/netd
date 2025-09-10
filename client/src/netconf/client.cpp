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

#include <client/include/netconf/client.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/exception.hpp>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sstream>

namespace netd::client::netconf {

  NetconfClient::NetconfClient() 
      : connected_(false), socket_fd_(-1) {
    transport_ = std::make_unique<netd::shared::netconf::UnixTransport>();
  }

  NetconfClient::~NetconfClient() {
    disconnect();
  }

  bool NetconfClient::connect(const std::string& socket_path) {
    if (isConnected()) {
      disconnect();
    }

    // Create Unix domain socket
    socket_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
      return false;
    }

    // Set up socket address
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);

    // Connect to server
    if (::connect(socket_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
      close(socket_fd_);
      socket_fd_ = -1;
      return false;
    }

    connected_ = true;

    return true;
  }

  void NetconfClient::disconnect() {
    if (socket_fd_ >= 0) {
      close(socket_fd_);
      socket_fd_ = -1;
    }
    connected_ = false;
  }

  bool NetconfClient::isConnected() const {
    return connected_ && socket_fd_ >= 0;
  }


  bool NetconfClient::sendData(const void* data, size_t len) {
    if (socket_fd_ < 0) {
      return false;
    }

    ssize_t sent = 0;
    while (sent < static_cast<ssize_t>(len)) {
      ssize_t result = send(socket_fd_, 
                           static_cast<const char*>(data) + sent, 
                           len - sent, 0);
      if (result < 0) {
        return false;
      }
      sent += result;
    }
    return true;
  }

  std::string NetconfClient::receiveData() {
    if (socket_fd_ < 0) {
      return "";
    }

    char buffer[4096];
    ssize_t received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
      return "";
    }

    buffer[received] = '\0';
    return std::string(buffer);
  }

  std::string NetconfClient::readXmlMessage() {
    std::string message;
    std::string buffer;
    
    // Simple XML message reading - look for complete XML documents
    while (true) {
      buffer = receiveData();
      if (buffer.empty()) {
        break;
      }
      
      message += buffer;
      
      // Check if we have a complete XML message
      if (message.find("</hello>") != std::string::npos ||
          message.find("</rpc-reply>") != std::string::npos ||
          message.find("</rpc>") != std::string::npos) {
        break;
      }
    }
    
    return message;
  }


} // namespace netd::client::netconf
