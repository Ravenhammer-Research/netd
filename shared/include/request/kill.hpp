#ifndef NETD_REQUEST_KILL_HPP
#define NETD_REQUEST_KILL_HPP

#include <shared/include/request/base.hpp>

namespace netd::shared::request {

  class KillRequest : public Request<KillRequest> {
  public:
    KillRequest() : Request<KillRequest>() {}
    KillRequest(struct nc_session *session, struct lyd_node *rpc) 
      : Request<KillRequest>(session, rpc) {}
    virtual ~KillRequest() = default;

    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<KillRequest> fromYang(const ly_ctx *ctx,
                                          const lyd_node *node) override;
  };

} // namespace netd::shared::request

#endif // NETD_REQUEST_KILL_HPP
