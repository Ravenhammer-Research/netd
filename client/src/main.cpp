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
#include <client/include/table.hpp>
#include <client/include/terminal.hpp>
#include <iostream>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace netd::client {

  class CommandProcessor {
  public:
    CommandProcessor(Terminal &terminal) : terminal_(terminal), parser_() {
      setupCompletions();
    }

    bool processCommand(const std::string &command) {
      if (command.empty()) {
        return true;
      }

      ParsedCommand parsed = parser_.parse(command);

      if (parsed.command == CommandType::UNKNOWN) {
        terminal_.writeLine("Unknown command: " + command);
        return false;
      }

      switch (parsed.command) {
      case CommandType::SHOW:
        return handleShowCommand(parsed);
      case CommandType::SET:
        return handleSetCommand(parsed);
      case CommandType::DELETE:
        return handleDeleteCommand(parsed);
      case CommandType::COMMIT:
        return handleCommitCommand(parsed);
      default:
        terminal_.writeLine("Unknown command type");
        return false;
      }
    }

  private:
    Terminal &terminal_;
    CommandParser parser_;

    void setupCompletions() {
      std::vector<std::string> completions = {
          "show",    "set",      "delete",    "commit",    "help",    "quit",
          "exit",    "vrf",      "interface", "protocol",  "static",  "host",
          "network", "gateway",  "iface",     "address",   "mtu",     "group",
          "member",  "laggport", "vlandev",   "vlanproto", "vxlanid", "tunnel",
          "local",   "remote",   "type",      "name",      "id",      "inet",
          "inet6"};
      terminal_.setCompletions(completions);
    }

    bool handleShowCommand(const ParsedCommand &parsed) {
      if (parsed.target == TargetType::UNKNOWN) {
        terminal_.writeLine("Usage: show <vrf|interface> [options]");
        return false;
      }

      switch (parsed.target) {
      case TargetType::VRF:
        return handleShowVrf(parsed);
      case TargetType::INTERFACE:
        return handleShowInterface(parsed);
      case TargetType::ROUTE:
        return handleShowRoute(parsed);
      default:
        terminal_.writeLine("Unknown show target");
        return false;
      }
    }

    bool handleShowVrf([[maybe_unused]] const ParsedCommand &parsed) {
      // TODO: Implement VRF display
      terminal_.writeLine("VRF information:");
      terminal_.writeLine("  FIB 0: default");
      return true;
    }

    bool handleShowRoute([[maybe_unused]] const ParsedCommand &parsed) {
      // TODO: Implement route display
      terminal_.writeLine("Route information:");
      terminal_.writeLine("  No routes configured");
      return true;
    }

    bool handleShowInterface([[maybe_unused]] const ParsedCommand &parsed) {
      try {
        // Send get-config request with interface filter
        // This should generate: <get-config><source><running/></source><filter
        // type="subtree"><interfaces/></filter></get-config>

        throw netd::shared::NotImplementedError(
            "handleShowInterface not implemented");

        // TODO: Parse response data to shared Interface types
        // For now, create a sample table
        netd::client::Table table;
        table.addColumn("Interface");
        table.addColumn("Type");
        table.addColumn("Status");
        table.addColumn("MTU");

        table.addRow({"lo0", "loopback", "UP", "16384"});
        table.addRow({"em0", "ethernet", "UP", "1500"});
        table.addRow({"bridge0", "bridge", "DOWN", "1500"});

        terminal_.writeLine(table.format());
        return true;
      } catch (const std::exception &e) {
        terminal_.writeLine("Error getting interface data: " +
                            std::string(e.what()));
        return false;
      }
    }

    bool handleSetCommand(const ParsedCommand &parsed) {
      if (parsed.target == TargetType::UNKNOWN) {
        terminal_.writeLine("Usage: set <vrf|interface> [options]");
        return false;
      }

      switch (parsed.target) {
      case TargetType::VRF:
        return handleSetVrf(parsed);
      case TargetType::INTERFACE:
        return handleSetInterface(parsed);
      case TargetType::ROUTE:
        return handleSetRoute(parsed);
      default:
        terminal_.writeLine("Unknown set target");
        return false;
      }
    }

    bool handleSetVrf([[maybe_unused]] const ParsedCommand &parsed) {
      // TODO: Implement VRF configuration
      terminal_.writeLine("VRF configuration not yet implemented");
      return true;
    }

    bool handleSetInterface([[maybe_unused]] const ParsedCommand &parsed) {
      // TODO: Implement interface configuration
      terminal_.writeLine("Interface configuration not yet implemented");
      return true;
    }

    bool handleSetRoute([[maybe_unused]] const ParsedCommand &parsed) {
      // TODO: Implement route configuration
      terminal_.writeLine("Route configuration not yet implemented");
      return true;
    }

    bool handleDeleteCommand([[maybe_unused]] const ParsedCommand &parsed) {
      // TODO: Implement delete operations
      terminal_.writeLine("Delete operations not yet implemented");
      return true;
    }

    bool handleCommitCommand([[maybe_unused]] const ParsedCommand &parsed) {
      try {
        terminal_.writeLine("Commit not implemented yet");
        return true;
      } catch (const std::exception &e) {
        terminal_.writeLine("Error during commit: " + std::string(e.what()));
        return false;
      }
    }
  };

} // namespace netd::client

int main() {
  auto &logger = netd::shared::Logger::getInstance();

  // Initialize terminal first
  netd::client::Terminal terminal;
  if (!terminal.initialize()) {
    std::cerr << "Failed to initialize terminal" << std::endl;
    return 1;
  }

  // Set up logger callback to use terminal for output
  logger.setCallback(
      [&terminal](netd::shared::LogLevel level, const std::string &message) {
        std::string prefix;
        switch (level) {
        case netd::shared::LogLevel::DEBUG:
          prefix = "[DEBUG] ";
          break;
        case netd::shared::LogLevel::INFO:
          prefix = "[INFO] ";
          break;
        case netd::shared::LogLevel::WARNING:
          prefix = "[WARN] ";
          break;
        case netd::shared::LogLevel::ERROR:
          prefix = "[ERROR] ";
          break;
        }
        terminal.writeLine(prefix + message);
        // Redraw prompt after logging
        terminal.redrawPrompt();
      });

  // Run interactive mode on separate thread
  std::thread interactiveThread([&terminal]() { terminal.runInteractive(); });

  // Set up command processor
  netd::client::CommandProcessor processor(terminal);
  terminal.setCommandHandler([&processor](const std::string &command) {
    return processor.processCommand(command);
  });

  // Connect to server
  std::string socketPath = "/tmp/netd.sock";
  if (!netd::client::connectToServer(socketPath)) {
    logger.error("Failed to connect to NETD server");
    logger.error("Make sure the server is running with: ./netd");
    terminal.cleanup();
    goto wait;
  }

  logger.info("Connected to NetD via UNIX socket");

  // Display welcome message
  logger.info("NETD CLI - Network Configuration Tool");
  logger.info("Type 'help' for available commands or 'quit' to exit.");

wait:
  // Block until interactive mode completes
  interactiveThread.join();
  // Cleanup
  netd::client::disconnectFromServer();
  terminal.cleanup();
  logger.info("NETD Client finished");
  return 0;
}
