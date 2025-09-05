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

#include <shared/include/marshalling/error.hpp>
#include <shared/include/response/base.hpp>

namespace netd::shared::response {

  Response::Response() {
    error = nullptr;
    data = nullptr;
  }

  // Convenience methods for common error types
  void Response::setProtocolError(netd::shared::marshalling::ErrorTag tag,
                                  const std::string &message) {
    auto err = std::make_unique<netd::shared::marshalling::Error>(
        netd::shared::marshalling::ErrorType::PROTOCOL, tag,
        netd::shared::marshalling::ErrorSeverity::ERROR);
    if (!message.empty()) {
      err->setMessage(message);
    }
    error = std::move(err);
  }

  void Response::setApplicationError(netd::shared::marshalling::ErrorTag tag,
                                     const std::string &message) {
    auto err = std::make_unique<netd::shared::marshalling::Error>(
        netd::shared::marshalling::ErrorType::APPLICATION, tag,
        netd::shared::marshalling::ErrorSeverity::ERROR);
    if (!message.empty()) {
      err->setMessage(message);
    }
    error = std::move(err);
  }

  void Response::setRpcError(netd::shared::marshalling::ErrorTag tag,
                             const std::string &message) {
    auto err = std::make_unique<netd::shared::marshalling::Error>(
        netd::shared::marshalling::ErrorType::RPC, tag,
        netd::shared::marshalling::ErrorSeverity::ERROR);
    if (!message.empty()) {
      err->setMessage(message);
    }
    error = std::move(err);
  }

  void Response::setTransportError(netd::shared::marshalling::ErrorTag tag,
                                   const std::string &message) {
    auto err = std::make_unique<netd::shared::marshalling::Error>(
        netd::shared::marshalling::ErrorType::TRANSPORT, tag,
        netd::shared::marshalling::ErrorSeverity::ERROR);
    if (!message.empty()) {
      err->setMessage(message);
    }
    error = std::move(err);
  }

  // Convenience methods for common data types
  void Response::setNetworkInstance(
      std::unique_ptr<netd::shared::marshalling::NetworkInstance> instance) {
    data = std::move(instance);
  }

  void
  Response::setRoute(std::unique_ptr<netd::shared::marshalling::Route> route) {
    data = std::move(route);
  }

  void Response::setInterface(
      std::unique_ptr<netd::shared::marshalling::Interface> interface) {
    data = std::move(interface);
  }

} // namespace netd::shared::response
