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

#include <client/include/processor.hpp>
#include <client/include/netconf/client.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/response/get/config.hpp>
#include <shared/include/yang.hpp>
#include <client/include/tui.hpp>

namespace netd::client {

  CommandProcessor::CommandProcessor(netd::client::tui::TUI &tui, netd::client::netconf::NetconfClient &client) : tui_(tui), client_(client), parser_() {
  }

  bool CommandProcessor::processCommand(const std::string &command) {
    if (command.empty()) {
      return true;
    }

    ParsedCommand parsed = parser_.parse(command);

    if (parsed.command == CommandType::UNKNOWN) {
      tui_.putLine("Unknown command: " + command);
      return true; // Continue the interactive loop
    }

    if (parsed.command == CommandType::QUIT) {
      return false; // Signal to exit the interactive loop
    }

    // Connect, execute command, then disconnect
    try {
      client_.connect();
      
      bool result = false;
      switch (parsed.command) {
      case CommandType::SHOW:
        result = handleShowCommand(parsed);
        break;
      case CommandType::SET:
        result = handleSetCommand(parsed);
        break;
      case CommandType::DELETE:
        result = handleDeleteCommand(parsed);
        break;
      case CommandType::COMMIT:
        result = handleCommitCommand(parsed);
        break;
      default:
        tui_.putLine("Unknown command type");
        result = false;
      }
      
      client_.disconnect(false); // Disconnect transport but don't close session
      return result;
      
    } catch (const std::exception& e) {
      tui_.putLine("Command failed: " + std::string(e.what()));
      client_.disconnect(false); // Ensure disconnect even on error, don't close session
      return true; // Continue the interactive loop
    }
  }


  bool CommandProcessor::handleShowCommand(const ParsedCommand &parsed) {
    if (parsed.target == TargetType::UNKNOWN) {
      tui_.putLine("Usage: show <vrf|interface> [options]");
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
      tui_.putLine("Unknown show target");
      return false;
    }
  }

  bool CommandProcessor::handleShowVrf([[maybe_unused]] const ParsedCommand &parsed) {
    throw netd::shared::NotImplementedError("handleShowVrf: not implemented");
  }

  bool CommandProcessor::handleShowRoute([[maybe_unused]] const ParsedCommand &parsed) {
    throw netd::shared::NotImplementedError("handleShowRoute: not implemented");
  }

  bool CommandProcessor::handleShowInterface([[maybe_unused]] const ParsedCommand &parsed) {
    throw netd::shared::NotImplementedError("handleShowInterface: not implemented");
  }

  bool CommandProcessor::handleSetCommand([[maybe_unused]] const ParsedCommand &parsed) {
    throw netd::shared::NotImplementedError("handleSetCommand: not implemented");
  }

  bool CommandProcessor::handleSetVrf([[maybe_unused]] const ParsedCommand &parsed) {
    throw netd::shared::NotImplementedError("handleSetVrf: not implemented");
  }

  bool CommandProcessor::handleSetInterface([[maybe_unused]] const ParsedCommand &parsed) {
    throw netd::shared::NotImplementedError("handleSetInterface: not implemented");
  }

  bool CommandProcessor::handleSetRoute([[maybe_unused]] const ParsedCommand &parsed) {
    throw netd::shared::NotImplementedError("handleSetRoute: not implemented");
  }

  bool CommandProcessor::handleDeleteCommand([[maybe_unused]] const ParsedCommand &parsed) {
    throw netd::shared::NotImplementedError("handleDeleteCommand: not implemented");
  }

  bool CommandProcessor::handleCommitCommand([[maybe_unused]] const ParsedCommand &parsed) {
    throw netd::shared::NotImplementedError("handleCommitCommand: not implemented");
  }

} // namespace netd::client
