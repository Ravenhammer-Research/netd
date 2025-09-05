#ifndef NETD_REQUEST_HELLO_HPP
#define NETD_REQUEST_HELLO_HPP

#include <shared/include/request/base.hpp>

namespace netd::shared::request {

  class HelloRequest : public Request<HelloRequest> {
  public:
    HelloRequest();
    virtual ~HelloRequest() = default;

    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<HelloRequest> fromYang(const ly_ctx *ctx,
                                           const lyd_node *node) override;
  };

} // namespace netd::shared::request

#endif // NETD_REQUEST_HELLO_HPP
