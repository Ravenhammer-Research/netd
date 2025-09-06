/* (License header) */

#include <shared/include/exception.hpp>
#include <shared/include/request/validate.hpp>
#include <libnetconf2/netconf.h>
#include <libyang/tree_data.h>

namespace netd::shared::request {


  lyd_node *ValidateRequest::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw NotImplementedError(
          "Invalid YANG context provided to ValidateRequest::toYang");
    }

    // TODO: Implement validate RPC
    return nullptr;
  }

  std::unique_ptr<ValidateRequest>
  ValidateRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                            const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to ValidateRequest::fromYang");
    }

    return std::make_unique<ValidateRequest>();
  }

} // namespace netd::shared::request
