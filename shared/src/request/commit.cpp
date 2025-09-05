/* (License header) */

#include <shared/include/exception.hpp>
#include <shared/include/request/commit.hpp>

namespace netd::shared::request {

  CommitRequest::CommitRequest() : Request<CommitRequest>() {}

  lyd_node *CommitRequest::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw NotImplementedError(
          "Invalid YANG context provided to CommitRequest::toYang");
    }

    // TODO: Implement commit RPC
    return nullptr;
  }

  std::unique_ptr<CommitRequest>
  CommitRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                          const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to CommitRequest::fromYang");
    }

    return std::make_unique<CommitRequest>();
  }

} // namespace netd::shared::request
