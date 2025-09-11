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
#include <shared/include/logger.hpp>

namespace netd::shared {

  DTLSSecurityWrapper::DTLSSecurityWrapper(std::unique_ptr<BaseTransport> transport,
                                          const std::string& cert_file,
                                          const std::string& key_file,
                                          const std::string& ca_file,
                                          bool verify_peer,
                                          uint32_t mtu_size)
    : TLSSecurityWrapper(std::move(transport), cert_file, key_file, ca_file, verify_peer)
    , mtu_size_(mtu_size)
    , cookie_exchange_enabled_(false)
    , retransmission_timeout_(1000) // Default 1 second
  {
  }

  bool DTLSSecurityWrapper::initializeDTLS() {
    // Use the parent TLS initialization
    if (!initializeTLS()) {
      return false;
    }

    auto& logger = Logger::getInstance();
    logger.info("DTLS security wrapper initialized (MTU: " + std::to_string(mtu_size_) + ")");
    return true;
  }

  void DTLSSecurityWrapper::cleanupDTLS() {
    // Use the parent TLS cleanup
    cleanupTLS();
    
    auto& logger = Logger::getInstance();
    logger.info("DTLS security wrapper cleaned up");
  }

  bool DTLSSecurityWrapper::isDTLSInitialized() const {
    return isTLSInitialized();
  }

  bool DTLSSecurityWrapper::performHandshake(int socket_fd, bool is_server) {
    if (!isDTLSInitialized()) {
      auto& logger = Logger::getInstance();
      logger.error("DTLS not initialized, cannot perform handshake");
      return false;
    }

    auto& logger = Logger::getInstance();
    logger.info("Performing DTLS handshake on socket " + std::to_string(socket_fd) + 
                " (server: " + (is_server ? "yes" : "no") + ")");
    
    // DTLS-specific handshake logic (connectionless)
    // This would implement the DTLS handshake protocol
    // For now, delegate to parent TLS handshake
    return TLSSecurityWrapper::performHandshake(socket_fd, is_server);
  }

  bool DTLSSecurityWrapper::sendEncryptedData(int socket_fd, const std::string& data) {
    if (!isDTLSInitialized()) {
      auto& logger = Logger::getInstance();
      logger.error("DTLS not initialized, cannot send encrypted data");
      return false;
    }

    // Check MTU size (DTLS-specific)
    if (data.length() > mtu_size_) {
      auto& logger = Logger::getInstance();
      logger.warning("Data size (" + std::to_string(data.length()) + 
                    ") exceeds MTU (" + std::to_string(mtu_size_) + ")");
    }

    auto& logger = Logger::getInstance();
    logger.debug("Sending encrypted DTLS data on socket " + std::to_string(socket_fd) + 
                 " (" + std::to_string(data.length()) + " bytes)");
    
    // Use parent TLS send with DTLS-specific handling
    return TLSSecurityWrapper::sendEncryptedData(socket_fd, data);
  }

  std::string DTLSSecurityWrapper::receiveEncryptedData(int socket_fd) {
    if (!isDTLSInitialized()) {
      auto& logger = Logger::getInstance();
      logger.error("DTLS not initialized, cannot receive encrypted data");
      return "";
    }

    auto& logger = Logger::getInstance();
    logger.debug("Receiving encrypted DTLS data from socket " + std::to_string(socket_fd));
    
    // Use parent TLS receive with DTLS-specific handling
    return TLSSecurityWrapper::receiveEncryptedData(socket_fd);
  }

  void DTLSSecurityWrapper::setMTUSize(uint32_t mtu) {
    mtu_size_ = mtu;
  }

  uint32_t DTLSSecurityWrapper::getMTUSize() const {
    return mtu_size_;
  }

  bool DTLSSecurityWrapper::enableCookieExchange() {
    cookie_exchange_enabled_ = true;
    auto& logger = Logger::getInstance();
    logger.info("DTLS cookie exchange enabled");
    return true;
  }

  bool DTLSSecurityWrapper::setCookieSecret(const std::string& secret) {
    cookie_secret_ = secret;
    auto& logger = Logger::getInstance();
    logger.info("DTLS cookie secret set (" + std::to_string(secret.length()) + " bytes)");
    return true;
  }

  bool DTLSSecurityWrapper::setRetransmissionTimeout(uint32_t timeout_ms) {
    retransmission_timeout_ = timeout_ms;
    auto& logger = Logger::getInstance();
    logger.info("DTLS retransmission timeout set to " + std::to_string(timeout_ms) + "ms");
    return true;
  }

  bool DTLSSecurityWrapper::isCookieExchangeEnabled() const {
    return cookie_exchange_enabled_;
  }

  uint32_t DTLSSecurityWrapper::getRetransmissionTimeout() const {
    return retransmission_timeout_;
  }

} // namespace netd::shared
