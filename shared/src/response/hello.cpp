/* (License header) */

#include <shared/include/exception.hpp>
#include <shared/include/response/hello.hpp>

namespace netd::shared::response {

  HelloResponse::HelloResponse() : Response() {}

  lyd_node *HelloResponse::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw NotImplementedError(
          "Invalid YANG context provided to HelloResponse::toYang");
    }

    // TODO: Implement hello response
    return nullptr;
  }

  std::unique_ptr<Response>
  HelloResponse::fromYang([[maybe_unused]] const ly_ctx *ctx,
                          const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to HelloResponse::fromYang");
    }

    return std::make_unique<HelloResponse>();
  }

} // namespace netd::shared::response
