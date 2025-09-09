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

#include <client/include/netconf.hpp>
#include <client/include/parser.hpp>
#include <client/include/processor.hpp>
#include <client/include/table.hpp>
#include <client/include/tui.hpp>
#include <iostream>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <getopt.h>
#include <vector>

void printUsage(const char *progname) {
  std::cerr << "Usage: " << progname << " [options]\n";
  std::cerr << "Options:\n";
  std::cerr << "  -s <path>    Socket path (default: /tmp/netd.sock)\n";
  std::cerr << "  -d           Enable warning logging\n";
  std::cerr << "  -dd          Enable info logging\n";
  std::cerr << "  -ddd         Enable debug logging\n";
  std::cerr << "  -h           Show this help message\n";
}

void showStartupInfo(netd::client::TUI& tui) {
  tui.putLine("NetD Client 1.0");
  tui.putLine(" ");
  tui.putLine("Copyright (c) 2025 RavenHammer Research. All rights reserved.");
  tui.putLine(" ");
  tui.putLine("Credits:");
  tui.putLine(" ");
  tui.putLine("  FreeBSD - Copyright (c) The Regents of the University of California.");
  tui.putLine("           All rights reserved.");
  tui.putLine("  libnetconf2 - Copyright (c) 2015-2020, CESNET. All rights reserved.");
  tui.putLine("  libyang - Copyright (c) 2015-2025, CESNET. All rights reserved.");
  tui.putLine(" ");
}

int main(int argc, char *argv[]) {
  // Default values
  std::string socketPath = "/tmp/netd.sock";
  int debugLevel = 0;  // 0=error only, 1=warning, 2=info, 3=debug
  
  // Parse command line options
  int opt;
  while ((opt = getopt(argc, argv, "s:dh")) != -1) {
    switch (opt) {
    case 's':
      socketPath = optarg;
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
  
  // Get logger instance and set log level
  auto &logger = netd::shared::Logger::getInstance();
  
  // Set log level based on debug flags
  switch (debugLevel) {
  case 0:
    logger.setLogLevel(netd::shared::LogLevel::ERROR);
    break;
  case 1:
    logger.setLogLevel(netd::shared::LogLevel::WARNING);
    break;
  case 2:
    logger.setLogLevel(netd::shared::LogLevel::INFO);
    break;
  case 3:
    logger.setLogLevel(netd::shared::LogLevel::DEBUG);
    break;
  default:
    logger.setLogLevel(netd::shared::LogLevel::TRACE);
    break;
  }
  
  // Initialize TUI
  netd::client::TUI tui;
  if (!tui.initialize()) {
    std::cerr << "Failed to initialize TUI" << std::endl;
    return 1;
  }
  
  // Set up logger integration AFTER TUI is initialized
  tui.setLoggerInstance(&tui);
  tui.setDebugLevel(debugLevel);
  
  // Show startup information
  showStartupInfo(tui);
  
  // Start netconf client
  netd::client::NetconfClient client;
  if (client.connect(socketPath)) {
    tui.setConnectionStatus("Connected to " + socketPath);
  } else {
    tui.setConnectionStatus("Disconnected");
  }
  
  // Set up command processor
  netd::client::CommandProcessor processor(tui);
  tui.setCommandHandler([&processor](const std::string &command) -> bool {
    return processor.processCommand(command);
  });
  
  tui.runInteractive();

  return 0;
}
