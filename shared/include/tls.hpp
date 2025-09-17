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

#ifndef NETD_SERVER_NETCONF_TLS_HPP
#define NETD_SERVER_NETCONF_TLS_HPP

#include <memory>
#include <string>

namespace netd::shared {

  /**
   * TLS Security Component
   *
   * This class provides TLS encryption/decryption as a security component
   * that can be consumed by transports. It provides TLS functionality
   * without wrapping transports.
   */
  class TLSSecurity {
  private:
    std::string cert_file_;
    std::string key_file_;
    std::string ca_file_;
    bool verify_peer_;
    bool initialized_;

  public:
    /**
     * Constructor
     * @param cert_file Path to certificate file
     * @param key_file Path to private key file
     * @param ca_file Path to CA certificate file (optional)
     * @param verify_peer Whether to verify peer certificates
     */
    TLSSecurity(const std::string &cert_file, const std::string &key_file,
                const std::string &ca_file = "", bool verify_peer = true);

    ~TLSSecurity();

    // TLS-specific methods
    bool initializeTLS();
    void cleanupTLS();
    bool isTLSInitialized() const;

    // Certificate management
    bool loadCertificate(const std::string &cert_file);
    bool loadPrivateKey(const std::string &key_file);
    bool loadCAFile(const std::string &ca_file);

    // TLS handshake
    virtual bool performHandshake(int socket_fd, bool is_server = true);

    // Encrypted data operations
    virtual bool sendEncryptedData(int socket_fd, const std::string &data);
    virtual std::string receiveEncryptedData(int socket_fd);

    // Configuration
    void setVerifyPeer(bool verify);
    bool getVerifyPeer() const;
  };

} // namespace netd::shared

#endif // NETD_SERVER_NETCONF_TLS_HPP
