#ifndef NETD_MARSHALLING_NETWORK_HPP
#define NETD_MARSHALLING_NETWORK_HPP

#include <memory>
#include <shared/include/marshalling/data.hpp>
#include <shared/include/vrf.hpp>
#include <vector>

namespace netd::shared::marshalling {

  // Network instance container
  class NetworkInstance : public Data {
  public:
    NetworkInstance();
    virtual ~NetworkInstance() = default;

    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<Data> fromYang(const ly_ctx *ctx,
                                   const lyd_node *node) override;
    std::unique_ptr<Data> exportData() const override;

    // Add VRF instance
    void addVRF(std::unique_ptr<netd::shared::VRF> vrf);
    void addVRF(const netd::shared::VRF &vrf);
    const std::vector<std::unique_ptr<netd::shared::VRF>> &getVRFs() const {
      return vrfs;
    }

  private:
    std::vector<std::unique_ptr<netd::shared::VRF>> vrfs;
  };

} // namespace netd::shared::marshalling

#endif // NETD_MARSHALLING_NETWORK_HPP
