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

#include <chrono>
#include <iostream>
#include <server/include/netconf/server.hpp>
#include <server/include/signal.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/yang.hpp>
#include <libyang/libyang.h>
#include <thread>
#include <unistd.h>
#include <getopt.h>
#include <string>

void printUsage(const char *progname) {
  std::cerr << "Usage: " << progname << " [options]\n";
  std::cerr << "Transport Options (choose one):\n";
  std::cerr << "  --unix [path]     Unix domain socket (default: /tmp/netd.sock)\n";
  std::cerr << "  --sctp [addr]     SCTP transport (default: bind all, port 19818)\n";
  std::cerr << "  --http [addr]     HTTP transport (default: bind all, port 19818)\n";
  std::cerr << "  --sctps [addr]    SCTP with DTLS (default: bind all, port 19819)\n";
  std::cerr << "  --https [addr]    HTTP with TLS (default: bind all, port 19819)\n";
  std::cerr << "Other Options:\n";
  std::cerr << "  -d               DEBUG\n";
  std::cerr << "  -dd              DEBUG + TRACE\n";
  std::cerr << "  -ddd             DEBUG + TRACE + TIMESTAMP\n";
  std::cerr << "  -dddd            DEBUG + TRACE + TIMESTAMP + YANG\n";
  std::cerr << "  -l               List available YANG modules and exit\n";
  std::cerr << "  -h               Show this help message\n";
}

// TransportType is now defined in shared/include/transport.hpp

int main(int argc, char *argv[]) {
  auto &logger = netd::shared::Logger::getInstance();
  
  // Default values
  netd::shared::TransportType transportType = netd::shared::TransportType::UNIX;
  std::string bindAddress = "/tmp/netd.sock";
  int port = 19818;
  int debugLevel = 0;
  
  // Parse command line options
  int opt;
  bool listModules = false;
  static struct option long_options[] = {
    {"unix", optional_argument, 0, 1000},
    {"sctp", optional_argument, 0, 1001},
    {"http", optional_argument, 0, 1002},
    {"sctps", optional_argument, 0, 1003},
    {"https", optional_argument, 0, 1004},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };
  
  int option_index = 0;
  while ((opt = getopt_long(argc, argv, "dlh", long_options, &option_index)) != -1) {
    switch (opt) {
    case 1000: // --unix
      transportType = netd::shared::TransportType::UNIX;
      if (optarg) bindAddress = optarg;
      break;
    case 1001: // --sctp
      transportType = netd::shared::TransportType::SCTP;
      port = 19818;
      if (optarg) bindAddress = optarg; else bindAddress = "::";
      break;
    case 1002: // --http
      transportType = netd::shared::TransportType::HTTP;
      port = 19818;
      if (optarg) bindAddress = optarg; else bindAddress = "::";
      break;
    case 1003: // --sctps
      transportType = netd::shared::TransportType::SCTPS;
      port = 19819;
      if (optarg) bindAddress = optarg; else bindAddress = "::";
      break;
    case 1004: // --https
      transportType = netd::shared::TransportType::HTTPS;
      port = 19819;
      if (optarg) bindAddress = optarg; else bindAddress = "::";
      break;
    case 'd':
      debugLevel++;
      break;
    case 'l':
      listModules = true;
      break;
    case 'h':
      printUsage(argv[0]);
      return 0;
    default:
      printUsage(argv[0]);
      return 1;
    }
  }
  
  // Set log level based on debug flags
  switch (debugLevel) {
  case 0:
    logger.setLogLevel(netd::shared::LogLevel::ERROR);
    ly_log_level(LY_LLERR);
    break;
  case 1:
    logger.setLogLevel(netd::shared::LogLevel::DEBUG);
    ly_log_level(LY_LLERR);
    break;
  case 2:
    logger.setLogLevel(netd::shared::LogLevel::TRACE);
    ly_log_level(LY_LLERR);
    break;
  case 3:
    logger.setLogLevel(netd::shared::LogLevel::TRACE);
    ly_log_level(LY_LLERR);
    logger.setTimestampEnabled(true);
    break;
  case 4:
    logger.setLogLevel(netd::shared::LogLevel::YANG);
    ly_log_level(LY_LLDBG);
    logger.setTimestampEnabled(true);
    logger.setYangDebugGroups(LY_LDGDICT | LY_LDGXPATH | LY_LDGDEPSETS);
    break;
  default:
    logger.setLogLevel(netd::shared::LogLevel::YANG);
    ly_log_level(LY_LLDBG);
    logger.setTimestampEnabled(true);
    logger.setYangDebugGroups(LY_LDGDICT | LY_LDGXPATH | LY_LDGDEPSETS);
    break;
  }

  // Initialize YANG manager
  try {
    netd::shared::Yang::getInstance();
    if (!listModules) {
      logger.info("YANG manager initialized successfully");
    }
  } catch (const std::exception &e) {
    logger.error("Failed to initialize YANG manager: " + std::string(e.what()));
    return 1;
  }

  // Handle list modules option
  if (listModules) {
    auto &yang = netd::shared::Yang::getInstance();
    ly_ctx *ctx = yang.getContext();
    
    if (!ctx) {
      std::cerr << "Error: YANG context not initialized\n";
      return 1;
    }
    
    std::cout << "Available YANG modules:\n";
    std::cout << "======================\n\n";
    
    uint32_t index = 0;
    const struct lys_module *iter = nullptr;
    int count = 0;
    
    while ((iter = ly_ctx_get_module_iter(ctx, &index)) != nullptr) {
      count++;
      std::cout << "Module: " << iter->name;
      if (iter->revision) {
        std::cout << "@" << iter->revision;
      }
      std::cout << "\n";
      
      if (iter->dsc) {
        std::cout << "  Description: " << iter->dsc << "\n";
      }
      
      if (iter->org) {
        std::cout << "  Organization: " << iter->org << "\n";
      }
      
      if (iter->contact) {
        std::cout << "  Contact: " << iter->contact << "\n";
      }
      
      std::cout << "\n";
    }
    
    std::cout << "Total modules loaded: " << count << "\n";
    return 0;
  }

  // Set up signal handlers for graceful shutdown
  netd::server::setupSignalHandlers();

  // Create and start NETCONF server
  netd::server::netconf::NetconfServer server(transportType, bindAddress, port);
  try {
    server.start();
  } catch (const std::exception& e) {
    std::cerr << "Failed to start server: " << e.what() << std::endl;
    netd::server::cleanupSignalHandlers();
    return 1;
  }

  // Run the server (this will accept connections and process them)
  server.run();

  // Graceful shutdown
  server.stop();
  netd::server::cleanupSignalHandlers();

  return 0;
}
