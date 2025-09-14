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

#ifndef NETD_EXCEPTION_HPP
#define NETD_EXCEPTION_HPP

#include <stdexcept>
#include <string>
#include <vector>

namespace netd::shared {

  class NetdError : public std::runtime_error {
  private:
    std::vector<void*> stackTrace_;

  public:
    explicit NetdError(const std::string &message);
    const std::vector<void*>& getStackTrace() const { return stackTrace_; }
    static std::string getStackTraceString(const std::vector<void*> &stackTrace);
  };

  class NotImplementedError : public NetdError {
  public:
    explicit NotImplementedError(const std::string &message)
        : NetdError(message) {}
  };

  class ArgumentError : public NetdError {
  public:
    explicit ArgumentError(const std::string &message)
        : NetdError(message) {}
  };

  class ConnectionError : public NetdError {
  public:
    explicit ConnectionError(const std::string &message)
        : NetdError(message) {}
  };

  class ConfigurationError : public NetdError {
  public:
    explicit ConfigurationError(const std::string &message)
        : NetdError(message) {}
  };

  class NetworkError : public NetdError {
  public:
    explicit NetworkError(const std::string &message)
        : NetdError(message) {}
  };

  class RpcError : public NetdError {
  public:
    explicit RpcError(const std::string &message)
        : NetdError(message) {}
  };

  // YANG-related exceptions
  class YangError : public NetdError {
  private:
    void* yang_ctx_;  // ly_ctx* but using void* to avoid libyang dependency in header

  public:
    explicit YangError(void* yang_ctx)
        : NetdError("YANG error"), yang_ctx_(yang_ctx) {}
    
    void* getYangContext() const { return yang_ctx_; }
  };

  class YangParseError : public YangError {
  public:
    explicit YangParseError(void* yang_ctx)
        : YangError(yang_ctx) {}
  };

  class YangValidationError : public YangError {
  public:
    explicit YangValidationError(void* yang_ctx)
        : YangError(yang_ctx) {}
  };

  class YangContextError : public YangError {
  public:
    explicit YangContextError(void* yang_ctx)
        : YangError(yang_ctx) {}
  };

  class YangDataError : public YangError {
  public:
    explicit YangDataError(void* yang_ctx)
        : YangError(yang_ctx) {}
  };

  class YangSchemaError : public YangError {
  public:
    explicit YangSchemaError(void* yang_ctx)
        : YangError(yang_ctx) {}
  };

  // Transport-specific exceptions
  class TransportError : public NetdError {
  public:
    explicit TransportError(const std::string &message)
        : NetdError(message) {}
  };

  class SocketError : public TransportError {
  public:
    explicit SocketError(const std::string &message)
        : TransportError(message) {}
  };

  class BindError : public TransportError {
  public:
    explicit BindError(const std::string &message)
        : TransportError(message) {}
  };

  class ListenError : public TransportError {
  public:
    explicit ListenError(const std::string &message)
        : TransportError(message) {}
  };

  class AcceptError : public TransportError {
  public:
    explicit AcceptError(const std::string &message)
        : TransportError(message) {}
  };

  class SendError : public TransportError {
  public:
    explicit SendError(const std::string &message)
        : TransportError(message) {}
  };

  class ReceiveError : public TransportError {
  public:
    explicit ReceiveError(const std::string &message)
        : TransportError(message) {}
  };

  // Protocol-specific exceptions
  class ProtocolError : public NetdError {
  public:
    explicit ProtocolError(const std::string &message)
        : NetdError(message) {}
  };

  class HttpError : public ProtocolError {
  public:
    explicit HttpError(const std::string &message)
        : ProtocolError(message) {}
  };

  class HttpParseError : public HttpError {
  public:
    explicit HttpParseError(const std::string &message)
        : HttpError(message) {}
  };

  class HttpVersionError : public HttpError {
  public:
    explicit HttpVersionError(const std::string &message)
        : HttpError(message) {}
  };

  class QuicError : public ProtocolError {
  public:
    explicit QuicError(const std::string &message)
        : ProtocolError(message) {}
  };

  class QuicConnectionError : public QuicError {
  public:
    explicit QuicConnectionError(const std::string &message)
        : QuicError(message) {}
  };

  class QuicStreamError : public QuicError {
  public:
    explicit QuicStreamError(const std::string &message)
        : QuicError(message) {}
  };

  class QuicPacketError : public QuicError {
  public:
    explicit QuicPacketError(const std::string &message)
        : QuicError(message) {}
  };

  class SctpError : public ProtocolError {
  public:
    explicit SctpError(const std::string &message)
        : ProtocolError(message) {}
  };

  class SctpAssociationError : public SctpError {
  public:
    explicit SctpAssociationError(const std::string &message)
        : SctpError(message) {}
  };

  class SctpStreamError : public SctpError {
  public:
    explicit SctpStreamError(const std::string &message)
        : SctpError(message) {}
  };

  class SctpMultihomingError : public SctpError {
  public:
    explicit SctpMultihomingError(const std::string &message)
        : SctpError(message) {}
  };

  // Security-related exceptions
  class SecurityError : public NetdError {
  public:
    explicit SecurityError(const std::string &message)
        : NetdError(message) {}
  };

  class TLSError : public SecurityError {
  public:
    explicit TLSError(const std::string &message)
        : SecurityError(message) {}
  };

  class TLSCertificateError : public TLSError {
  public:
    explicit TLSCertificateError(const std::string &message)
        : TLSError(message) {}
  };

  class TLSHandshakeError : public TLSError {
  public:
    explicit TLSHandshakeError(const std::string &message)
        : TLSError(message) {}
  };

  class TLSKeyError : public TLSError {
  public:
    explicit TLSKeyError(const std::string &message)
        : TLSError(message) {}
  };

  class DTLSError : public TLSError {
  public:
    explicit DTLSError(const std::string &message)
        : TLSError(message) {}
  };

  class DTLSMTUError : public DTLSError {
  public:
    explicit DTLSMTUError(const std::string &message)
        : DTLSError(message) {}
  };

  class DTLSRetransmissionError : public DTLSError {
  public:
    explicit DTLSRetransmissionError(const std::string &message)
        : DTLSError(message) {}
  };

  // Service discovery exceptions
  class ServiceDiscoveryError : public NetdError {
  public:
    explicit ServiceDiscoveryError(const std::string &message)
        : NetdError(message) {}
  };

  class LLDPError : public ServiceDiscoveryError {
  public:
    explicit LLDPError(const std::string &message)
        : ServiceDiscoveryError(message) {}
  };

  class LLDPRegistrationError : public LLDPError {
  public:
    explicit LLDPRegistrationError(const std::string &message)
        : LLDPError(message) {}
  };

  class LLDPDiscoveryError : public LLDPError {
  public:
    explicit LLDPDiscoveryError(const std::string &message)
        : LLDPError(message) {}
  };

  // XML-related exceptions
  class XmlError : public NetdError {
  public:
    explicit XmlError(const std::string &message)
        : NetdError(message) {}
  };

  class XmlParseError : public XmlError {
  public:
    explicit XmlParseError(const std::string &message)
        : XmlError(message) {}
  };

  class XmlValidationError : public XmlError {
  public:
    explicit XmlValidationError(const std::string &message)
        : XmlError(message) {}
  };

  class XmlSerializationError : public XmlError {
  public:
    explicit XmlSerializationError(const std::string &message)
        : XmlError(message) {}
  };

  class XmlNamespaceError : public XmlError {
  public:
    explicit XmlNamespaceError(const std::string &message)
        : XmlError(message) {}
  };

  class XmlSchemaError : public XmlError {
  public:
    explicit XmlSchemaError(const std::string &message)
        : XmlError(message) {}
  };

  class XmlEncodingError : public XmlError {
  public:
    explicit XmlEncodingError(const std::string &message)
        : XmlError(message) {}
  };

  class XmlMalformedError : public XmlError {
  public:
    explicit XmlMalformedError(const std::string &message)
        : XmlError(message) {}
  };

} // namespace netd::shared

#endif // NETD_EXCEPTION_HPP
