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

#include <algorithm>
#include <cctype>
#include <client/include/parser.hpp>
#include <shared/include/logger.hpp>
#include <sstream>

// Forward declarations for lexer functions
namespace netd::client {

  CommandParser::CommandParser() {}

  CommandParser::~CommandParser() {}

  ParsedCommand CommandParser::parse(const std::string &command) {
    auto &logger = netd::shared::Logger::getInstance();
    logger.debug("Parsing command: " + command);

    std::vector<std::string> tokens = tokenize(command);
    if (tokens.empty()) {
      return ParsedCommand();
    }

    CommandType cmdType = getCommandType(tokens[0]);

    switch (cmdType) {
    case CommandType::SHOW:
      return parseShowCommand(command);
    case CommandType::SET:
      return parseSetCommand(command);
    case CommandType::DELETE:
      return parseDeleteCommand(command);
    case CommandType::COMMIT:
      return parseCommitCommand(command);
    case CommandType::QUIT:
      {
        ParsedCommand result;
        result.command = CommandType::QUIT;
        return result;
      }
    default:
      logger.warning("Unknown command type: " + tokens[0]);
      return ParsedCommand();
    }
  }

  bool CommandParser::isValid(const std::string &command) {
    ParsedCommand parsed = parse(command);
    return parsed.command != CommandType::UNKNOWN;
  }

  ParsedCommand CommandParser::parseShowCommand(const std::string &command) {
    ParsedCommand result;
    result.command = CommandType::SHOW;

    std::vector<std::string> tokens = tokenize(command);
    if (tokens.size() < 2) {
      return result; // Invalid - need at least "show <target>"
    }

    result.target = getTargetType(tokens[1]);

    // Add remaining tokens as arguments
    for (size_t i = 2; i < tokens.size(); ++i) {
      result.arguments.push_back(tokens[i]);
    }

    return result;
  }

  ParsedCommand CommandParser::parseSetCommand(const std::string &command) {
    ParsedCommand result;
    result.command = CommandType::SET;

    std::vector<std::string> tokens = tokenize(command);
    if (tokens.size() < 2) {
      return result; // Invalid - need at least "set <target>"
    }

    result.target = getTargetType(tokens[1]);

    // Add remaining tokens as arguments
    for (size_t i = 2; i < tokens.size(); ++i) {
      result.arguments.push_back(tokens[i]);
    }

    return result;
  }

  ParsedCommand CommandParser::parseDeleteCommand(const std::string &command) {
    ParsedCommand result;
    result.command = CommandType::DELETE;

    std::vector<std::string> tokens = tokenize(command);
    if (tokens.size() < 2) {
      return result; // Invalid - need at least "delete <target>"
    }

    result.target = getTargetType(tokens[1]);

    // Add remaining tokens as arguments
    for (size_t i = 2; i < tokens.size(); ++i) {
      result.arguments.push_back(tokens[i]);
    }

    return result;
  }

  ParsedCommand CommandParser::parseCommitCommand(const std::string &command) {
    ParsedCommand result;
    result.command = CommandType::COMMIT;
    result.target = TargetType::UNKNOWN; // Commit doesn't have a target

    std::vector<std::string> tokens = tokenize(command);

    // Add all tokens after "commit" as arguments
    for (size_t i = 1; i < tokens.size(); ++i) {
      result.arguments.push_back(tokens[i]);
    }

    return result;
  }

  std::vector<std::string> CommandParser::tokenize(const std::string &command) {
    std::vector<std::string> tokens;
    std::istringstream iss(command);
    std::string token;

    while (iss >> token) {
      tokens.push_back(token);
    }

    return tokens;
  }

  CommandType CommandParser::getCommandType(const std::string &cmd) {
    std::string lowerCmd = cmd;
    std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(),
                   ::tolower);

    if (lowerCmd == "show") {
      return CommandType::SHOW;
    } else if (lowerCmd == "set") {
      return CommandType::SET;
    } else if (lowerCmd == "delete") {
      return CommandType::DELETE;
    } else if (lowerCmd == "commit") {
      return CommandType::COMMIT;
    } else if (lowerCmd == "quit" || lowerCmd == "exit") {
      return CommandType::QUIT;
    } else {
      return CommandType::UNKNOWN;
    }
  }

  TargetType CommandParser::getTargetType(const std::string &target) {
    std::string lowerTarget = target;
    std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(),
                   ::tolower);

    if (lowerTarget == "interface") {
      return TargetType::INTERFACE;
    } else if (lowerTarget == "vrf") {
      return TargetType::VRF;
    } else if (lowerTarget == "route") {
      return TargetType::ROUTE;
    } else {
      return TargetType::UNKNOWN;
    }
  }

} // namespace netd::client
