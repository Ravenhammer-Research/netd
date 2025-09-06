/* (License header) */

#include <shared/include/exception.hpp>
#include <shared/include/request/kill.hpp>
#include <libnetconf2/netconf.h>
#include <libyang/tree_data.h>

namespace netd::shared::request {


  lyd_node *KillRequest::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw NotImplementedError(
          "Invalid YANG context provided to KillRequest::toYang");
    }

    // TODO: Implement kill-session RPC
    return nullptr;
  }

  std::unique_ptr<KillRequest>
  KillRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                        const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to KillRequest::fromYang");
    }

    return std::make_unique<KillRequest>();
  }

} // namespace netd::shared::request
