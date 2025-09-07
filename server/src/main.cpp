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
#include <csignal>
#include <iostream>
#include <server/include/netconf/server.hpp>
#include <shared/include/logger.hpp>
#include <thread>
#include <unistd.h>
#include <getopt.h>
#include <string>

// Global flag for graceful shutdown
static volatile bool g_running = true;

void signalHandler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    g_running = false;
  }
}

void printUsage(const char *progname) {
  std::cerr << "Usage: " << progname << " [options]\n";
  std::cerr << "Options:\n";
  std::cerr << "  -s <path>    Socket path (default: /tmp/netd.sock)\n";
  std::cerr << "  -d           Enable warning logging\n";
  std::cerr << "  -dd          Enable info logging\n";
  std::cerr << "  -ddd         Enable debug logging\n";
  std::cerr << "  -h           Show this help message\n";
}

int main(int argc, char *argv[]) {
  auto &logger = netd::shared::Logger::getInstance();
  
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

  // Set up signal handlers for graceful shutdown
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  // Start NETCONF server
  if (!netd::server::netconf::startNetconfServer(socketPath)) {
    return 1;
  }
  // Run the server in a separate thread so we can handle signals
  std::thread serverThread([]() { netd::server::netconf::runNetconfServer(); });

  // Main loop - just wait for shutdown signal
  while (g_running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Stop the server and wait for thread to finish
  netd::server::netconf::stopNetconfServer();
  serverThread.join();

  // Graceful shutdown
  netd::server::netconf::stopNetconfServer();

  return 0;
}
