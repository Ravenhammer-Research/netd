/* (License header) */

#include <libnetconf2/netconf.h>
#include <libyang/tree_data.h>
#include <shared/include/exception.hpp>
#include <shared/include/request/hello.hpp>
#include <shared/include/yang.hpp>

namespace netd::shared::request {

  lyd_node *HelloRequest::toYang([[maybe_unused]] ly_ctx *ctx) const {
    throw netd::shared::NotImplementedError("HelloRequest::toYang not implemented");
  }

  std::unique_ptr<HelloRequest>
  HelloRequest::fromYang([[maybe_unused]] const ly_ctx *ctx,
                         [[maybe_unused]] const lyd_node *node) {
    throw netd::shared::NotImplementedError("HelloRequest::fromYang not implemented");
  }

  std::unique_ptr<HelloRequest>
  HelloRequest::fromHelloToServer(const netd::shared::xml::HelloToServer& hello_to_server) {
    auto request = std::make_unique<HelloRequest>();
    
    // Extract capabilities from HelloToServer
    request->capabilities_ = hello_to_server.getCapabilities();
    
    // HelloToServer doesn't have session ID (client-to-server hello)
    // session_id_ remains -1 (no session ID)
    
    return request;
  }

} // namespace netd::shared::request
