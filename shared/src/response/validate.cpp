/* (License header) */

#include <shared/include/exception.hpp>
#include <shared/include/response/validate.hpp>

namespace netd::shared::response {

  ValidateResponse::ValidateResponse() : Response() {}

  lyd_node *ValidateResponse::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw NotImplementedError(
          "Invalid YANG context provided to ValidateResponse::toYang");
    }

    // TODO: Implement validate response
    return nullptr;
  }

  std::unique_ptr<Response>
  ValidateResponse::fromYang([[maybe_unused]] const ly_ctx *ctx,
                             const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to ValidateResponse::fromYang");
    }

    return std::make_unique<ValidateResponse>();
  }

} // namespace netd::shared::response
