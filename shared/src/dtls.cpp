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

#include <shared/include/dtls.hpp>
#include <shared/include/exception.hpp>

namespace netd::shared {

  DTLSSecurity::DTLSSecurity(const std::string& cert_file,
                            const std::string& key_file,
                            const std::string& ca_file,
                            bool verify_peer,
                            uint32_t mtu_size)
    : TLSSecurity(cert_file, key_file, ca_file, verify_peer)
    , mtu_size_(mtu_size)
    , cookie_exchange_enabled_(false)
    , cookie_secret_("")
    , retransmission_timeout_(1000)
  {
  }

  DTLSSecurity::~DTLSSecurity() {
    cleanupDTLS();
  }

  // DTLS-specific initialization (overrides TLS)
  bool DTLSSecurity::initializeDTLS() {
    throw netd::shared::NotImplementedError("DTLSSecurity::initializeDTLS not implemented");
  }

  void DTLSSecurity::cleanupDTLS() {
    throw netd::shared::NotImplementedError("DTLSSecurity::cleanupDTLS not implemented");
  }

  bool DTLSSecurity::isDTLSInitialized() const {
    throw netd::shared::NotImplementedError("DTLSSecurity::isDTLSInitialized not implemented");
  }

  // DTLS handshake (connectionless - overrides TLS)
  bool DTLSSecurity::performHandshake(int socket_fd, bool is_server) {
    (void)socket_fd; (void)is_server;
    throw netd::shared::NotImplementedError("DTLSSecurity::performHandshake not implemented");
  }

  // Encrypted data operations (overrides TLS with MTU awareness)
  bool DTLSSecurity::sendEncryptedData(int socket_fd, const std::string& data) {
    (void)socket_fd; (void)data;
    throw netd::shared::NotImplementedError("DTLSSecurity::sendEncryptedData not implemented");
  }

  std::string DTLSSecurity::receiveEncryptedData(int socket_fd) {
    (void)socket_fd;
    throw netd::shared::NotImplementedError("DTLSSecurity::receiveEncryptedData not implemented");
  }

  // DTLS-specific configuration
  void DTLSSecurity::setMTUSize(uint32_t mtu) {
    (void)mtu;
    throw netd::shared::NotImplementedError("DTLSSecurity::setMTUSize not implemented");
  }

  uint32_t DTLSSecurity::getMTUSize() const {
    throw netd::shared::NotImplementedError("DTLSSecurity::getMTUSize not implemented");
  }

  // DTLS-specific features
  bool DTLSSecurity::enableCookieExchange() {
    throw netd::shared::NotImplementedError("DTLSSecurity::enableCookieExchange not implemented");
  }

  bool DTLSSecurity::setCookieSecret(const std::string& secret) {
    (void)secret;
    throw netd::shared::NotImplementedError("DTLSSecurity::setCookieSecret not implemented");
  }

  bool DTLSSecurity::setRetransmissionTimeout(uint32_t timeout_ms) {
    (void)timeout_ms;
    throw netd::shared::NotImplementedError("DTLSSecurity::setRetransmissionTimeout not implemented");
  }

  // Access to DTLS-specific state
  bool DTLSSecurity::isCookieExchangeEnabled() const {
    throw netd::shared::NotImplementedError("DTLSSecurity::isCookieExchangeEnabled not implemented");
  }

  uint32_t DTLSSecurity::getRetransmissionTimeout() const {
    throw netd::shared::NotImplementedError("DTLSSecurity::getRetransmissionTimeout not implemented");
  }

} // namespace netd::shared
