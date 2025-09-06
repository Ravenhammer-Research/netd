#ifndef NETD_RESPONSE_KILL_HPP
#define NETD_RESPONSE_KILL_HPP

#include <shared/include/response/base.hpp>

namespace netd::shared::response {

  class KillResponse : public Response {
  public:
    KillResponse();
    virtual ~KillResponse() = default;

    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<Response> fromYang(const ly_ctx *ctx,
                                       const lyd_node *node) override;
    struct nc_server_reply *
    toNetconfReply(struct nc_session *session) const override;
  };

} // namespace netd::shared::response

#endif // NETD_RESPONSE_KILL_HPP
