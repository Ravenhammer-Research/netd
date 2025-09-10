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
  std::cerr << "Options:\n";
  std::cerr << "  -s <path>    Socket path (default: /tmp/netd.sock)\n";
  std::cerr << "  -d           DEBUG\n";
  std::cerr << "  -dd          DEBUG + TRACE\n";
  std::cerr << "  -ddd         DEBUG + TRACE + TIMESTAMP\n";
  std::cerr << "  -dddd        DEBUG + TRACE + TIMESTAMP + YANG\n";
  std::cerr << "  -l           List available YANG modules and exit\n";
  std::cerr << "  -h           Show this help message\n";
}

int main(int argc, char *argv[]) {
  auto &logger = netd::shared::Logger::getInstance();
  
  // Default values
  std::string socketPath = "/tmp/netd.sock";
  int debugLevel = 0;  // 0=error only, 1=warning, 2=info, 3=debug
  
  // Parse command line options
  int opt;
  bool listModules = false;
  while ((opt = getopt(argc, argv, "s:dlh")) != -1) {
    switch (opt) {
    case 's':
      socketPath = optarg;
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
  netd::server::netconf::NetconfServer server(socketPath);
  if (!server.start()) {
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
