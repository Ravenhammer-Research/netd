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

#include <shared/include/exception.hpp>
#include <shared/include/quic.hpp>

namespace netd::shared {

  QuicTransport::QuicTransport() : listen_address_(""), listen_port_(0) {}

  QuicTransport::~QuicTransport() { stop(); }

  // BaseTransport interface implementation
  bool QuicTransport::start(const std::string &address) {
    (void)address;
    throw netd::shared::NotImplementedError(
        "QuicTransport::start not implemented");
  }

  bool QuicTransport::start(const std::string &address, uint16_t port) {
    (void)address;
    (void)port;
    throw netd::shared::NotImplementedError(
        "QuicTransport::start not implemented");
  }

  void QuicTransport::stop() {
    throw netd::shared::NotImplementedError(
        "QuicTransport::stop not implemented");
  }

  bool QuicTransport::isListening() const {
    throw netd::shared::NotImplementedError(
        "QuicTransport::isListening not implemented");
  }

  int QuicTransport::acceptConnection() {
    throw netd::shared::NotImplementedError(
        "QuicTransport::acceptConnection not implemented");
  }

  void QuicTransport::closeConnection(int socket_fd) {
    (void)socket_fd;
    throw netd::shared::NotImplementedError(
        "QuicTransport::closeConnection not implemented");
  }

  bool QuicTransport::connect(const std::string &address) {
    (void)address;
    throw netd::shared::NotImplementedError(
        "QuicTransport::connect not implemented");
  }

  void QuicTransport::disconnect() {
    throw netd::shared::NotImplementedError(
        "QuicTransport::disconnect not implemented");
  }

  int QuicTransport::getSocket() const {
    throw netd::shared::NotImplementedError(
        "QuicTransport::getSocket not implemented");
  }

  bool QuicTransport::sendData(int socket_fd, const std::string &data) {
    (void)socket_fd;
    (void)data;
    throw netd::shared::NotImplementedError(
        "QuicTransport::sendData not implemented");
  }

  std::string QuicTransport::receiveData(int socket_fd) {
    (void)socket_fd;
    throw netd::shared::NotImplementedError(
        "QuicTransport::receiveData not implemented");
  }

  bool QuicTransport::hasData(int socket_fd) {
    (void)socket_fd;
    throw netd::shared::NotImplementedError(
        "QuicTransport::hasData not implemented");
  }

  void QuicTransport::cancelOperation(int socket_fd) {
    (void)socket_fd;
    throw netd::shared::NotImplementedError(
        "QuicTransport::cancelOperation not implemented");
  }

  const std::string &QuicTransport::getAddress() const {
    throw netd::shared::NotImplementedError(
        "QuicTransport::getAddress not implemented");
  }

  // QUIC-specific interface
  uint16_t QuicTransport::getPort() const {
    throw netd::shared::NotImplementedError(
        "QuicTransport::getPort not implemented");
  }

  void QuicTransport::setMaxStreamData(uint32_t max_data) {
    (void)max_data;
    throw netd::shared::NotImplementedError(
        "QuicTransport::setMaxStreamData not implemented");
  }

  void QuicTransport::setMaxData(uint32_t max_data) {
    (void)max_data;
    throw netd::shared::NotImplementedError(
        "QuicTransport::setMaxData not implemented");
  }

  void QuicTransport::setMaxStreams(uint32_t max_bidi, uint32_t max_uni) {
    (void)max_bidi;
    (void)max_uni;
    throw netd::shared::NotImplementedError(
        "QuicTransport::setMaxStreams not implemented");
  }

  void QuicTransport::setIdleTimeout(uint32_t timeout_ms) {
    (void)timeout_ms;
    throw netd::shared::NotImplementedError(
        "QuicTransport::setIdleTimeout not implemented");
  }

  size_t QuicTransport::getActiveConnections() const {
    throw netd::shared::NotImplementedError(
        "QuicTransport::getActiveConnections not implemented");
  }

  std::vector<QuicConnectionId> QuicTransport::getConnectionIds() const {
    throw netd::shared::NotImplementedError(
        "QuicTransport::getConnectionIds not implemented");
  }

  bool QuicTransport::sendData(QuicConnectionId connection_id,
                               QuicStreamId stream_id,
                               const std::vector<uint8_t> &data) {
    (void)connection_id;
    (void)stream_id;
    (void)data;
    throw netd::shared::NotImplementedError(
        "QuicTransport::sendData not implemented");
  }

  std::vector<uint8_t>
  QuicTransport::receiveData(QuicConnectionId connection_id,
                             QuicStreamId stream_id) {
    (void)connection_id;
    (void)stream_id;
    throw netd::shared::NotImplementedError(
        "QuicTransport::receiveData not implemented");
  }

  bool QuicTransport::closeStream(QuicConnectionId connection_id,
                                  QuicStreamId stream_id) {
    (void)connection_id;
    (void)stream_id;
    throw netd::shared::NotImplementedError(
        "QuicTransport::closeStream not implemented");
  }

} // namespace netd::shared
