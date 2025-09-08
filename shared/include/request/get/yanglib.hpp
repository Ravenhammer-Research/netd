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

#ifndef NETD_REQUEST_GET_YANGLIB_HPP
#define NETD_REQUEST_GET_YANGLIB_HPP

#include <shared/include/request/base.hpp>

namespace netd::shared::request::get {

  class GetYanglibRequest
      : public netd::shared::request::Request<GetYanglibRequest> {
  public:
    GetYanglibRequest() : netd::shared::request::Request<GetYanglibRequest>() {}
    GetYanglibRequest(struct nc_session *session, struct lyd_node *rpc)
        : netd::shared::request::Request<GetYanglibRequest>(session, rpc) {}
    virtual ~GetYanglibRequest() = default;

    // Override base methods
    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<GetYanglibRequest> fromYang(const ly_ctx *ctx,
                                                const lyd_node *node) override;

    // Access methods for filter information
    bool hasYanglibFilter() const { return hasFilter_; }
    std::string getYanglibFilterType() const { return filterType_; }
    std::string getYanglibFilterSelect() const { return filterSelect_; }

  private:
    bool hasFilter_ = false;
    std::string filterType_;
    std::string filterSelect_;
  };

} // namespace netd::shared::request::get

#endif // NETD_REQUEST_GET_YANGLIB_HPP
