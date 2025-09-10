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

#include <client/include/netconf/base.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/exception.hpp>

namespace netd::client::netconf {

  NetconfBase::NetconfBase() : initialized_(false) {
  }

  NetconfBase::~NetconfBase() {
    cleanup();
  }

  bool NetconfBase::initialize() {
    if (initialized_) {
      return true;
    }

    try {
      // Create a YANG context
      ly_ctx* ctx = nullptr;
      LY_ERR err = ly_ctx_new(nullptr, 0, &ctx);
      if (err != LY_SUCCESS || !ctx) {
        return false;
      }
      
      // Create a session with the context
      session_ = std::make_unique<netd::shared::netconf::NetconfSession>(ctx);
      initialized_ = true;
      return true;
    } catch (const std::exception& e) {
      return false;
    }
  }

  void NetconfBase::cleanup() {
    if (session_) {
      session_.reset();
    }
    initialized_ = false;
  }


  bool NetconfBase::isInitialized() const {
    return initialized_;
  }

} // namespace netd::client::netconf
