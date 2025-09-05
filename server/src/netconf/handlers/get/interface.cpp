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

#include <freebsd/include/interface/base.hpp>
#include <libnetconf2/messages_server.h>
#include <libnetconf2/netconf.h>
#include <libyang/libyang.h>
#include <server/include/netconf/handlers.hpp>
#include <shared/include/interface/base/ether.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/marshalling/interface.hpp>

namespace netd::server::netconf::handlers {

  // Helper function to get all interface names from the system
  std::vector<std::string> RpcHandler::getAllInterfaceNames() {
    std::vector<std::string> interfaceNames;

    // Use FreeBSD interface discovery to get all interfaces as Ether objects
    auto interfaces = netd::freebsd::interface::Interface::getAllInterfaces();

    for (const auto &iface : interfaces) {
      if (iface) {
        interfaceNames.push_back(iface->getName());
      }
    }

    return interfaceNames;
  }

  // Helper function to get interface information based on name
  std::unique_ptr<netd::shared::interface::base::Ether>
  RpcHandler::getInterfaceInfo(const std::string &ifName) {
    // Use FreeBSD interface discovery to get all interfaces
    auto interfaces = netd::freebsd::interface::Interface::getAllInterfaces();

    // Find the interface with the matching name
    for (auto &iface : interfaces) {
      if (iface && iface->getName() == ifName) {
        return std::move(iface);
      }
    }

    // If not found, return nullptr
    return nullptr;
  }

  // Function to handle interface-specific requests
  struct nc_server_reply *
  handleInterfaceRequest(struct nc_session *session,
                         [[maybe_unused]] struct lyd_node *rpc) {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Handling interface request");

    try {
      // Get all network interfaces from the system using FreeBSD interface
      // discovery
      auto interfaces = netd::freebsd::interface::Interface::getAllInterfaces();
      logger.info("Found " + std::to_string(interfaces.size()) + " interfaces");

      // Process each interface
      for (const auto &iface : interfaces) {
        if (iface) {
          logger.info("Processing interface: " + iface->getName() +
                      " type: " + iface->getType());

          // TODO: Convert interface-specific data to YANG format
          // This is where we would convert each interface type to its YANG
          // representation
        }
      }

      // For now, return a simple OK response
      // TODO: Implement actual interface data response
      return nc_server_reply_ok();

    } catch (const std::exception &e) {
      logger.error("Exception in interface handler: " + std::string(e.what()));
      return nc_server_reply_err(
          nc_err(nc_session_get_ctx(session), NC_ERR_OP_FAILED, e.what()));
    }
  }

} // namespace netd::server::netconf::handlers
