#ifndef NETD_REQUEST_CLOSE_HPP
#define NETD_REQUEST_CLOSE_HPP

#include <shared/include/request/base.hpp>

namespace netd::shared::request {

  class CloseRequest : public Request<CloseRequest> {
  public:
    CloseRequest() : Request<CloseRequest>() {}
    CloseRequest(struct nc_session *session, struct lyd_node *rpc) 
      : Request<CloseRequest>(session, rpc) {}
    virtual ~CloseRequest() = default;

    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<CloseRequest> fromYang(const ly_ctx *ctx,
                                           const lyd_node *node) override;
  };

} // namespace netd::shared::request

#endif // NETD_REQUEST_CLOSE_HPP
