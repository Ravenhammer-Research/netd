#ifndef NETD_REQUEST_COMMIT_HPP
#define NETD_REQUEST_COMMIT_HPP

#include <shared/include/request/base.hpp>

namespace netd::shared::request {

  class CommitRequest : public Request<CommitRequest> {
  public:
    CommitRequest() : Request<CommitRequest>() {}
    CommitRequest(struct nc_session *session, struct lyd_node *rpc)
        : Request<CommitRequest>(session, rpc) {}
    virtual ~CommitRequest() = default;

    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<CommitRequest> fromYang(const ly_ctx *ctx,
                                            const lyd_node *node) override;
  };

} // namespace netd::shared::request

#endif // NETD_REQUEST_COMMIT_HPP
