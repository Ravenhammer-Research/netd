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
#include <exception>
#include <getopt.h>
#include <iostream>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/request/get/library.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#define SPACE(n) std::string(n, ' ')

constexpr const char ANSI_CLEAR_SCREEN[] = {0x1b, 0x5b, 0x32, 0x4a,
                                            0x1b, 0x5b, 0x48, 0x00};
constexpr const char ANSI_BOLD[] = {0x1b, 0x5b, 0x31, 0x6d, 0x00};
constexpr const char ANSI_RESET[] = {0x1b, 0x5b, 0x30, 0x6d, 0x00};
constexpr const char ANSI_ITALIC[] = {0x1b, 0x5b, 0x33, 0x6d, 0x00};
constexpr const char NEWLINE[] = {0x0a, 0x00};

void global_exception_handler() {
  std::cout << ANSI_CLEAR_SCREEN << std::flush;

  std::exception_ptr current_exception = std::current_exception();
  if (current_exception) {
    std::cerr << "FATAL: Uncaught exception in netc" << std::endl;
    std::cerr << "This might explain why netc -d exits unexpectedly" << std::endl;
    std::cerr << "Exception type check not possible without try-catch" << std::endl;
  } else {
    std::cerr << "FATAL: Uncaught exception in netc" << std::endl;
    std::cerr << "This might explain why netc -d exits unexpectedly" << std::endl;
  }
}

void printUsage(const char *progname) {
  std::cerr << ANSI_BOLD << "Usage:" << ANSI_RESET << SPACE(1) << progname
            << SPACE(1) << "[" << ANSI_ITALIC << "options" << ANSI_RESET << "]"
            << NEWLINE << NEWLINE;
            
  std::cerr << ANSI_BOLD << "Transport Options" << ANSI_RESET << ":" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "--unix" << ANSI_RESET << SPACE(2)
            << "[" << ANSI_ITALIC << "path" << ANSI_RESET << "]" << SPACE(14)
            << "Unix domain socket (default: /tmp/netd.sock)" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "--sctps" << ANSI_RESET << SPACE(1)
            << "[" << ANSI_ITALIC << "addr" << ANSI_RESET << "]:["
            << ANSI_ITALIC << "port" << ANSI_RESET << "]" << SPACE(7)
            << "SCTP with DTLS" << SPACE(1) << ANSI_ITALIC
            << "(not implemented)" << ANSI_RESET << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "--https" << ANSI_RESET << SPACE(1)
            << "[" << ANSI_ITALIC << "addr" << ANSI_RESET << "]:["
            << ANSI_ITALIC << "port" << ANSI_RESET << "]" << SPACE(7)
            << "HTTP with TLS " << ANSI_ITALIC << SPACE(1)
            << "(not implemented)" << ANSI_RESET << NEWLINE << NEWLINE;

  std::cerr << ANSI_BOLD << "Debug Options" << ANSI_RESET << ":" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "-d" << ANSI_RESET << SPACE(26)
            << "Basic debug output" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "-dd" << ANSI_RESET << SPACE(25)
            << "Basic debug + trace output" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "-q" << ANSI_RESET << SPACE(26)
            << "Quiet mode (errors only)" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "--debug" << ANSI_RESET << SPACE(21)
            << "Basic debug output" << NEWLINE;

#ifdef HAVE_LLDP
  std::cerr << SPACE(2) << ANSI_BOLD << "--debug-lldp" << ANSI_RESET
            << SPACE(16) << "LLDP debug output" << NEWLINE;
#endif
  std::cerr << SPACE(2) << ANSI_BOLD << "--debug-yang" << ANSI_RESET
            << SPACE(16) << "YANG debug output" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "--debug-yang-dict" << ANSI_RESET
            << SPACE(11) << "YANG dictionary debug" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "--debug-yang-xpath" << ANSI_RESET
            << SPACE(10) << "YANG XPath debug" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "--debug-yang-depsets" << ANSI_RESET
            << SPACE(8) << "YANG dependency sets debug" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "--debug-trace" << ANSI_RESET
            << SPACE(15) << "Application trace debug" << NEWLINE << NEWLINE;

  std::cerr << ANSI_BOLD << "Other Options" << ANSI_RESET << ":" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "-L" << ANSI_RESET << SPACE(26)
            << "List LLDP neighbors and exit" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "-h" << ANSI_RESET << SPACE(26)
            << "Show this help message" << NEWLINE;
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

void testdebug() {
  auto &logger = netd::shared::Logger::getInstance();
  uint32_t logMask = netd::shared::LOG_DEFAULT;
  logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG);
  logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_TRACE);
  logger.setLogMask(logMask);
  logger.setTimestampEnabled(true);

  auto client = std::make_unique<netd::client::netconf::NetconfClient>(
      netd::shared::TransportType::UNIX, "/tmp/netd.sock");
  client->connect();
  client->disconnect(true);
}

int main(int argc, char *argv[]) {
  std::set_terminate(global_exception_handler);

  auto &logger = netd::shared::Logger::getInstance();

  netd::shared::TransportType transportType = netd::shared::TransportType::UNIX;

  testdebug();
  return 0;

  std::string bindAddress = "/tmp/netd.sock";
  uint32_t logMask = netd::shared::LOG_DEFAULT;
  bool listLLDP = false;

  int opt;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-dd") == 0) {
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG);
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_TRACE);
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
    case 1000:
      transportType = netd::shared::TransportType::UNIX;
      if (optarg)
        bindAddress = optarg;
      break;
    case 1003:
      transportType = netd::shared::TransportType::SCTPS;
      if (optarg)
        bindAddress = optarg;
      else
        bindAddress = "::";
      break;
    case 1004:
      transportType = netd::shared::TransportType::HTTPS;
      if (optarg)
        bindAddress = optarg;
      else
        bindAddress = "::";
      break;
    case 'd':
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG);
      break;
    case 'q':
      logMask = static_cast<uint32_t>(netd::shared::LogMask::ERROR);
      break;
    case 2000:
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG);
      break;
#ifdef HAVE_LLDP
    case 2001:
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_LLDP);
      break;
#endif
    case 2002:
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_YANG);
      break;
    case 2003:
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_YANG_DICT);
      break;
    case 2004:
      logMask |= static_cast<uint32_t>(netd::shared::LogMask::DEBUG_YANG_XPATH);
      break;
    case 2005:
      logMask |=
          static_cast<uint32_t>(netd::shared::LogMask::DEBUG_YANG_DEPSETS);
      break;
    case 2006:
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

  logger.setLogMask(logMask);

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
    client->connect();
    tui.setConnectionStatus("Server available at " + bindAddress);
    client->disconnect(false);

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
