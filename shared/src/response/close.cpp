/* (License header) */

#include <libnetconf2/messages_server.h>
#include <libnetconf2/netconf.h>
#include <shared/include/exception.hpp>
#include <shared/include/response/close.hpp>

namespace netd::shared::response {

  CloseResponse::CloseResponse() : Response() {}

  lyd_node *CloseResponse::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw netd::shared::ArgumentError("toYang: ctx is null");
    }

    // Close-session response is just an empty OK response
    // No YANG data needed, just return nullptr to indicate empty response
    return nullptr;
  }

  std::unique_ptr<Response>
  CloseResponse::fromYang([[maybe_unused]] const ly_ctx *ctx,
                          const lyd_node *node) {
    if (!node) {
      throw netd::shared::ArgumentError("Invalid YANG node provided to CloseResponse::fromYang");
    }

    // For close-session response, we just need to create a simple response object
    // No additional parsing needed since close-session response is just OK
    return std::make_unique<CloseResponse>();
  }


} // namespace netd::shared::response
