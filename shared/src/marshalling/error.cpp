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

#include <libyang/libyang.h>
#include <libyang/tree_data.h>
#include <shared/include/exception.hpp>
#include <shared/include/marshalling/error.hpp>
#include <shared/include/yang.hpp>

namespace netd::shared::marshalling {

  Error::Error() {
    type = ErrorType::APPLICATION;
    tag = ErrorTag::OPERATION_FAILED;
    severity = ErrorSeverity::ERROR;
  }

  Error::Error(ErrorType type, ErrorTag tag, ErrorSeverity severity)
      : type(type), tag(tag), severity(severity) {}

  lyd_node *Error::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      return nullptr;
    }

    // Create rpc-error element
    lyd_node *errorNode = nullptr;
    if (lyd_new_opaq2(nullptr, ctx, "rpc-error", nullptr, nullptr,
                      "urn:ietf:params:xml:ns:netconf:base:1.0",
                      &errorNode) != LY_SUCCESS) {
      return nullptr;
    }

    // Add error-type
    lyd_node *typeNode = nullptr;
    if (lyd_new_inner(errorNode, nullptr, "error-type", 0, &typeNode) !=
        LY_SUCCESS) {
      lyd_free_tree(errorNode);
      return nullptr;
    }
    if (lyd_new_term(typeNode, nullptr, typeToString().c_str(), nullptr, 0,
                     nullptr) != LY_SUCCESS) {
      lyd_free_tree(errorNode);
      return nullptr;
    }

    // Add error-tag
    lyd_node *tagNode = nullptr;
    if (lyd_new_inner(errorNode, nullptr, "error-tag", 0, &tagNode) !=
        LY_SUCCESS) {
      lyd_free_tree(errorNode);
      return nullptr;
    }
    if (lyd_new_term(tagNode, nullptr, tagToString().c_str(), nullptr, 0,
                     nullptr) != LY_SUCCESS) {
      lyd_free_tree(errorNode);
      return nullptr;
    }

    // Add error-severity
    lyd_node *severityNode = nullptr;
    if (lyd_new_inner(errorNode, nullptr, "error-severity", 0, &severityNode) !=
        LY_SUCCESS) {
      lyd_free_tree(errorNode);
      return nullptr;
    }
    if (lyd_new_term(severityNode, nullptr, severityToString().c_str(), nullptr,
                     0, nullptr) != LY_SUCCESS) {
      lyd_free_tree(errorNode);
      return nullptr;
    }

    // Add error-message if present
    if (!message.empty()) {
      lyd_node *msgNode = nullptr;
      if (lyd_new_inner(errorNode, nullptr, "error-message", 0, &msgNode) !=
          LY_SUCCESS) {
        lyd_free_tree(errorNode);
        return nullptr;
      }
      if (lyd_new_term(msgNode, nullptr, message.c_str(), nullptr, 0,
                       nullptr) != LY_SUCCESS) {
        lyd_free_tree(errorNode);
        return nullptr;
      }
    }

    // Add error-path if present
    if (!path.empty()) {
      lyd_node *pathNode = nullptr;
      if (lyd_new_inner(errorNode, nullptr, "error-path", 0, &pathNode) !=
          LY_SUCCESS) {
        lyd_free_tree(errorNode);
        return nullptr;
      }
      if (lyd_new_term(pathNode, nullptr, path.c_str(), nullptr, 0, nullptr) !=
          LY_SUCCESS) {
        lyd_free_tree(errorNode);
        return nullptr;
      }
    }

    // Add error-info if present
    if (!info.empty()) {
      lyd_node *infoNode = nullptr;
      if (lyd_new_inner(errorNode, nullptr, "error-info", 0, &infoNode) !=
          LY_SUCCESS) {
        lyd_free_tree(errorNode);
        return nullptr;
      }
      if (lyd_new_term(infoNode, nullptr, info.c_str(), nullptr, 0, nullptr) !=
          LY_SUCCESS) {
        lyd_free_tree(errorNode);
        return nullptr;
      }
    }

    return errorNode;
  }

  std::unique_ptr<Error> Error::fromYang([[maybe_unused]] const ly_ctx *ctx,
                                         const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to Error::fromYang");
    }

    auto error = std::make_unique<Error>();
    // TODO: Parse error data from node
    return error;
  }

  std::string Error::typeToString() const {
    switch (type) {
    case ErrorType::TRANSPORT:
      return "transport";
    case ErrorType::RPC:
      return "rpc";
    case ErrorType::PROTOCOL:
      return "protocol";
    case ErrorType::APPLICATION:
      return "application";
    default:
      return "application";
    }
  }

  std::string Error::tagToString() const {
    switch (tag) {
    case ErrorTag::IN_USE:
      return "in-use";
    case ErrorTag::INVALID_VALUE:
      return "invalid-value";
    case ErrorTag::TOO_BIG:
      return "too-big";
    case ErrorTag::MISSING_ATTRIBUTE:
      return "missing-attribute";
    case ErrorTag::BAD_ATTRIBUTE:
      return "bad-attribute";
    case ErrorTag::UNKNOWN_ATTRIBUTE:
      return "unknown-attribute";
    case ErrorTag::MISSING_ELEMENT:
      return "missing-element";
    case ErrorTag::BAD_ELEMENT:
      return "bad-element";
    case ErrorTag::UNKNOWN_ELEMENT:
      return "unknown-element";
    case ErrorTag::UNKNOWN_NAMESPACE:
      return "unknown-namespace";
    case ErrorTag::ACCESS_DENIED:
      return "access-denied";
    case ErrorTag::LOCK_DENIED:
      return "lock-denied";
    case ErrorTag::RESOURCE_DENIED:
      return "resource-denied";
    case ErrorTag::ROLLBACK_FAILED:
      return "rollback-failed";
    case ErrorTag::DATA_EXISTS:
      return "data-exists";
    case ErrorTag::DATA_MISSING:
      return "data-missing";
    case ErrorTag::OPERATION_NOT_SUPPORTED:
      return "operation-not-supported";
    case ErrorTag::OPERATION_FAILED:
      return "operation-failed";
    case ErrorTag::PARTIAL_OPERATION:
      return "partial-operation";
    case ErrorTag::MALFORMED_MESSAGE:
      return "malformed-message";
    default:
      return "operation-failed";
    }
  }

  std::string Error::severityToString() const {
    switch (severity) {
    case ErrorSeverity::ERROR:
      return "error";
    case ErrorSeverity::WARNING:
      return "warning";
    default:
      return "error";
    }
  }

} // namespace netd::shared::marshalling
