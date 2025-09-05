/* (License header) */

#include <shared/include/exception.hpp>
#include <shared/include/marshalling/route.hpp>

namespace netd::shared::marshalling {

  Route::Route() : Data() {}

  lyd_node *Route::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw NotImplementedError(
          "Invalid YANG context provided to Route::toYang");
    }

    lyd_node *containerNode = nullptr;

    for ([[maybe_unused]] const auto &route : routes) {
      // TODO: Build YANG nodes from route data
      // This is a placeholder implementation
    }

    return containerNode;
  }

  std::unique_ptr<Data> Route::fromYang([[maybe_unused]] const ly_ctx *ctx,
                                        const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to Route::fromYang");
    }

    auto route = std::make_unique<Route>();
    // TODO: Parse route data from node
    return route;
  }

  std::unique_ptr<Data> Route::exportData() const {
    auto exported = std::make_unique<Route>();
    exported->routes = routes; // Copy the routes
    return exported;
  }

  void Route::addRoute(const std::string &destination,
                       const std::string &gateway,
                       const std::string &interface) {
    routes.emplace_back(destination, gateway, interface);
  }

} // namespace netd::shared::marshalling
