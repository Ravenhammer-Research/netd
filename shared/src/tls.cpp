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
#include <shared/include/tls.hpp>

namespace netd::shared {

  TLSSecurity::TLSSecurity(const std::string &cert_file,
                           const std::string &key_file,
                           const std::string &ca_file, bool verify_peer)
      : cert_file_(cert_file), key_file_(key_file), ca_file_(ca_file),
        verify_peer_(verify_peer), initialized_(false) {
    (void)verify_peer_;
    (void)initialized_;
  }

  TLSSecurity::~TLSSecurity() { cleanupTLS(); }

  // TLS-specific methods
  bool TLSSecurity::initializeTLS() {
    throw netd::shared::NotImplementedError(
        "TLSSecurity::initializeTLS not implemented");
  }

  void TLSSecurity::cleanupTLS() {
    throw netd::shared::NotImplementedError(
        "TLSSecurity::cleanupTLS not implemented");
  }

  bool TLSSecurity::isTLSInitialized() const {
    throw netd::shared::NotImplementedError(
        "TLSSecurity::isTLSInitialized not implemented");
  }

  // Certificate management
  bool TLSSecurity::loadCertificate(const std::string &cert_file) {
    (void)cert_file;
    throw netd::shared::NotImplementedError(
        "TLSSecurity::loadCertificate not implemented");
  }

  bool TLSSecurity::loadPrivateKey(const std::string &key_file) {
    (void)key_file;
    throw netd::shared::NotImplementedError(
        "TLSSecurity::loadPrivateKey not implemented");
  }

  bool TLSSecurity::loadCAFile(const std::string &ca_file) {
    (void)ca_file;
    throw netd::shared::NotImplementedError(
        "TLSSecurity::loadCAFile not implemented");
  }

  // TLS handshake
  bool TLSSecurity::performHandshake(int socket_fd, bool is_server) {
    (void)socket_fd;
    (void)is_server;
    throw netd::shared::NotImplementedError(
        "TLSSecurity::performHandshake not implemented");
  }

  // Encrypted data operations
  bool TLSSecurity::sendEncryptedData(int socket_fd, const std::string &data) {
    (void)socket_fd;
    (void)data;
    throw netd::shared::NotImplementedError(
        "TLSSecurity::sendEncryptedData not implemented");
  }

  std::string TLSSecurity::receiveEncryptedData(int socket_fd) {
    (void)socket_fd;
    throw netd::shared::NotImplementedError(
        "TLSSecurity::receiveEncryptedData not implemented");
  }

  // Configuration
  void TLSSecurity::setVerifyPeer(bool verify) {
    (void)verify;
    throw netd::shared::NotImplementedError(
        "TLSSecurity::setVerifyPeer not implemented");
  }

  bool TLSSecurity::getVerifyPeer() const {
    throw netd::shared::NotImplementedError(
        "TLSSecurity::getVerifyPeer not implemented");
  }

} // namespace netd::shared
