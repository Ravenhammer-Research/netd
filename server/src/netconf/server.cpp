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

#include <server/include/netconf/server.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/unix.hpp>
#include <server/include/netconf/session.hpp>
#include <shared/include/netconf/rpc.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/yang.hpp>
#include <server/include/signal.hpp>
#ifdef HAVE_LLDP
#include <shared/include/lldp/client.hpp>
#endif
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

namespace netd::server::netconf {

  NetconfServer::NetconfServer(netd::shared::TransportType transport_type, 
                               const std::string& bind_address, 
                               int port)
      : transport_type_(transport_type), bind_address_(bind_address), port_(port), running_(false) {
  }

  NetconfServer::~NetconfServer() {
    stop();
  }

  std::unique_ptr<netd::shared::BaseTransport> NetconfServer::createTransport() {
    switch (transport_type_) {
      case netd::shared::TransportType::UNIX:
        return std::make_unique<netd::shared::UnixTransport>();
      default:
        throw netd::shared::TransportError("Unsupported transport type: " + std::to_string(static_cast<int>(transport_type_)));
    }
  }

  bool NetconfServer::start() {
    if (running_) {
      return true;
    }

    // Create transport
    transport_ = createTransport();
    if (!transport_) {
      throw netd::shared::TransportError("Failed to create transport");
    }

    // Start transport
    if (!transport_->start(bind_address_)) {
      throw netd::shared::BindError("Failed to start transport on " + bind_address_);
    }

#ifdef HAVE_LLDP
    lldp_client_ = std::make_unique<netd::shared::lldp::Client>();
    
    try {
      auto &logger = netd::shared::Logger::getInstance();
      lldp_client_->initialize();
      
      // Configure LLDP system settings
      auto config = lldp_client_->getConfiguration();
      if (config) {
        config->setHostname("netd-server");
        config->setDescription("NETD NETCONF Server - Network Management Interface");
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
        logger.info("LLDP Local chassis description: " + chassis->getChassisDescription());
        {
          std::stringstream ss;
          ss << "LLDP Available capabilities: 0x" << std::hex << std::setfill('0') << std::setw(8) << chassis->getCapabilitiesAvailable();
          logger.info(ss.str());
        }
        {
          std::stringstream ss;
          ss << "LLDP Enabled capabilities: 0x" << std::hex << std::setfill('0') << std::setw(8) << chassis->getCapabilitiesEnabled();
          logger.info(ss.str());
        }
        
        auto mgmt_addrs = chassis->getManagementAddresses();
        logger.info("LLDP Management addresses: " + std::to_string(mgmt_addrs.size()) + " found");
        for (const auto& addr : mgmt_addrs) {
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
          logger.info("LLDP Found " + std::to_string(local_ports.size()) + " local ports, managing custom TLVs with OUI: " + std::string(NETD_OUI));

          for (auto& local_port : local_ports) {
              if (local_port && local_port->isValid()) {
                  // First, clear all existing custom TLVs to avoid duplicates
                  local_port->clearCustomTLVs();

                  // Add a custom TLV for NETD server information
                  local_port->addCustomTLV(
                    NETD_OUI,      // OUI defined in CMakeLists.txt
                    1,             // OUI subtype
                    "NETD-SERVER:1.0:NETCONF:UNIX",  // Custom info string
                    "add"          // Operation (use "add" instead of "advertise")
                  );
                  
                  // List existing custom TLVs on this port
                  auto custom_tlvs = local_port->getCustomTLVs();
                  logger.debug_lldp("Custom TLVs on port: " + std::to_string(custom_tlvs.size()) + " found");
                  for (const auto& tlv : custom_tlvs) {
                    if (tlv && tlv->isValid()) {
                      logger.debug_lldp("  Custom TLV: OUI=" + tlv->getOUI() + 
                                 " subtype=" + std::to_string(tlv->getOUISubtype()) +
                                 " info=" + tlv->getOUIInfoString());
                    }
                  }
              }
          }
          
          logger.info("LLDP configuration completed successfully on all local ports");
      } else {
          logger.log(netd::shared::LogLevel::WARNING, "No local ports found - custom TLVs will not be configured");
      }
      
      // Get and log actual LLDP interfaces
      auto lldp_interfaces = lldp_client_->getLLDPInterfaces();
      logger.debug_lldp("LLDP interfaces: " + std::to_string(lldp_interfaces.size()) + " found");
      for (const auto& iface : lldp_interfaces) {
        logger.debug_lldp("LLDP interface: " + iface);
      }
      
      auto link_local_addrs = lldp_client_->getLinkLocalAddresses();
      logger.debug_lldp("Link-local addresses: " + std::to_string(link_local_addrs.size()) + " found");
      for (const auto& [interface_name, address] : link_local_addrs) {
        logger.debug_lldp("Link-local address: " + interface_name + " = " + address);
      }
    } catch (const netd::shared::LLDPError& e) {
      // LLDP daemon not available, continue without LLDP
      lldp_client_.reset();
    }
#endif

    running_ = true;
    return true;
  }

  void NetconfServer::stop() {
    if (!running_) {
      return;
    }

    running_ = false;

#ifdef HAVE_LLDP
    if (lldp_client_) {
      lldp_client_->cleanup();
      lldp_client_.reset();
    }
#endif

    if (transport_) {
      transport_->stop();
      transport_.reset();
    }

    for (auto& thread : session_threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    session_threads_.clear();
  }

  void NetconfServer::run() {
    if (!running_ || !transport_) {
      throw netd::shared::TransportError("Server not started or transport not available");
    }

    auto &logger = netd::shared::Logger::getInstance();
    logger.info("NETCONF server is now running and waiting for connections...");

    while (running_ && netd::server::isRunning()) {
      int client_socket = transport_->acceptConnection();
      if (client_socket < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          std::this_thread::sleep_for(std::chrono::milliseconds(8));
          continue;
        }
        if (running_) {
          throw netd::shared::AcceptError("Failed to accept connection: " + std::string(strerror(errno)));
        }
        continue;
      }

      auto& yang = netd::shared::Yang::getInstance();
      ly_ctx* ctx = yang.getContext();
      auto session = std::make_unique<netd::shared::netconf::NetconfSession>(
          ctx, client_socket, netd::shared::TransportType::UNIX);

      auto& session_manager = SessionManager::getInstance();
      session_manager.addSession(std::move(session));

      session_threads_.emplace_back([this, client_socket]() {
        this->handleClientSession(client_socket);
      });
    }

    for (auto& thread : session_threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    session_threads_.clear();
  }

  void NetconfServer::handleClientSession(int client_socket) {
    auto& session_manager = SessionManager::getInstance();
    auto* session = session_manager.getSession(client_socket);
    if (!session) {
      throw netd::shared::ArgumentError("Session not found for socket: " + std::to_string(client_socket));
    }

    while (running_ && netd::server::isRunning()) {
      std::string data = transport_->receiveData(client_socket);
      if (data.empty()) {
        break;
      }

      std::istringstream data_stream(data);
      auto response_stream = netd::shared::netconf::Rpc::processRpc(data_stream, session);
      
      if (response_stream) {
        std::string response((std::istreambuf_iterator<char>(*response_stream)),
                            std::istreambuf_iterator<char>());
        
        if (!transport_->sendData(client_socket, response)) {
          throw netd::shared::SendError("Failed to send RPC response");
        }
      }
    }

    session_manager.removeSession(client_socket);
    transport_->closeConnection(client_socket);
  }

} // namespace netd::server::netconf