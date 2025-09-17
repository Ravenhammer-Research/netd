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
#include <exception>
#include <getopt.h>
#include <iostream>
#include <libyang/libyang.h>
#include <server/include/netconf/server.hpp>
#include <server/include/signal.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/yang.hpp>
#include <string>
#include <thread>
#include <unistd.h>

#define SPACE(n) std::string(n, ' ')

constexpr const char ANSI_CLEAR_SCREEN[] = {0x1b, 0x5b, 0x32, 0x4a,
                                            0x1b, 0x5b, 0x48, 0x00};
constexpr const char ANSI_BOLD[] = {0x1b, 0x5b, 0x31, 0x6d, 0x00};
constexpr const char ANSI_RESET[] = {0x1b, 0x5b, 0x30, 0x6d, 0x00};
constexpr const char ANSI_ITALIC[] = {0x1b, 0x5b, 0x33, 0x6d, 0x00};
constexpr const char NEWLINE[] = {0x0a, 0x00};

void global_exception_handler() {
  std::cout << ANSI_CLEAR_SCREEN << std::flush;

  std::cerr << "FATAL: Uncaught exception in netd server" << std::endl;
  std::cerr << "This might explain unexpected server crashes" << std::endl;

  try {
    std::rethrow_exception(std::current_exception());
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception type" << std::endl;
  }

  std::abort();
}

void printUsage(const char *progname) {
  std::cerr << ANSI_BOLD << "Usage:" << ANSI_RESET << SPACE(1) << progname
            << SPACE(1) << "[" << ANSI_ITALIC << "options" << ANSI_RESET << "]"
            << NEWLINE << NEWLINE;

  std::cerr << ANSI_BOLD << "Transport Options" << ANSI_RESET
            << " (can be specified multiple times):" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "--unix" << ANSI_RESET << SPACE(2)
            << "[" << ANSI_ITALIC << "path" << ANSI_RESET << "]" << SPACE(14)
            << "Unix domain socket" << NEWLINE;

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
  std::cerr << SPACE(2) << ANSI_BOLD << "-l" << ANSI_RESET << SPACE(26)
            << "List available YANG modules and exit" << NEWLINE;

  std::cerr << SPACE(2) << ANSI_BOLD << "-h" << ANSI_RESET << SPACE(26)
            << "Show this help message" << NEWLINE;
}

int main(int argc, char *argv[]) {
  std::set_terminate(global_exception_handler);

  auto &logger = netd::shared::Logger::getInstance();

  netd::shared::TransportType transportType = netd::shared::TransportType::UNIX;
  std::string bindAddress = "/tmp/netd.sock";
  int port = 19818;
  uint32_t logMask = netd::shared::LOG_DEFAULT;

  int opt;
  bool listModules = false;

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
  while ((opt = getopt_long(argc, argv, "dqlh", long_options, &option_index)) !=
         -1) {
    switch (opt) {
    case 1000:
      transportType = netd::shared::TransportType::UNIX;
      if (optarg)
        bindAddress = optarg;
      break;
    case 1003:
      transportType = netd::shared::TransportType::SCTPS;
      port = 19819;
      if (optarg)
        bindAddress = optarg;
      else
        bindAddress = "::";
      break;
    case 1004:
      transportType = netd::shared::TransportType::HTTPS;
      port = 19819;
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

  logger.setLogMask(logMask);

  if (logMask & (static_cast<uint32_t>(netd::shared::LogMask::DEBUG) |
                 static_cast<uint32_t>(netd::shared::LogMask::DEBUG_LLDP) |
                 static_cast<uint32_t>(netd::shared::LogMask::DEBUG_YANG) |
                 static_cast<uint32_t>(netd::shared::LogMask::DEBUG_TRACE))) {
    logger.setTimestampEnabled(true);
  }

  try {
    netd::shared::Yang::getInstance();
    if (!listModules) {
      logger.info("YANG manager initialized successfully");
    }
  } catch (const std::exception &e) {
    logger.error("Failed to initialize YANG manager: " + std::string(e.what()));
    return 1;
  }
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

  netd::server::setupSignalHandlers();

  netd::server::netconf::NetconfServer server(transportType, bindAddress, port);
  try {
    server.start();
  } catch (const std::exception &e) {
    std::cerr << "Failed to start server: " << e.what() << std::endl;
    netd::server::cleanupSignalHandlers();
    return 1;
  }

  server.run();

  server.stop();
  netd::server::cleanupSignalHandlers();

  return 0;
}
