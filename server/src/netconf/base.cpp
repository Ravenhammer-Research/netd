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

#include <server/include/netconf/base.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>

namespace netd::server::netconf {

  bool Server::initializeLLDP() {
#ifdef HAVE_LLDP
    try {
      lldp_client_ = std::make_unique<netd::shared::lldp::Client>();

      auto &logger = netd::shared::Logger::getInstance();
      lldp_client_->initialize();

      // Configure LLDP system settings
      auto config = lldp_client_->getConfiguration();
      if (config) {
        config->setHostname("netd-server");
        config->setDescription(
            "NETD NETCONF Server - Network Management Interface");
        config->setPlatform("NETD");
        config->setTxInterval(30);
        config->setTxHold(4);
        config->setReceiveOnly(false);
      }

      // Configure chassis capabilities
      auto chassis = lldp_client_->getLocalChassis();
      if (chassis) {
        // Station capability enabled, others disabled
        // This would need to be done through the chassis atom directly
        logger.info("LLDP Local chassis ID: " + chassis->getChassisId());
        logger.info("LLDP Local chassis name: " + chassis->getChassisName());
        logger.info("LLDP Local chassis description: " +
                    chassis->getChassisDescription());
        {
          std::stringstream ss;
          ss << "LLDP Available capabilities: 0x" << std::hex
             << std::setfill('0') << std::setw(8)
             << chassis->getCapabilitiesAvailable();
          logger.info(ss.str());
        }
        {
          std::stringstream ss;
          ss << "LLDP Enabled capabilities: 0x" << std::hex << std::setfill('0')
             << std::setw(8) << chassis->getCapabilitiesEnabled();
          logger.info(ss.str());
        }

        auto mgmt_addrs = chassis->getManagementAddresses();
        logger.info("LLDP Management addresses: " +
                    std::to_string(mgmt_addrs.size()) + " found");
        for (const auto &addr : mgmt_addrs) {
          if (addr) {
            logger.info("LLDP Management address: " + addr->getString());
          }
        }
      }

      // Configure LLDP settings
      auto mgmt_config = lldp_client_->getConfiguration();
      if (mgmt_config) {
        // Set management address pattern to only advertise link-local addresses
        mgmt_config->setManagementPattern("fe80::/10,169.254.0.0/16");
        // Set transmit interval to 30 seconds
        mgmt_config->setTxInterval(30);

        // Enable chassis capabilities advertisement
        mgmt_config->setChassisCapAdvertise(true);
      }

      // Configure custom TLVs on all local ports
      auto local_ports = lldp_client_->getAllLocalPorts();
      if (!local_ports.empty()) {
        logger.info("LLDP Found " + std::to_string(local_ports.size()) +
                    " local ports, managing custom TLVs with OUI: " +
                    std::string(NETD_OUI));

        for (auto &local_port : local_ports) {
          if (local_port && local_port->isValid()) {
            // First, clear all existing custom TLVs to avoid duplicates
            local_port->clearCustomTLVs();

            // Add a custom TLV for NETD server information
            local_port->addCustomTLV(
                NETD_OUI,                       // OUI defined in CMakeLists.txt
                1,                              // OUI subtype
                "NETD-SERVER:1.0:NETCONF:UNIX", // Custom info string
                "add" // Operation (use "add" instead of "advertise")
            );

            // List existing custom TLVs on this port
            auto custom_tlvs = local_port->getCustomTLVs();
            logger.debug_lldp("Custom TLVs on port: " +
                              std::to_string(custom_tlvs.size()) + " found");
            for (const auto &tlv : custom_tlvs) {
              if (tlv && tlv->isValid()) {
                logger.debug_lldp(
                    "  Custom TLV: OUI=" + tlv->getOUI() +
                    " subtype=" + std::to_string(tlv->getOUISubtype()) +
                    " info=" + tlv->getOUIInfoString());
              }
            }
          }
        }

        logger.info(
            "LLDP configuration completed successfully on all local ports");
      } else {
        logger.log(netd::shared::LogLevel::WARNING,
                   "No local ports found - custom TLVs will not be configured");
      }

      // Get and log actual LLDP interfaces
      auto lldp_interfaces = lldp_client_->getLLDPInterfaces();
      logger.debug_lldp("LLDP interfaces: " +
                        std::to_string(lldp_interfaces.size()) + " found");
      for (const auto &iface : lldp_interfaces) {
        logger.debug_lldp("LLDP interface: " + iface);
      }

      auto link_local_addrs = lldp_client_->getLinkLocalAddresses();
      logger.debug_lldp("Link-local addresses: " +
                        std::to_string(link_local_addrs.size()) + " found");
      for (const auto &[interface_name, address] : link_local_addrs) {
        logger.debug_lldp("Link-local address: " + interface_name + " = " +
                          address);
      }

      return true;
    } catch (const netd::shared::LLDPError &e) {
      // LLDP daemon not available, continue without LLDP
      lldp_client_.reset();
      return true; // Not an error, just not available
    }
#else
    return true; // LLDP not compiled in, not an error
#endif
  }

  void Server::cleanupLLDP() {
#ifdef HAVE_LLDP
    if (lldp_client_) {
      lldp_client_->cleanup();
      lldp_client_.reset();
    }
#endif
  }

} // namespace netd::server::netconf