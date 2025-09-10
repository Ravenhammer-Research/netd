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
#include <sstream>
#include <shared/include/request/base.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/yang.hpp>
#include <shared/include/exception.hpp>

namespace netd::shared::request {

  // NOTE: This base class contains stubbed implementations for interface compliance only.
  // The actual functionality should be implemented in derived classes, not here.

  // Base Request class implementation
  // Template class - implementations are in the header
  
  // toXml implementation for template class
  // NOTE: This method should NOT be implemented in base class - it is stubbed for interface compliance only
  template<typename T>
  std::string Request<T>::toXml() const {
    throw netd::shared::NotImplementedError("Request::toXml not implemented");
  }


  // Explicit template instantiation for GetConfigRequest
  template class Request<get::GetConfigRequest>;

} // namespace netd::shared::request
