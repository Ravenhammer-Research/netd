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

#include <shared/include/tls.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>

namespace netd::shared {

  TLSSecurityWrapper::TLSSecurityWrapper(std::unique_ptr<BaseTransport> transport,
                                        const std::string& cert_file,
                                        const std::string& key_file,
                                        const std::string& ca_file,
                                        bool verify_peer)
    : transport_(std::move(transport))
    , cert_file_(cert_file)
    , key_file_(key_file)
    , ca_file_(ca_file)
    , verify_peer_(verify_peer)
    , initialized_(false)
  {
    if (!transport_) {
      throw std::invalid_argument("Transport cannot be null");
    }
  }

  TLSSecurityWrapper::~TLSSecurityWrapper() {
    cleanupTLS();
  }

  bool TLSSecurityWrapper::initializeTLS() {
    if (initialized_) {
      return true;
    }

    auto& logger = Logger::getInstance();
    
    // Load certificates
    if (!loadCertificate(cert_file_)) {
      logger.error("Failed to load TLS certificate: " + cert_file_);
      return false;
    }

    if (!loadPrivateKey(key_file_)) {
      logger.error("Failed to load TLS private key: " + key_file_);
      return false;
    }

    if (!ca_file_.empty() && !loadCAFile(ca_file_)) {
      logger.error("Failed to load CA file: " + ca_file_);
      return false;
    }

    initialized_ = true;
    logger.info("TLS security wrapper initialized");
    return true;
  }

  void TLSSecurityWrapper::cleanupTLS() {
    if (initialized_) {
      auto& logger = Logger::getInstance();
      logger.info("TLS security wrapper cleaned up");
      initialized_ = false;
    }
  }

  bool TLSSecurityWrapper::isTLSInitialized() const {
    return initialized_;
  }

  bool TLSSecurityWrapper::loadCertificate(const std::string& cert_file) {
    // Placeholder for certificate loading
    auto& logger = Logger::getInstance();
    logger.info("Loading TLS certificate: " + cert_file);
    return true;
  }

  bool TLSSecurityWrapper::loadPrivateKey(const std::string& key_file) {
    // Placeholder for private key loading
    auto& logger = Logger::getInstance();
    logger.info("Loading TLS private key: " + key_file);
    return true;
  }

  bool TLSSecurityWrapper::loadCAFile(const std::string& ca_file) {
    // Placeholder for CA file loading
    auto& logger = Logger::getInstance();
    logger.info("Loading TLS CA file: " + ca_file);
    return true;
  }

  bool TLSSecurityWrapper::performHandshake(int socket_fd, bool is_server) {
    if (!initialized_) {
      auto& logger = Logger::getInstance();
      logger.error("TLS not initialized, cannot perform handshake");
      return false;
    }

    auto& logger = Logger::getInstance();
    logger.info("Performing TLS handshake on socket " + std::to_string(socket_fd) + 
                " (server: " + (is_server ? "yes" : "no") + ")");
    
    // Placeholder for TLS handshake
    return true;
  }

  bool TLSSecurityWrapper::sendEncryptedData(int socket_fd, const std::string& data) {
    if (!initialized_) {
      auto& logger = Logger::getInstance();
      logger.error("TLS not initialized, cannot send encrypted data");
      return false;
    }

    // Placeholder for encrypted data sending
    auto& logger = Logger::getInstance();
    logger.debug("Sending encrypted data on socket " + std::to_string(socket_fd) + 
                 " (" + std::to_string(data.length()) + " bytes)");
    
    // For now, just pass through to underlying transport
    return transport_->sendData(socket_fd, data);
  }

  std::string TLSSecurityWrapper::receiveEncryptedData(int socket_fd) {
    if (!initialized_) {
      auto& logger = Logger::getInstance();
      logger.error("TLS not initialized, cannot receive encrypted data");
      return "";
    }

    // Placeholder for encrypted data receiving
    auto& logger = Logger::getInstance();
    logger.debug("Receiving encrypted data from socket " + std::to_string(socket_fd));
    
    // For now, just pass through to underlying transport
    return transport_->receiveData(socket_fd);
  }

  BaseTransport* TLSSecurityWrapper::getTransport() const {
    return transport_.get();
  }

  void TLSSecurityWrapper::setVerifyPeer(bool verify) {
    verify_peer_ = verify;
  }

  bool TLSSecurityWrapper::getVerifyPeer() const {
    return verify_peer_;
  }

} // namespace netd::shared
