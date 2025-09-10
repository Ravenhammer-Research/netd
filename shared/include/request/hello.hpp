#ifndef NETD_REQUEST_HELLO_HPP
#define NETD_REQUEST_HELLO_HPP

#include <shared/include/request/base.hpp>

namespace netd::shared::request {

  class HelloRequest : public Request<HelloRequest> {
  public:
    HelloRequest() : Request<HelloRequest>() {}
    HelloRequest(netd::shared::netconf::NetconfSession *session, struct lyd_node *rpc)
        : Request<HelloRequest>(session, rpc) {}
    virtual ~HelloRequest() = default;

    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<HelloRequest> fromYang(const ly_ctx *ctx,
                                           const lyd_node *node) override;

  private:
    std::vector<std::string> capabilities_;
  };

} // namespace netd::shared::request

#endif // NETD_REQUEST_HELLO_HPP
