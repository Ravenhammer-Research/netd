#ifndef NETD_REQUEST_COMMIT_HPP
#define NETD_REQUEST_COMMIT_HPP

#include <shared/include/request/base.hpp>
#include <shared/include/xml/base.hpp>
#include <shared/include/xml/envelope.hpp>

namespace netd::shared::request {

  class CommitRequest : public Request<CommitRequest> {
  public:
    CommitRequest() : Request<CommitRequest>() {}
    CommitRequest(netd::shared::netconf::NetconfSession *session, struct lyd_node *rpc)
        : Request<CommitRequest>(session, rpc) {}
    virtual ~CommitRequest() = default;

    lyd_node *toYang(ly_ctx *ctx) const;
    static std::unique_ptr<CommitRequest> fromYang(const ly_ctx *ctx,
                                                  const lyd_node *node);
    static std::unique_ptr<CommitRequest> fromRpcEnvelope(const ly_ctx *ctx,
                                                         std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);

  private:
    bool confirmed_ = false;
    int timeout_ = 0;
    std::string persist_;
  };

} // namespace netd::shared::request

#endif // NETD_REQUEST_COMMIT_HPP
