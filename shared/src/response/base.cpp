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
#include <shared/include/marshalling/error.hpp>
#include <shared/include/response/base.hpp>
#include <shared/include/xml/base.hpp>
#include <shared/include/xml/envelope.hpp>
#include <sstream>

namespace netd::shared::response {

  // NOTE: This base class contains stubbed implementations for interface
  // compliance only. The actual functionality should be implemented in derived
  // classes, not here.

  Response::Response() {
    error = nullptr;
    data = nullptr;
  }

  Response::Response(Response &&other) noexcept
      : error(std::move(other.error)), data(std::move(other.data)) {
    other.error = nullptr;
    other.data = nullptr;
  }

  Response &Response::operator=(Response &&other) noexcept {
    if (this != &other) {
      error = std::move(other.error);
      data = std::move(other.data);
      other.error = nullptr;
      other.data = nullptr;
    }
    return *this;
  }

  // Convenience methods for common error types
  // NOTE: These methods should NOT be implemented in base class - they are
  // stubbed for interface compliance only
  void Response::setProtocolError(
      [[maybe_unused]] netd::shared::marshalling::ErrorTag tag,
      [[maybe_unused]] const std::string &message) {
    throw netd::shared::NotImplementedError(
        "Response::setProtocolError not implemented");
  }

  void Response::setApplicationError(
      [[maybe_unused]] netd::shared::marshalling::ErrorTag tag,
      [[maybe_unused]] const std::string &message) {
    throw netd::shared::NotImplementedError(
        "Response::setApplicationError not implemented");
  }

  void Response::setRpcError(
      [[maybe_unused]] netd::shared::marshalling::ErrorTag tag,
      [[maybe_unused]] const std::string &message) {
    throw netd::shared::NotImplementedError(
        "Response::setRpcError not implemented");
  }

  void Response::setTransportError(
      [[maybe_unused]] netd::shared::marshalling::ErrorTag tag,
      [[maybe_unused]] const std::string &message) {
    throw netd::shared::NotImplementedError(
        "Response::setTransportError not implemented");
  }

  std::unique_ptr<netd::shared::xml::RpcEnvelope> Response::toRpcEnvelope(
      std::shared_ptr<netd::shared::xml::RpcEnvelope> request_envelope,
      ly_ctx *ctx) const {
    if (!request_envelope) {
      throw netd::shared::ArgumentError(
          "Invalid request envelope provided to Response::toRpcEnvelope");
    }

    // Get the response data
    lyd_node *response_data = toYang(ctx);

    // Determine if this is an error response
    netd::shared::xml::RpcType reply_type =
        isError() ? netd::shared::xml::RpcType::RPC_ERROR
                  : netd::shared::xml::RpcType::RPC_REPLY;

    // Create the reply envelope with the same message ID and operation as the
    // request
    return netd::shared::xml::RpcEnvelope::toXml(
        reply_type, request_envelope->getMessageId(),
        request_envelope->getOperation(),
        nullptr, // No additional filter needed for responses
        response_data, ctx);
  }

} // namespace netd::shared::response
