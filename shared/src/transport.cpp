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

#include <shared/include/transport.hpp>
#include <shared/include/unix.hpp>
#include <shared/include/sctp.hpp>
#include <shared/include/http.hpp>
#include <shared/include/quic.hpp>
#include <shared/include/exception.hpp>

namespace netd::shared {

  std::unique_ptr<BaseTransport> BaseTransport::create(TransportType type) {
    switch (type) {
      case TransportType::UNIX:
        return std::make_unique<UnixTransport>();
        
      case TransportType::SCTP:
        throw NotImplementedError("SCTP transport not yet implemented");
        
      case TransportType::HTTP:
        throw NotImplementedError("HTTP transport not yet implemented");
        
      case TransportType::SCTPS:
        throw NotImplementedError("SCTP over TLS transport not yet implemented");
        
      case TransportType::HTTPS:
        throw NotImplementedError("HTTPS transport not yet implemented");
        
      default:
        throw TransportError("Unsupported transport type: " + std::to_string(static_cast<int>(type)));
    }
  }

  std::string BaseTransport::formatAddress(TransportType type, const std::string& bind_address, int port) {
    switch (type) {
      case TransportType::UNIX:
        // For Unix domain sockets, use the bind_address as the socket path
        return bind_address;
        
      case TransportType::SCTP:
      case TransportType::HTTP:
        // For TCP-based transports, format as "address:port"
        return bind_address + ":" + std::to_string(port);
        
      case TransportType::SCTPS:
      case TransportType::HTTPS:
        // For TLS-based transports, format as "address:port"
        return bind_address + ":" + std::to_string(port);
        
      default:
        throw TransportError("Unknown transport type for address formatting");
    }
  }

} // namespace netd::shared
