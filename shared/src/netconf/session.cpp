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

#include <algorithm>
#include <cstring>
#include <errno.h>
#include <shared/include/logger.hpp>
#include <shared/include/netconf/session.hpp>
#include <shared/include/request/hello.hpp>
#include <sys/socket.h>
#include <unistd.h>

namespace netd::shared::netconf {

  NetconfSession::NetconfSession(ly_ctx *ctx, int socket,
                                 netd::shared::TransportType transport_type)
      : ctx_(ctx), message_id_counter_(0), connected_(true), socket_(socket),
        user_id_(0), transport_type_(transport_type) {

    auto &logger = Logger::getInstance();
    logger.info("Created new NETCONF session with socket: " +
                std::to_string(socket));
  }

  NetconfSession::~NetconfSession() { close(); }

  bool NetconfSession::isConnected() const { return connected_; }

  void NetconfSession::close() {
    if (connected_) {
      connected_ = false;

      // Close the socket if it's valid
      if (socket_ >= 0) {
        auto &logger = Logger::getInstance();
        logger.info("Closed NETCONF session with socket: " +
                    std::to_string(socket_));
        ::close(socket_);
        socket_ = -1;
      }
    }
  }

  void NetconfSession::processHelloRequest(
      const netd::shared::request::HelloRequest &hello_request) {
    auto &logger = Logger::getInstance();
    logger.info("Processing hello request for session: " +
                std::to_string(socket_));

    // Update session state to indicate hello was received
    setState(SessionState::HELLO_RECEIVED);

    // Extract and store client capabilities from the hello request
    setCapabilities(hello_request.getCapabilities());

    logger.debug("Hello request processed successfully");
  }

  void NetconfSession::setState(SessionState state) { state_ = state; }

} // namespace netd::shared::netconf
