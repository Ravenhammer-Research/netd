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

#ifndef NETD_SERVER_NETCONF_DTLS_HPP
#define NETD_SERVER_NETCONF_DTLS_HPP

#include <shared/include/tls.hpp>
#include <string>

namespace netd::shared {

  /**
   * DTLS Security Component
   *
   * This class extends TLSSecurity to provide DTLS encryption/decryption
   * for datagram-based transports. DTLS is particularly useful for UDP-based
   * transports and SCTP (RFC 6083).
   */
  class DTLSSecurity : public TLSSecurity {
  private:
    uint32_t mtu_size_;
    bool cookie_exchange_enabled_;
    std::string cookie_secret_;
    uint32_t retransmission_timeout_;

  public:
    /**
     * Constructor
     * @param cert_file Path to certificate file
     * @param key_file Path to private key file
     * @param ca_file Path to CA certificate file (optional)
     * @param verify_peer Whether to verify peer certificates
     * @param mtu_size Maximum transmission unit size (default 1500)
     */
    DTLSSecurity(const std::string &cert_file, const std::string &key_file,
                 const std::string &ca_file = "", bool verify_peer = true,
                 uint32_t mtu_size = 1500);

    ~DTLSSecurity();

    // DTLS-specific initialization (overrides TLS)
    bool initializeDTLS();
    void cleanupDTLS();
    bool isDTLSInitialized() const;

    // DTLS handshake (connectionless - overrides TLS)
    bool performHandshake(int socket_fd, bool is_server = true) override;

    // Encrypted data operations (overrides TLS with MTU awareness)
    bool sendEncryptedData(int socket_fd, const std::string &data) override;
    std::string receiveEncryptedData(int socket_fd) override;

    // DTLS-specific configuration
    void setMTUSize(uint32_t mtu);
    uint32_t getMTUSize() const;

    // DTLS-specific features
    bool enableCookieExchange();
    bool setCookieSecret(const std::string &secret);
    bool setRetransmissionTimeout(uint32_t timeout_ms);

    // Access to DTLS-specific state
    bool isCookieExchangeEnabled() const;
    uint32_t getRetransmissionTimeout() const;
  };

} // namespace netd::shared

#endif // NETD_SERVER_NETCONF_DTLS_HPP
