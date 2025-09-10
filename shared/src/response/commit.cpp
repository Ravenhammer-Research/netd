/* (License header) */

#include <libnetconf2/messages_server.h>
#include <libnetconf2/netconf.h>
#include <shared/include/exception.hpp>
#include <shared/include/response/commit.hpp>

namespace netd::shared::response {

  CommitResponse::CommitResponse() : Response() {}

  lyd_node *CommitResponse::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw netd::shared::ArgumentError("toYang: ctx is null");
    }

    // Commit response is just an empty OK response
    // No YANG data needed, just return nullptr to indicate empty response
    return nullptr;
  }

  std::unique_ptr<Response>
  CommitResponse::fromYang([[maybe_unused]] const ly_ctx *ctx,
                           const lyd_node *node) {
    if (!node) {
      throw netd::shared::ArgumentError("Invalid YANG node provided to CommitResponse::fromYang");
    }

    // For commit response, we just need to create a simple response object
    // No additional parsing needed since commit response is just OK
    return std::make_unique<CommitResponse>();
  }


} // namespace netd::shared::response
