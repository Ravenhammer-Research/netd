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

#include <shared/include/netconf/sctp.hpp>
#include <shared/include/exception.hpp>

namespace netd::shared::netconf {

  SCTPTransport::SCTPTransport() : address_(""), port_(0), listening_(false) {}

  SCTPTransport::~SCTPTransport() { 
    stop(); 
  }

  bool SCTPTransport::start([[maybe_unused]] const std::string &address, [[maybe_unused]] int port) {
    throw netd::shared::NotImplementedError("SCTPTransport::start not implemented");
  }

  void SCTPTransport::stop() {
    throw netd::shared::NotImplementedError("SCTPTransport::stop not implemented");
  }

  bool SCTPTransport::isListening() const { 
    throw netd::shared::NotImplementedError("SCTPTransport::isListening not implemented");
  }

  const std::string &SCTPTransport::getAddress() const { 
    throw netd::shared::NotImplementedError("SCTPTransport::getAddress not implemented");
  }

  int SCTPTransport::getPort() const {
    throw netd::shared::NotImplementedError("SCTPTransport::getPort not implemented");
  }

} // namespace netd::shared::netconf
