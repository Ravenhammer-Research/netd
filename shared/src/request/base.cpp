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
#include <shared/include/request/get/base.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/request/get/library.hpp>
#include <shared/include/request/commit.hpp>
#include <shared/include/request/copy.hpp>
#include <shared/include/request/delete.hpp>
#include <shared/include/request/discard.hpp>
#include <shared/include/request/edit.hpp>
#include <shared/include/request/hello.hpp>
#include <shared/include/request/lock.hpp>
#include <shared/include/request/unlock.hpp>
#include <shared/include/request/validate.hpp>
#include <shared/include/request/session/close.hpp>
#include <shared/include/request/session/destroy.hpp>
#include <shared/include/yang.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/xml/base.hpp>
#include <shared/include/xml/envelope.hpp>

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


  // Explicit template instantiations for all request types
  template class Request<get::GetRequest>;
  template class Request<get::GetConfigRequest>;
  template class Request<get::GetLibraryRequest>;
  template class Request<CommitRequest>;
  template class Request<CopyConfigRequest>;
  template class Request<DeleteConfigRequest>;
  template class Request<DiscardRequest>;
  template class Request<EditConfigRequest>;
  template class Request<HelloRequest>;
  template class Request<LockRequest>;
  template class Request<UnlockRequest>;
  template class Request<ValidateRequest>;
  template class Request<session::CloseRequest>;
  template class Request<session::DestroyRequest>;

  // fromRpcEnvelope implementation for template class
  template<typename T>
  std::unique_ptr<T> Request<T>::fromRpcEnvelope(const ly_ctx *ctx,
                                                std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope) {
    if (!envelope) {
      throw netd::shared::ArgumentError("Invalid RpcEnvelope provided to Request::fromRpcEnvelope");
    }
    
    // Call fromYang with the envelope's data
    auto request = fromYang(ctx, envelope->getLydData());
    
    // Store the envelope pointer
    if (request) {
      request->envelope_ = envelope;
    }
    
    return request;
  }

  // Explicit template instantiations for fromRpcEnvelope
  template std::unique_ptr<get::GetRequest> Request<get::GetRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
  template std::unique_ptr<get::GetConfigRequest> Request<get::GetConfigRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
  template std::unique_ptr<get::GetLibraryRequest> Request<get::GetLibraryRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
  template std::unique_ptr<CommitRequest> Request<CommitRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
  template std::unique_ptr<CopyConfigRequest> Request<CopyConfigRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
  template std::unique_ptr<DeleteConfigRequest> Request<DeleteConfigRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
  template std::unique_ptr<DiscardRequest> Request<DiscardRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
  template std::unique_ptr<EditConfigRequest> Request<EditConfigRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
  template std::unique_ptr<HelloRequest> Request<HelloRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
  template std::unique_ptr<LockRequest> Request<LockRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
  template std::unique_ptr<UnlockRequest> Request<UnlockRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
  template std::unique_ptr<ValidateRequest> Request<ValidateRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
  template std::unique_ptr<session::CloseRequest> Request<session::CloseRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
  template std::unique_ptr<session::DestroyRequest> Request<session::DestroyRequest>::fromRpcEnvelope(const ly_ctx *ctx, std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);

} // namespace netd::shared::request
