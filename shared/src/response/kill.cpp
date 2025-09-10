/* (License header) */

#include <libnetconf2/messages_server.h>
#include <libnetconf2/netconf.h>
#include <shared/include/exception.hpp>
#include <shared/include/response/kill.hpp>

namespace netd::shared::response {

  KillResponse::KillResponse() : Response() {}

  lyd_node *KillResponse::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw NotImplementedError(
          "Invalid YANG context provided to KillResponse::toYang");
    }

    // TODO: Implement kill-session response
    return nullptr;
  }

  std::unique_ptr<Response>
  KillResponse::fromYang([[maybe_unused]] const ly_ctx *ctx,
                         const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to KillResponse::fromYang");
    }

    return std::make_unique<KillResponse>();
  }


} // namespace netd::shared::response
