#ifndef NETD_RESPONSE_HELLO_HPP
#define NETD_RESPONSE_HELLO_HPP

#include <shared/include/response/base.hpp>

namespace netd::shared::response {

  class HelloResponse : public Response {
  public:
    HelloResponse();
    virtual ~HelloResponse() = default;

    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<Response> fromYang(const ly_ctx *ctx,
                                       const lyd_node *node) override;
    struct nc_server_reply *
    toNetconfReply(struct nc_session *session) const override;
  };

} // namespace netd::shared::response

#endif // NETD_RESPONSE_HELLO_HPP
