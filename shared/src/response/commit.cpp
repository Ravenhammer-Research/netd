/* (License header) */

#include <shared/include/exception.hpp>
#include <shared/include/response/commit.hpp>

namespace netd::shared::response {

  CommitResponse::CommitResponse() : Response() {}

  lyd_node *CommitResponse::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw NotImplementedError(
          "Invalid YANG context provided to CommitResponse::toYang");
    }

    // TODO: Implement commit response
    return nullptr;
  }

  std::unique_ptr<Response>
  CommitResponse::fromYang([[maybe_unused]] const ly_ctx *ctx,
                           const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to CommitResponse::fromYang");
    }

    return std::make_unique<CommitResponse>();
  }

} // namespace netd::shared::response
