/* (License header) */

#include <shared/include/exception.hpp>
#include <shared/include/request/hello.hpp>

namespace netd::shared::request {

  HelloRequest::HelloRequest() : Request<HelloRequest>() {}

  lyd_node *HelloRequest::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw NotImplementedError(
          "Invalid YANG context provided to HelloRequest::toYang");
    }

    // TODO: Implement hello RPC
    return nullptr;
  }

  std::unique_ptr<HelloRequest>
  HelloRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                         const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to HelloRequest::fromYang");
    }

    return std::make_unique<HelloRequest>();
  }

} // namespace netd::shared::request
