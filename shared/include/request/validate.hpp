#ifndef NETD_REQUEST_VALIDATE_HPP
#define NETD_REQUEST_VALIDATE_HPP

#include <shared/include/request/base.hpp>
#include <shared/include/xml/base.hpp>
#include <shared/include/xml/envelope.hpp>

namespace netd::shared::request {

  class ValidateRequest : public Request<ValidateRequest> {
  public:
    ValidateRequest() : Request<ValidateRequest>() {}
    ValidateRequest(netd::shared::netconf::NetconfSession *session, struct lyd_node *rpc)
        : Request<ValidateRequest>(session, rpc) {}
    virtual ~ValidateRequest() = default;

    lyd_node *toYang(ly_ctx *ctx) const ;
    static std::unique_ptr<ValidateRequest> fromYang(const ly_ctx *ctx,
                                                    const lyd_node *node) ;
    static std::unique_ptr<ValidateRequest> fromRpcEnvelope(const ly_ctx *ctx,
                                                           std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope) ;
  };

} // namespace netd::shared::request

#endif // NETD_REQUEST_VALIDATE_HPP
