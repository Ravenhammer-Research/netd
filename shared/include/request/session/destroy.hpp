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

#ifndef NETD_REQUEST_SESSION_DESTROY_HPP
#define NETD_REQUEST_SESSION_DESTROY_HPP

#include <shared/include/request/base.hpp>
#include <shared/include/xml/base.hpp>
#include <shared/include/xml/envelope.hpp>

namespace netd::shared::request::session {

  class DestroyRequest : public Request<DestroyRequest> {
  public:
    DestroyRequest() : Request<DestroyRequest>() {}
    DestroyRequest(netd::shared::netconf::NetconfSession *session, struct lyd_node *rpc)
        : Request<DestroyRequest>(session, rpc) {}
    virtual ~DestroyRequest() = default;

    lyd_node *toYang(ly_ctx *ctx) const ;
    static std::unique_ptr<DestroyRequest> fromYang(const ly_ctx *ctx,
                                                    const lyd_node *node) ;
    static std::unique_ptr<DestroyRequest> fromRpcEnvelope(const ly_ctx *ctx,
                                                          std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope) ;
  };

} // namespace netd::shared::request::session

#endif // NETD_REQUEST_SESSION_DESTROY_HPP
