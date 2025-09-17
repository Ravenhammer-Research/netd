/* (License header) */

#include <libnetconf2/messages_server.h>
#include <libnetconf2/netconf.h>
#include <shared/include/exception.hpp>
#include <shared/include/response/hello.hpp>
#include <shared/include/yang.hpp>

namespace netd::shared::response {

  HelloResponse::HelloResponse() : Response() {}

  lyd_node *HelloResponse::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw netd::shared::ArgumentError("toYang: ctx is null");
    }

    // Get the ietf-netconf module
    const struct lys_module *mod =
        ly_ctx_get_module(ctx, "ietf-netconf", "2011-06-01");
    if (!mod) {
      throw netd::shared::ArgumentError(
          "toYang: ietf-netconf module not found");
    }

    // Create hello element with server capabilities
    lyd_node *helloNode = nullptr;
    if (lyd_new_inner(nullptr, mod, "hello", 0, &helloNode) != LY_SUCCESS) {
      throw netd::shared::ArgumentError(
          "toYang: failed to create hello element");
    }

    // Add capabilities container
    lyd_node *capabilitiesNode = nullptr;
    if (lyd_new_inner(helloNode, mod, "capabilities", 0, &capabilitiesNode) !=
        LY_SUCCESS) {
      lyd_free_tree(helloNode);
      throw netd::shared::ArgumentError(
          "toYang: failed to create capabilities element");
    }

    // Get dynamic capabilities from YANG context
    auto serverCapabilities =
        netd::shared::Yang::getInstance().getCapabilities();

    // Add each capability
    for (const auto &capability : serverCapabilities) {
      lyd_node *capabilityNode = nullptr;
      if (lyd_new_term(capabilitiesNode, mod, "capability", capability.c_str(),
                       0, &capabilityNode) != LY_SUCCESS) {
        lyd_free_tree(helloNode);
        throw netd::shared::ArgumentError(
            "toYang: failed to create capability element");
      }
    }

    return helloNode;
  }

  std::unique_ptr<Response>
  HelloResponse::fromYang([[maybe_unused]] const ly_ctx *ctx,
                          const lyd_node *node) {
    if (!node) {
      throw netd::shared::ArgumentError(
          "Invalid YANG node provided to HelloResponse::fromYang");
    }

    // For hello response, we just need to create a simple response object
    // No additional parsing needed since hello response is just capabilities
    return std::make_unique<HelloResponse>();
  }

} // namespace netd::shared::response
