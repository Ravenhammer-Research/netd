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
  std::cerr << "\033[1mUsage:\033[0m " << progname << " [\033[3moptions\033[0m]\n\n";
  std::cerr << "\033[1mTransport Options\033[0m (can be specified multiple times):\n";
  std::cerr << "  \033[1m--unix\033[0m  [\033[3mpath\033[0m]             Unix domain socket\n";
  std::cerr << "  \033[1m--sctps\033[0m [\033[3maddr\033[0m]:[\033[3mport\033[0m]      SCTP with DTLS \033[3m(not implemented)\033[0m\n";
  std::cerr << "  \033[1m--https\033[0m [\033[3maddr\033[0m]:[\033[3mport\033[0m]      HTTP with TLS \033[3m (not implemented)\033[0m\n\n";
  std::cerr << "\033[1mDebug Options\033[0m:\n";
  std::cerr << "  \033[1m-d\033[0m                         Basic debug output\n";
  std::cerr << "  \033[1m-dd\033[0m                        Basic debug + trace output\n";
  std::cerr << "  \033[1m-q\033[0m                         Quiet mode (errors only)\n";
  std::cerr << "  \033[1m--debug\033[0m                    Basic debug output\n";
#ifdef HAVE_LLDP
  std::cerr << "  \033[1m--debug-lldp\033[0m               LLDP debug output\n";
#endif
  std::cerr << "  \033[1m--debug-yang\033[0m               YANG debug output\n";
  std::cerr << "  \033[1m--debug-yang-dict\033[0m          YANG dictionary debug\n";
  std::cerr << "  \033[1m--debug-yang-xpath\033[0m         YANG XPath debug\n";
  std::cerr << "  \033[1m--debug-yang-depsets\033[0m       YANG dependency sets debug\n";
  std::cerr << "  \033[1m--debug-trace\033[0m              Application trace debug\n\n";
  std::cerr << "\033[1mOther Options\033[0m:\n";
  std::cerr << "  \033[1m-l\033[0m                         List available YANG modules and exit\n";
  std::cerr << "  \033[1m-h\033[0m                         Show this help message\n";
}

// TransportType is now defined in shared/include/transport.hpp

int main(int argc, char *argv[]) {
  auto &logger = netd::shared::Logger::getInstance();
  
  // Default values
  netd::shared::TransportType transportType = netd::shared::TransportType::UNIX;
  std::string bindAddress = "/tmp/netd.sock";
  int port = 19818;
  uint32_t logMask = netd::shared::LOG_DEFAULT;
  
  // Parse command line options
  int opt;
  bool listModules = false;
  
  // Handle -dd before main parsing (getopt doesn't handle -dd naturally)
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-dd") == 0) {
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG);
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_TRACE);
      // Replace -dd with -d so getopt can handle it normally
      argv[i] = const_cast<char*>("-d");
    }
  }
  static struct option long_options[] = {
    {"unix", optional_argument, 0, 1000},
    {"sctps", optional_argument, 0, 1003},
    {"https", optional_argument, 0, 1004},
    {"debug", no_argument, 0, 2000},
#ifdef HAVE_LLDP
    {"debug-lldp", no_argument, 0, 2001},
#endif
    {"debug-yang", no_argument, 0, 2002},
    {"debug-yang-dict", no_argument, 0, 2003},
    {"debug-yang-xpath", no_argument, 0, 2004},
                {"debug-yang-depsets", no_argument, 0, 2005},
                {"debug-trace", no_argument, 0, 2006},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };
  
  int option_index = 0;
  while ((opt = getopt_long(argc, argv, "dqlh", long_options, &option_index)) != -1) {
    switch (opt) {
    case 1000: // --unix
      transportType = netd::shared::TransportType::UNIX;
      if (optarg) bindAddress = optarg;
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
      // Legacy debug flag - add basic debug
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG);
      break;
    case 'q':
      // Quiet mode - only error messages
      logMask = static_cast<uint32_t>(netd::shared::LogMask::ERROR);
      break;
    case 2000: // --debug
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG);
      break;
#ifdef HAVE_LLDP
    case 2001: // --debug-lldp
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_LLDP);
      break;
#endif
    case 2002: // --debug-yang
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_YANG);
      break;
    case 2003: // --debug-yang-dict
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_YANG_DICT);
      break;
    case 2004: // --debug-yang-xpath
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_YANG_XPATH);
      break;
                case 2005: // --debug-yang-depsets
                  logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_YANG_DEPSETS);
                  break;
                case 2006: // --debug-trace
                  logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_TRACE);
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
  
  // Set log mask based on command line options
  logger.setLogMask(logMask);
  
  // Enable timestamps if any debug is enabled
  if (logMask & (static_cast<uint32_t>(netd::shared::LogMask::DEBUG) | 
                 static_cast<uint32_t>(netd::shared::LogMask::DEBUG_LLDP) | 
                 static_cast<uint32_t>(netd::shared::LogMask::DEBUG_YANG) |
                 static_cast<uint32_t>(netd::shared::LogMask::DEBUG_TRACE))) {
    logger.setTimestampEnabled(true);
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
