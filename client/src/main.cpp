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

#include <client/include/netconf/client.hpp>
#include <client/include/parser.hpp>
#include <client/include/processor.hpp>
#include <client/include/tui.hpp>
#include <shared/include/lldp/client.hpp>
#include <iostream>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include <iomanip>
#include <unistd.h>
#include <getopt.h>
#include <vector>

void printUsage(const char *progname) {
  std::cerr << "Usage: " << progname << " [options]\n";
  std::cerr << "Options:\n";
  std::cerr << "  -s <path>    Socket path (default: /tmp/netd.sock)\n";
  std::cerr << "  -L           List LLDP neighbors and exit\n";
  std::cerr << "  -d           Enable debug logging\n";
  std::cerr << "  -dd          Enable debug and trace logging\n";
  std::cerr << "  -ddd         Enable debug, trace, and timestamps\n";
  std::cerr << "  -h           Show this help message\n";
}

void showStartupInfo(netd::client::tui::TUI& tui) {
  tui.putLine("NetD Client 1.0");
  tui.putLine(" ");
  tui.putLine("Copyright (c) 2025 RavenHammer Research. All rights reserved.");
  tui.putLine(" ");
  tui.putLine("Credits:");
  tui.putLine(" ");
  tui.putLine("  FreeBSD - Copyright (c) The Regents of the University of California.");
  tui.putLine("           All rights reserved.");
  tui.putLine("  libyang - Copyright (c) 2015-2025, CESNET. All rights reserved.");
  tui.putLine(" ");
}

int listLLDPNeighbors() {
  try {
    // Initialize LLDP client
    netd::shared::lldp::Client lldp_client;
    if (!lldp_client.initialize()) {
      std::cerr << "Failed to initialize LLDP client" << std::endl;
      return 1;
    }
    
    // Do immediate discovery (no background thread)
    if (!lldp_client.discoverOnce()) {
      std::cerr << "Failed to discover LLDP neighbors" << std::endl;
      return 1;
    }
    
    // Get discovered services
    auto services = lldp_client.getDiscoveredServices();
    
    if (services.empty()) {
      std::cout << "No LLDP neighbors found." << std::endl;
    } else {
      // Print table header
      std::cout << std::left;
      std::cout << std::setw(20) << "SERVICE" 
                << std::setw(12) << "TYPE" 
                << std::setw(20) << "HOSTNAME" 
                << std::setw(15) << "ADDRESS" 
                << std::setw(15) << "INTERFACE" << std::endl;
      std::cout << std::string(82, '-') << std::endl;
      
      // Print table rows
      for (const auto& service : services) {
        std::string type_str = (service.getServiceType() == netd::shared::lldp::ServiceType::NETD_SERVER ? "SERVER" : "CLIENT");
        std::string address_str = "";
        
        auto addresses = service.getAddressStrings();
        if (!addresses.empty()) {
          address_str = addresses[0]; // Show first address only
        }
        
        std::cout << std::setw(20) << service.getServiceName()
                  << std::setw(12) << type_str
                  << std::setw(20) << service.getHostname()
                  << std::setw(15) << address_str
                  << std::setw(15) << service.getInterfaceName() << std::endl;
      }
    }
    
    // Cleanup
    lldp_client.cleanup();
    
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Error listing LLDP neighbors: " << e.what() << std::endl;
    return 1;
  }
}

int main(int argc, char *argv[]) {
  // Default values
  std::string socketPath = "/tmp/netd.sock";
  int debugLevel = 0;  // 0=error only, 1=warning, 2=info, 3=debug
  bool listLLDP = false;
  
  // Parse command line options
  int opt;
  while ((opt = getopt(argc, argv, "s:Ldh")) != -1) {
    switch (opt) {
    case 's':
      socketPath = optarg;
      break;
    case 'L':
      listLLDP = true;
      break;
    case 'd':
      debugLevel++;
      break;
    case 'h':
      printUsage(argv[0]);
      return 0;
    default:
      printUsage(argv[0]);
      return 1;
    }
  }
  
  // Handle LLDP listing
  if (listLLDP) {
    return listLLDPNeighbors();
  }
  
  // Get logger instance and set log level
  auto &logger = netd::shared::Logger::getInstance();
  
  // Set log level based on debug flags
  switch (debugLevel) {
  case 0:
    logger.setLogLevel(netd::shared::LogLevel::INFO); // Default: INFO, WARNING, ERROR
    break;
  case 1:
    logger.setLogLevel(netd::shared::LogLevel::DEBUG); // -d: DEBUG and above
    break;
  case 2:
    logger.setLogLevel(netd::shared::LogLevel::TRACE); // -dd: TRACE and above
    break;
  case 3:
    logger.setLogLevel(netd::shared::LogLevel::TRACE); // -ddd: TRACE and above + timestamps
    break;
  default:
    logger.setLogLevel(netd::shared::LogLevel::TRACE); // More than 3: same as -ddd
    break;
  }
  
  // Initialize TUI
  netd::client::tui::TUI tui;
  if (!tui.initialize()) {
    std::cerr << "Failed to initialize TUI" << std::endl;
    return 1;
  }
  
  // Set up logger integration AFTER TUI is initialized
  tui.setLoggerInstance(&tui);
  tui.setDebugLevel(debugLevel);
  
  // Show startup information
  showStartupInfo(tui);
  
  // Get netconf client singleton
  auto& client = netd::client::netconf::NetconfClient::getInstance();
  
  // Try to connect in background thread
  std::thread connect_thread([&client, &tui, socketPath]() {
    try {
      client.connect(socketPath);
      tui.setConnectionStatus("Connected to " + socketPath);
    } catch (const std::exception& e) {
      tui.setConnectionStatus("Disconnected: " + std::string(e.what()));
    }
  });
  connect_thread.detach();
  
  // Show initial status
  tui.setConnectionStatus("Connecting...");
  
  // Set up command processor
  netd::client::CommandProcessor processor(tui, client);
  tui.setCommandHandler([&processor](const std::string &command) -> bool {
    return processor.processCommand(command);
  });
  
  tui.runInteractive();

  return 0;
}
