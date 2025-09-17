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
#include <client/include/lldp.hpp>
#include <client/include/netconf/client.hpp>
#include <client/include/processor/parser.hpp>
#include <client/include/tui.hpp>
#include <getopt.h>
#include <iostream>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

void printUsage(const char *progname) {
  std::cerr << "\033[1mUsage:\033[0m " << progname
            << " [\033[3moptions\033[0m]\n\n";
  std::cerr << "\033[1mTransport Options\033[0m:\n";
  std::cerr << "  \033[1m--unix\033[0m  [\033[3mpath\033[0m]             Unix "
               "domain socket (default: /tmp/netd.sock)\n";
  std::cerr
      << "  \033[1m--sctps\033[0m [\033[3maddr\033[0m]:[\033[3mport\033[0m]    "
         "  SCTP with DTLS \033[3m(not implemented)\033[0m\n";
  std::cerr
      << "  \033[1m--https\033[0m [\033[3maddr\033[0m]:[\033[3mport\033[0m]    "
         "  HTTP with TLS \033[3m (not implemented)\033[0m\n\n";
  std::cerr << "\033[1mDebug Options\033[0m:\n";
  std::cerr
      << "  \033[1m-d\033[0m                         Basic debug output\n";
  std::cerr << "  \033[1m-dd\033[0m                        Basic debug + trace "
               "output\n";
  std::cerr << "  \033[1m-q\033[0m                         Quiet mode (errors "
               "only)\n";
  std::cerr
      << "  \033[1m--debug\033[0m                    Basic debug output\n";
#ifdef HAVE_LLDP
  std::cerr << "  \033[1m--debug-lldp\033[0m               LLDP debug output\n";
#endif
  std::cerr << "  \033[1m--debug-yang\033[0m               YANG debug output\n";
  std::cerr
      << "  \033[1m--debug-yang-dict\033[0m          YANG dictionary debug\n";
  std::cerr << "  \033[1m--debug-yang-xpath\033[0m         YANG XPath debug\n";
  std::cerr << "  \033[1m--debug-yang-depsets\033[0m       YANG dependency "
               "sets debug\n";
  std::cerr << "  \033[1m--debug-trace\033[0m              Application trace "
               "debug\n\n";
  std::cerr << "\033[1mOther Options\033[0m:\n";
  std::cerr << "  \033[1m-L\033[0m                         List LLDP neighbors "
               "and exit\n";
  std::cerr
      << "  \033[1m-h\033[0m                         Show this help message\n";
}

void showStartupInfo(netd::client::tui::TUI &tui) {
  tui.putLine("NetD Client 1.0");
  tui.putLine(" ");
  tui.putLine("Copyright (c) 2025 RavenHammer Research. All rights reserved.");
  tui.putLine(" ");
  tui.putLine("Third-Party Licenses:");
  tui.putLine(" ");
  tui.putLine(
      "  FreeBSD - Copyright (c) The Regents of the University of California.");
  tui.putLine("           All rights reserved. BSD License.");
  tui.putLine(
      "  libyang - Copyright (c) 2015-2025, CESNET. All rights reserved.");
  tui.putLine("           BSD License.");
  tui.putLine("  lldpd   - Copyright (c) 2008-2017, Vincent Bernat "
              "<vincent@bernat.im>");
  tui.putLine("           ISC License. See LICENSE.txt from your application");
  tui.putLine("           distribution for details.");
  tui.putLine(" ");
}

int main(int argc, char *argv[]) {
  auto &logger = netd::shared::Logger::getInstance();

  // Default values
  netd::shared::TransportType transportType = netd::shared::TransportType::UNIX;
  std::string bindAddress = "/tmp/netd.sock";
  uint32_t logMask = netd::shared::LOG_DEFAULT;
  bool listLLDP = false;

  // Parse command line options
  int opt;

  // Handle -dd before main parsing (getopt doesn't handle -dd naturally)
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-dd") == 0) {
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG);
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_TRACE);
      // Replace -dd with -d so getopt can handle it normally
      argv[i] = const_cast<char *>("-d");
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
      {0, 0, 0, 0}};

  int option_index = 0;
  while ((opt = getopt_long(argc, argv, "dqlLh", long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 1000: // --unix
      transportType = netd::shared::TransportType::UNIX;
      if (optarg)
        bindAddress = optarg;
      break;
    case 1003: // --sctps
      transportType = netd::shared::TransportType::SCTPS;
      if (optarg)
        bindAddress = optarg;
      else
        bindAddress = "::";
      break;
    case 1004: // --https
      transportType = netd::shared::TransportType::HTTPS;
      if (optarg)
        bindAddress = optarg;
      else
        bindAddress = "::";
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
      logMask |=
          static_cast<uint32_t>(netd::shared::LogMask::DEBUG_YANG_DEPSETS);
      break;
    case 2006: // --debug-trace
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_TRACE);
      break;
    case 'L':
      listLLDP = true;
      break;
    case 'h':
      printUsage(argv[0]);
      return 0;
    default:
      printUsage(argv[0]);
      return 1;
    }
  }

  if (listLLDP) {
    return listLLDPNeighbors();
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

  netd::client::tui::TUI tui;
  if (!tui.initialize()) {
    std::cerr << "Failed to initialize TUI" << std::endl;
    return 1;
  }

  tui.setLoggerInstance(&tui);

  showStartupInfo(tui);

  auto client = std::make_unique<netd::client::netconf::NetconfClient>(
      transportType, bindAddress);

  try {
    // Connect on startup to verify server availability
    client->connect();
    tui.setConnectionStatus("Server available at " + bindAddress);
    client->disconnect(false); // Disconnect transport but don't close session
                               // after verification

  } catch (const std::exception &e) {
    tui.setConnectionStatus("Server unavailable: " + std::string(e.what()));
  }

  tui.redrawScreen();

  netd::client::processor::CommandProcessor processor(tui, *client);

  tui.runInteractive([&processor](const std::string &command) -> bool {
    return processor.processCommand(command);
  });

  return 0;
}
