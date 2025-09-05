/* (License header) */

#include <shared/include/exception.hpp>
#include <shared/include/request/close.hpp>

namespace netd::shared::request {

  CloseRequest::CloseRequest() : Request<CloseRequest>() {}

  lyd_node *CloseRequest::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw NotImplementedError(
          "Invalid YANG context provided to CloseRequest::toYang");
    }

    // TODO: Implement close-session RPC
    return nullptr;
  }

  std::unique_ptr<CloseRequest>
  CloseRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                         const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to CloseRequest::fromYang");
    }

    return std::make_unique<CloseRequest>();
  }

} // namespace netd::shared::request
