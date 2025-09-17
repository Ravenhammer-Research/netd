/* (License header) */

#include <libnetconf2/netconf.h>
#include <libyang/tree_data.h>
#include <shared/include/exception.hpp>
#include <shared/include/request/commit.hpp>

namespace netd::shared::request {

  lyd_node *CommitRequest::toYang(ly_ctx *ctx) const {
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

    // Create commit element
    lyd_node *commitNode = nullptr;
    if (lyd_new_inner(nullptr, mod, "commit", 0, &commitNode) != LY_SUCCESS) {
      throw netd::shared::ArgumentError(
          "toYang: failed to create commit element");
    }

    // Add confirmed element if present
    if (confirmed_) {
      lyd_node *confirmedNode = nullptr;
      if (lyd_new_inner(commitNode, mod, "confirmed", 0, &confirmedNode) !=
          LY_SUCCESS) {
        lyd_free_tree(commitNode);
        throw netd::shared::ArgumentError(
            "toYang: failed to create confirmed element");
      }

      // Add timeout if specified
      if (timeout_ > 0) {
        lyd_node *timeoutNode = nullptr;
        if (lyd_new_term(confirmedNode, mod, "confirm-timeout",
                         std::to_string(timeout_).c_str(), 0,
                         &timeoutNode) != LY_SUCCESS) {
          lyd_free_tree(commitNode);
          throw netd::shared::ArgumentError(
              "toYang: failed to create confirm-timeout element");
        }
      }
    }

    // Add persist element if present
    if (!persist_.empty()) {
      lyd_node *persistNode = nullptr;
      if (lyd_new_term(commitNode, mod, "persist", persist_.c_str(), 0,
                       &persistNode) != LY_SUCCESS) {
        lyd_free_tree(commitNode);
        throw netd::shared::ArgumentError(
            "toYang: failed to create persist element");
      }
    }

    return commitNode;
  }

  std::unique_ptr<CommitRequest>
  CommitRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                          const lyd_node *node) {
    if (!node) {
      throw netd::shared::ArgumentError(
          "Invalid YANG node provided to CommitRequest::fromYang");
    }

    auto request = std::make_unique<CommitRequest>();

    // Find the commit node
    lyd_node *commitNode = lyd_child(node);
    while (commitNode &&
           strcmp(lyd_node_schema(commitNode)->name, "commit") != 0) {
      commitNode = commitNode->next;
    }

    if (commitNode) {
      // Look for confirmed element
      lyd_node *confirmedNode = lyd_child(commitNode);
      while (confirmedNode &&
             strcmp(lyd_node_schema(confirmedNode)->name, "confirmed") != 0) {
        confirmedNode = confirmedNode->next;
      }

      if (confirmedNode) {
        request->confirmed_ = true;

        // Look for confirm-timeout
        lyd_node *timeoutNode = lyd_child(confirmedNode);
        while (timeoutNode && strcmp(lyd_node_schema(timeoutNode)->name,
                                     "confirm-timeout") != 0) {
          timeoutNode = timeoutNode->next;
        }

        if (timeoutNode) {
          const char *timeoutStr = lyd_get_value(timeoutNode);
          if (timeoutStr) {
            request->timeout_ = std::stoi(timeoutStr);
          }
        }
      }

      // Look for persist element
      lyd_node *persistNode = lyd_child(commitNode);
      while (persistNode &&
             strcmp(lyd_node_schema(persistNode)->name, "persist") != 0) {
        persistNode = persistNode->next;
      }

      if (persistNode) {
        const char *persistStr = lyd_get_value(persistNode);
        if (persistStr) {
          request->persist_ = std::string(persistStr);
        }
      }
    }

    return request;
  }

} // namespace netd::shared::request
