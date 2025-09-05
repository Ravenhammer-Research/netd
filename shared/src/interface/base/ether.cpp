/*
 * Copyright (c) 2024 Paige Thompson / Ravenhammer Research (paige@paige.bio)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <algorithm>
#include <freebsd/include/interface/80211.hpp>
#include <freebsd/include/interface/bridge.hpp>
#include <freebsd/include/interface/ethernet.hpp>
#include <freebsd/include/interface/ppp.hpp>
#include <freebsd/include/interface/tun.hpp>
#include <freebsd/include/interface/vlan.hpp>
#include <shared/include/interface/base/ether.hpp>
#include <shared/include/yang.hpp>

namespace netd::shared::interface::base {

  // Provide implementations for pure virtual methods to avoid linker errors
  // These are placeholder implementations that should be overridden by derived
  // classes

  bool
  Ether::addAddress(const std::shared_ptr<netd::shared::Address> &address) {
    if (address && address->isValid()) {
      addresses_.push_back(address);
      return true;
    }
    return false;
  }

  bool
  Ether::removeAddress(const std::shared_ptr<netd::shared::Address> &address) {
    auto it = std::find(addresses_.begin(), addresses_.end(), address);
    if (it != addresses_.end()) {
      addresses_.erase(it);
      return true;
    }
    return false;
  }

  std::vector<std::shared_ptr<netd::shared::Address>>
  Ether::getAddresses() const {
    return addresses_;
  }

  bool Ether::addGroup(const std::string &group) {
    if (!group.empty()) {
      groups_.push_back(group);
      return true;
    }
    return false;
  }

  bool Ether::removeGroup(const std::string &group) {
    auto it = std::find(groups_.begin(), groups_.end(), group);
    if (it != groups_.end()) {
      groups_.erase(it);
      return true;
    }
    return false;
  }

  std::vector<std::string> Ether::getGroups() const { return groups_; }

  bool Ether::setMTU(uint16_t mtu) {
    if (mtu >= 68 && mtu <= 9000) { // Reasonable MTU range
      mtu_ = mtu;
      return true;
    }
    return false;
  }

  uint16_t Ether::getMTU() const { return mtu_; }

  bool Ether::setFlags(uint32_t flags) {
    flags_ = flags;
    return true;
  }

  uint32_t Ether::getFlags() const { return flags_; }

  std::string Ether::getName() const { return name_; }

  void Ether::setName(const std::string &name) { name_ = name; }

  bool Ether::isUp() const { return up_; }

  bool Ether::up() {
    up_ = true;
    return true;
  }

  bool Ether::down() {
    up_ = false;
    return true;
  }

  bool Ether::setVRF(uint32_t vrfId) {
    vrfId_ = vrfId;
    return true;
  }

  uint32_t Ether::getVRF() const { return vrfId_; }

  // YANG serialization methods - placeholder implementation
  // These will be implemented when the YANG schemas are properly integrated

  // Static interface discovery function
  std::vector<std::unique_ptr<Ether>> Ether::getAllInterfaces() {
    std::vector<std::unique_ptr<Ether>> allInterfaces;

    // Get ethernet interfaces (already Ether type)
    auto ethernetInterfaces =
        netd::freebsd::interface::EthernetInterface::getAllEthernetInterfaces();
    for (auto &iface : ethernetInterfaces) {
      allInterfaces.push_back(std::move(iface));
    }

    // Get bridge interfaces (Master type) - convert to Ether
    auto bridgeInterfaces =
        netd::freebsd::interface::BridgeInterface::getAllBridgeInterfaces();
    for (auto &iface : bridgeInterfaces) {
      // Create a new Ether object with the bridge interface name
      auto etherIface = std::make_unique<Ether>();
      etherIface->setName(iface->getName());
      allInterfaces.push_back(std::move(etherIface));
    }

    // Get VLAN interfaces (already Ether type)
    auto vlanInterfaces =
        netd::freebsd::interface::VlanInterface::getAllVlanInterfaces();
    for (auto &iface : vlanInterfaces) {
      allInterfaces.push_back(std::move(iface));
    }

    // Get WiFi interfaces (already Ether type)
    auto wifiInterfaces =
        netd::freebsd::interface::WifiInterface::getAllWifiInterfaces();
    for (auto &iface : wifiInterfaces) {
      allInterfaces.push_back(std::move(iface));
    }

    // Get PPP interfaces (already Ether type)
    auto pppInterfaces =
        netd::freebsd::interface::PppInterface::getAllPppInterfaces();
    for (auto &iface : pppInterfaces) {
      allInterfaces.push_back(std::move(iface));
    }

    // Get TUN interfaces (Tunnel type) - convert to Ether
    auto tunInterfaces =
        netd::freebsd::interface::TunInterface::getAllTunInterfaces();
    for (auto &iface : tunInterfaces) {
      // Create a new Ether object with the TUN interface name
      auto etherIface = std::make_unique<Ether>();
      etherIface->setName(iface->getName());
      allInterfaces.push_back(std::move(etherIface));
    }

    // TODO: Add remaining tunnel interface types (VXLAN, WireGuard, TAP, etc.)
    // Each should have their own getAll*Interfaces() function
    // and be converted to Ether objects here

    return allInterfaces;
  }

} // namespace netd::shared::interface::base
