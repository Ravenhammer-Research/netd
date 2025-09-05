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

#include <freebsd/include/interface/80211.hpp>
#include <freebsd/include/interface/base.hpp>
#include <freebsd/include/interface/bridge.hpp>
#include <freebsd/include/interface/ethernet.hpp>
#include <freebsd/include/interface/ppp.hpp>
#include <freebsd/include/interface/tun.hpp>
#include <freebsd/include/interface/vlan.hpp>

namespace netd::freebsd::interface {

  // Static interface discovery function
  std::vector<std::unique_ptr<netd::shared::interface::base::Ether>>
  Interface::getAllInterfaces() {
    std::vector<std::unique_ptr<netd::shared::interface::base::Ether>>
        allInterfaces;

    // Get ethernet interfaces (already Ether type)
    auto ethernetInterfaces = EthernetInterface::getAllEthernetInterfaces();
    for (auto &iface : ethernetInterfaces) {
      allInterfaces.push_back(std::move(iface));
    }

    // Get bridge interfaces (Master type) - convert to Ether
    auto bridgeInterfaces = BridgeInterface::getAllBridgeInterfaces();
    for (auto &iface : bridgeInterfaces) {
      // Create a new Ether object with the bridge interface name
      auto etherIface =
          std::make_unique<netd::shared::interface::base::Ether>();
      etherIface->setName(iface->getName());
      allInterfaces.push_back(std::move(etherIface));
    }

    // Get VLAN interfaces (already Ether type)
    auto vlanInterfaces = VlanInterface::getAllVlanInterfaces();
    for (auto &iface : vlanInterfaces) {
      allInterfaces.push_back(std::move(iface));
    }

    // Get WiFi interfaces (already Ether type)
    auto wifiInterfaces = WifiInterface::getAllWifiInterfaces();
    for (auto &iface : wifiInterfaces) {
      allInterfaces.push_back(std::move(iface));
    }

    // Get PPP interfaces (already Ether type)
    auto pppInterfaces = PppInterface::getAllPppInterfaces();
    for (auto &iface : pppInterfaces) {
      allInterfaces.push_back(std::move(iface));
    }

    // Get TUN interfaces (Tunnel type) - convert to Ether
    auto tunInterfaces = TunInterface::getAllTunInterfaces();
    for (auto &iface : tunInterfaces) {
      // Create a new Ether object with the TUN interface name
      auto etherIface =
          std::make_unique<netd::shared::interface::base::Ether>();
      etherIface->setName(iface->getName());
      allInterfaces.push_back(std::move(etherIface));
    }

    // TODO: Add remaining tunnel interface types (VXLAN, WireGuard, TAP, etc.)
    // Each should have their own getAll*Interfaces() function
    // and be converted to Ether objects here

    return allInterfaces;
  }

} // namespace netd::freebsd::interface
