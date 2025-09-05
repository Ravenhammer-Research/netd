/* (License header) */

#include <shared/include/exception.hpp>
#include <shared/include/marshalling/network.hpp>

namespace netd::shared::marshalling {

  NetworkInstance::NetworkInstance() : Data() {}

  lyd_node *NetworkInstance::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw NotImplementedError(
          "Invalid YANG context provided to NetworkInstance::toYang");
    }

    // If we have stored tree data, return it
    if (getTree()) {
      return getTree();
    }

    // Otherwise, build from VRF data
    lyd_node *containerNode = nullptr;

    for (const auto &vrf : vrfs) {
      if (vrf) {
        lyd_node *vrfNode = vrf->toYang(ctx);
        if (vrfNode) {
          if (!containerNode) {
            containerNode = vrfNode;
          } else {
            lyd_insert_child(containerNode, vrfNode);
          }
        }
      }
    }

    return containerNode;
  }

  std::unique_ptr<Data>
  NetworkInstance::fromYang([[maybe_unused]] const ly_ctx *ctx,
                            const lyd_node *node) {
    if (!node) {
      throw NotImplementedError(
          "Invalid YANG node provided to NetworkInstance::fromYang");
    }

    auto instance = std::make_unique<NetworkInstance>();
    // TODO: Parse VRF data from node
    return instance;
  }

  std::unique_ptr<Data> NetworkInstance::exportData() const {
    auto exported = std::make_unique<NetworkInstance>();
    // Copy the VRFs
    for (const auto &vrf : vrfs) {
      if (vrf) {
        exported->addVRF(*vrf);
      }
    }
    return exported;
  }

  void NetworkInstance::addVRF(std::unique_ptr<netd::shared::VRF> vrf) {
    if (vrf) {
      vrfs.push_back(std::move(vrf));
    }
  }

  void NetworkInstance::addVRF(const netd::shared::VRF &vrf) {
    auto vrfCopy = std::make_unique<netd::shared::VRF>(vrf);
    vrfs.push_back(std::move(vrfCopy));
  }

} // namespace netd::shared::marshalling
