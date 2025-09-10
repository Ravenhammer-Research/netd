/* (License header) */

#include <libnetconf2/netconf.h>
#include <libyang/tree_data.h>
#include <shared/include/exception.hpp>
#include <shared/include/request/hello.hpp>

namespace netd::shared::request {

  lyd_node *HelloRequest::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw netd::shared::ArgumentError("toYang: ctx is null");
    }
    
    // Validate that the provided context matches the session context
    if (session_ && session_->getContext() != ctx) {
      throw netd::shared::ArgumentError("toYang: provided context does not match session context");
    }

    // Get the ietf-netconf module
    const struct lys_module *mod = ly_ctx_get_module(ctx, "ietf-netconf", "2011-06-01");
    if (!mod) {
      throw netd::shared::ArgumentError("toYang: ietf-netconf module not found");
    }

    // Create hello element
    lyd_node *helloNode = nullptr;
    if (lyd_new_inner(nullptr, mod, "hello", 0, &helloNode) != LY_SUCCESS) {
      throw netd::shared::ArgumentError("toYang: failed to create hello element");
    }

    // Add capabilities if present
    if (!capabilities_.empty()) {
      lyd_node *capabilitiesNode = nullptr;
      if (lyd_new_inner(helloNode, mod, "capabilities", 0, &capabilitiesNode) != LY_SUCCESS) {
        lyd_free_tree(helloNode);
        throw netd::shared::ArgumentError("toYang: failed to create capabilities element");
      }

      // Add each capability
      for (const auto &capability : capabilities_) {
        lyd_node *capabilityNode = nullptr;
        if (lyd_new_term(capabilitiesNode, mod, "capability", capability.c_str(), 0, &capabilityNode) != LY_SUCCESS) {
          lyd_free_tree(helloNode);
          throw netd::shared::ArgumentError("toYang: failed to create capability element");
        }
      }
    }

    return helloNode;
  }

  std::unique_ptr<HelloRequest>
  HelloRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                         const lyd_node *node) {
    if (!node) {
      throw netd::shared::ArgumentError("Invalid YANG node provided to HelloRequest::fromYang");
    }

    auto request = std::make_unique<HelloRequest>();

    // Find the hello node
    lyd_node *helloNode = lyd_child(node);
    while (helloNode && strcmp(lyd_node_schema(helloNode)->name, "hello") != 0) {
      helloNode = helloNode->next;
    }

    if (helloNode) {
      // Look for capabilities
      lyd_node *capabilitiesNode = lyd_child(helloNode);
      while (capabilitiesNode && strcmp(lyd_node_schema(capabilitiesNode)->name, "capabilities") != 0) {
        capabilitiesNode = capabilitiesNode->next;
      }

      if (capabilitiesNode) {
        // Extract capabilities
        lyd_node *capabilityNode = lyd_child(capabilitiesNode);
        while (capabilityNode) {
          if (strcmp(lyd_node_schema(capabilityNode)->name, "capability") == 0) {
            const char *capability = lyd_get_value(capabilityNode);
            if (capability) {
              request->capabilities_.push_back(std::string(capability));
            }
          }
          capabilityNode = capabilityNode->next;
        }
      }
    }

    return request;
  }

} // namespace netd::shared::request
