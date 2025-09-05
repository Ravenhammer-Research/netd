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

#ifndef NETD_MARSHALLING_ERROR_HPP
#define NETD_MARSHALLING_ERROR_HPP

#include <memory>
#include <shared/include/base/serialization.hpp>
#include <string>

namespace netd::shared::marshalling {

  // Error type enumeration
  enum class ErrorType { TRANSPORT, RPC, PROTOCOL, APPLICATION };

  // Error tag enumeration (NETCONF standard tags)
  enum class ErrorTag {
    IN_USE,
    INVALID_VALUE,
    TOO_BIG,
    MISSING_ATTRIBUTE,
    BAD_ATTRIBUTE,
    UNKNOWN_ATTRIBUTE,
    MISSING_ELEMENT,
    BAD_ELEMENT,
    UNKNOWN_ELEMENT,
    UNKNOWN_NAMESPACE,
    ACCESS_DENIED,
    LOCK_DENIED,
    RESOURCE_DENIED,
    ROLLBACK_FAILED,
    DATA_EXISTS,
    DATA_MISSING,
    OPERATION_NOT_SUPPORTED,
    OPERATION_FAILED,
    PARTIAL_OPERATION,
    MALFORMED_MESSAGE
  };

  // Error severity enumeration
  enum class ErrorSeverity { ERROR, WARNING };

  class Error {
  public:
    Error();
    Error(ErrorType type, ErrorTag tag, ErrorSeverity severity);
    virtual ~Error() = default;

    // Getters and setters
    ErrorType getType() const { return type; }
    void setType(ErrorType t) { type = t; }

    ErrorTag getTag() const { return tag; }
    void setTag(ErrorTag t) { tag = t; }

    ErrorSeverity getSeverity() const { return severity; }
    void setSeverity(ErrorSeverity s) { severity = s; }

    const std::string &getMessage() const { return message; }
    void setMessage(const std::string &msg) { message = msg; }

    const std::string &getPath() const { return path; }
    void setPath(const std::string &p) { path = p; }

    const std::string &getInfo() const { return info; }
    void setInfo(const std::string &i) { info = i; }

    // Serialization methods
    lyd_node *toYang(ly_ctx *ctx) const;
    std::unique_ptr<Error> fromYang(const ly_ctx *ctx, const lyd_node *node);

    // Helper methods to convert enums to strings
    std::string typeToString() const;
    std::string tagToString() const;
    std::string severityToString() const;

  private:
    ErrorType type = ErrorType::APPLICATION;
    ErrorTag tag = ErrorTag::OPERATION_FAILED;
    ErrorSeverity severity = ErrorSeverity::ERROR;
    std::string message;
    std::string path;
    std::string info;
  };

} // namespace netd::shared::marshalling

#endif // NETD_MARSHALLING_ERROR_HPP
