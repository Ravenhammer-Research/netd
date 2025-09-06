/* (License header) */

#include <libnetconf2/messages_server.h>
#include <libnetconf2/netconf.h>
#include <shared/include/exception.hpp>
#include <shared/include/response/close.hpp>

namespace netd::shared::response {

  CloseResponse::CloseResponse() : Response() {}

  lyd_node *CloseResponse::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw NotImplementedError(
          "Invalid YANG context provided to CloseResponse::toYang");
    }

    // TODO: Implement close-session response
    return nullptr;
  }

  std::unique_ptr<Response>
  CloseResponse::fromYang([[maybe_unused]] const ly_ctx *ctx,
                          const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to CloseResponse::fromYang");
    }

    return std::make_unique<CloseResponse>();
  }

  struct nc_server_reply *
  CloseResponse::toNetconfReply(struct nc_session *session) const {
    // Use the base class implementation which handles error cases
    return Response::toNetconfReply(session);
  }

} // namespace netd::shared::response
