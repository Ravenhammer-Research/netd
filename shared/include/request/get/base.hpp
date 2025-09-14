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

#ifndef NETD_REQUEST_GET_BASE_HPP
#define NETD_REQUEST_GET_BASE_HPP

#include <shared/include/request/base.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/xml/base.hpp>
#include <shared/include/xml/envelope.hpp>

namespace netd::shared::request::get {

  class GetRequest : public netd::shared::request::Request<GetRequest> {
  public:
    GetRequest() : netd::shared::request::Request<GetRequest>() {}
    GetRequest(netd::shared::netconf::NetconfSession *session, struct lyd_node *rpc)
        : netd::shared::request::Request<GetRequest>(session, rpc) {
      // Parse the RPC data to populate filter information
      if (rpc) {
        auto &logger = netd::shared::Logger::getInstance();
        logger.info("GetRequest constructor: calling parseRpcData");
        parseRpcData(rpc);
        logger.info("GetRequest constructor: parseRpcData completed, hasFilter=" + std::string(hasFilter_ ? "true" : "false"));
      }
    }
    virtual ~GetRequest() = default;

    // Override base methods
    lyd_node *toYang(ly_ctx *ctx) const;
    static std::unique_ptr<GetRequest> fromYang(const ly_ctx *ctx,
                                                const lyd_node *node);
    static std::unique_ptr<GetRequest> fromRpcEnvelope(const ly_ctx *ctx,
                                                      std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);

    // Access methods for filter information
    bool hasFilter() const { return hasFilter_; }
    std::string getFilterType() const { return filterType_; }
    std::string getFilterSelect() const { return filterSelect_; }

  protected:
    bool hasFilter_ = false;
    std::string filterType_;
    std::string filterSelect_;
    
    // Helper method to parse RPC data
    void parseRpcData(const lyd_node *node);
  };

} // namespace netd::shared::request::get

#endif // NETD_REQUEST_GET_BASE_HPP
