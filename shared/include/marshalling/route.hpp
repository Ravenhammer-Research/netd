#ifndef NETD_MARSHALLING_ROUTE_HPP
#define NETD_MARSHALLING_ROUTE_HPP

#include <shared/include/marshalling/data.hpp>
#include <string>
#include <tuple>
#include <vector>

namespace netd::shared::marshalling {

  // Route container
  class Route : public Data {
  public:
    Route();
    virtual ~Route() = default;

    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<Data> fromYang(const ly_ctx *ctx,
                                   const lyd_node *node) override;
    std::unique_ptr<Data> exportData() const override;

    // Add route data
    void addRoute(const std::string &destination, const std::string &gateway,
                  const std::string &interface);
    const std::vector<std::tuple<std::string, std::string, std::string>> &
    getRoutes() const {
      return routes;
    }

  private:
    std::vector<std::tuple<std::string, std::string, std::string>> routes;
  };

} // namespace netd::shared::marshalling

#endif // NETD_MARSHALLING_ROUTE_HPP
