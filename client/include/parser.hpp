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

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace netd::client {

  enum class CommandType { SHOW, SET, DELETE, COMMIT, QUIT, UNKNOWN };

  enum class TargetType { INTERFACE, VRF, ROUTE, UNKNOWN };

  struct ParsedCommand {
    CommandType command;
    TargetType target;
    std::vector<std::string> arguments;

    ParsedCommand()
        : command(CommandType::UNKNOWN), target(TargetType::UNKNOWN) {}
  };

  class CommandParser {
  public:
    CommandParser();
    ~CommandParser();

    // Parse a command string and return the parsed result
    ParsedCommand parse(const std::string &command);

    // Check if the command is valid
    bool isValid(const std::string &command);

  private:
    // Internal parsing methods for different command types
    ParsedCommand parseShowCommand(const std::string &command);
    ParsedCommand parseSetCommand(const std::string &command);
    ParsedCommand parseDeleteCommand(const std::string &command);
    ParsedCommand parseCommitCommand(const std::string &command);

    // Helper methods
    std::vector<std::string> tokenize(const std::string &command);
    CommandType getCommandType(const std::string &cmd);
    TargetType getTargetType(const std::string &target);
  };

} // namespace netd::client
