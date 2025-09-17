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
#include <shared/include/sctp.hpp>

namespace netd::shared {

  SCTPTransport::SCTPTransport(SCTPMode mode)
      : mode_(mode), primary_address_(""), port_(0), listening_(false),
        running_(false), server_socket_(-1) {}

  SCTPTransport::~SCTPTransport() { stop(); }

  // BaseTransport interface implementation
  bool SCTPTransport::start(const std::string &address) {
    (void)address;
    throw netd::shared::NotImplementedError(
        "SCTPTransport::start not implemented");
  }

  bool SCTPTransport::start(const std::string &address, int port) {
    (void)address;
    (void)port;
    throw netd::shared::NotImplementedError(
        "SCTPTransport::start not implemented");
  }

  void SCTPTransport::stop() {
    throw netd::shared::NotImplementedError(
        "SCTPTransport::stop not implemented");
  }

  bool SCTPTransport::isListening() const {
    throw netd::shared::NotImplementedError(
        "SCTPTransport::isListening not implemented");
  }

  int SCTPTransport::acceptConnection() {
    throw netd::shared::NotImplementedError(
        "SCTPTransport::acceptConnection not implemented");
  }

  void SCTPTransport::closeConnection(int socket_fd) {
    (void)socket_fd;
    throw netd::shared::NotImplementedError(
        "SCTPTransport::closeConnection not implemented");
  }

  bool SCTPTransport::connect(const std::string &address) {
    (void)address;
    throw netd::shared::NotImplementedError(
        "SCTPTransport::connect not implemented");
  }

  void SCTPTransport::disconnect() {
    throw netd::shared::NotImplementedError(
        "SCTPTransport::disconnect not implemented");
  }

  int SCTPTransport::getSocket() const {
    throw netd::shared::NotImplementedError(
        "SCTPTransport::getSocket not implemented");
  }

  bool SCTPTransport::sendData(int socket_fd, const std::string &data) {
    (void)socket_fd;
    (void)data;
    throw netd::shared::NotImplementedError(
        "SCTPTransport::sendData not implemented");
  }

  std::string SCTPTransport::receiveData(int socket_fd) {
    (void)socket_fd;
    throw netd::shared::NotImplementedError(
        "SCTPTransport::receiveData not implemented");
  }

  void SCTPTransport::cancelOperation(int socket_fd) {
    (void)socket_fd;
    throw netd::shared::NotImplementedError(
        "SCTPTransport::cancelOperation not implemented");
  }

  const std::string &SCTPTransport::getAddress() const {
    throw netd::shared::NotImplementedError(
        "SCTPTransport::getAddress not implemented");
  }

  // Additional SCTP-specific methods
  bool SCTPTransport::isRunning() const {
    throw netd::shared::NotImplementedError(
        "SCTPTransport::isRunning not implemented");
  }

  int SCTPTransport::getPort() const {
    throw netd::shared::NotImplementedError(
        "SCTPTransport::getPort not implemented");
  }

  std::vector<std::string> SCTPTransport::getConnectedPeers() const {
    throw netd::shared::NotImplementedError(
        "SCTPTransport::getConnectedPeers not implemented");
  }

  // Multihoming methods
  bool SCTPTransport::addLocalAddress(const std::string &address,
                                      bool is_primary) {
    (void)address;
    (void)is_primary;
    throw netd::shared::NotImplementedError(
        "SCTPTransport::addLocalAddress not implemented");
  }

  bool SCTPTransport::removeLocalAddress(const std::string &address) {
    (void)address;
    throw netd::shared::NotImplementedError(
        "SCTPTransport::removeLocalAddress not implemented");
  }

  std::vector<SCTPAddress> SCTPTransport::getLocalAddresses() const {
    throw netd::shared::NotImplementedError(
        "SCTPTransport::getLocalAddresses not implemented");
  }

  std::vector<SCTPAddress>
  SCTPTransport::getRemoteAddresses(int socket_fd) const {
    (void)socket_fd;
    throw netd::shared::NotImplementedError(
        "SCTPTransport::getRemoteAddresses not implemented");
  }

  // SCTP-AUTH methods (for use with DTLS wrapper)
  bool SCTPTransport::enableSCTPAuth() {
    throw netd::shared::NotImplementedError(
        "SCTPTransport::enableSCTPAuth not implemented");
  }

  bool SCTPTransport::setSCTPAuthKey(const std::string &key) {
    (void)key;
    throw netd::shared::NotImplementedError(
        "SCTPTransport::setSCTPAuthKey not implemented");
  }

  // Stream management (RFC 6083)
  bool SCTPTransport::sendDataOnStream(int socket_fd, const std::string &data,
                                       uint16_t stream_id, bool ordered) {
    (void)socket_fd;
    (void)data;
    (void)stream_id;
    (void)ordered;
    throw netd::shared::NotImplementedError(
        "SCTPTransport::sendDataOnStream not implemented");
  }

  std::string SCTPTransport::receiveDataFromStream(int socket_fd,
                                                   uint16_t &stream_id,
                                                   bool &ordered) {
    (void)socket_fd;
    (void)stream_id;
    (void)ordered;
    throw netd::shared::NotImplementedError(
        "SCTPTransport::receiveDataFromStream not implemented");
  }

} // namespace netd::shared
