#ifndef NETD_REQUEST_VALIDATE_HPP
#define NETD_REQUEST_VALIDATE_HPP

#include <shared/include/request/base.hpp>

namespace netd::shared::request {

  class ValidateRequest : public Request<ValidateRequest> {
  public:
    ValidateRequest();
    virtual ~ValidateRequest() = default;

    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<ValidateRequest> fromYang(const ly_ctx *ctx,
                                              const lyd_node *node) override;
  };

} // namespace netd::shared::request

#endif // NETD_REQUEST_VALIDATE_HPP
