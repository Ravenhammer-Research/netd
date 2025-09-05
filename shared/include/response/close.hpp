#ifndef NETD_RESPONSE_CLOSE_HPP
#define NETD_RESPONSE_CLOSE_HPP

#include <shared/include/response/base.hpp>

namespace netd::shared::response {

  class CloseResponse : public Response {
  public:
    CloseResponse();
    virtual ~CloseResponse() = default;

    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<Response> fromYang(const ly_ctx *ctx,
                                       const lyd_node *node) override;
  };

} // namespace netd::shared::response

#endif // NETD_RESPONSE_CLOSE_HPP
