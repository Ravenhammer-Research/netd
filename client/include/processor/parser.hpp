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

#ifndef NETD_CLIENT_PROCESSOR_PARSER_HPP
#define NETD_CLIENT_PROCESSOR_PARSER_HPP

#include <string>
#include <memory>
#include <client/include/netconf/client.hpp>
#include <client/include/tui.hpp>
#include <client/include/processor/command.hpp>

namespace netd::client::processor {

  class CommandProcessor {
  public:
    /**
     * @brief Constructs a CommandProcessor
     * @param tui Reference to the TUI interface
     * @param client Reference to the NETCONF client
     */
    CommandProcessor(netd::client::tui::TUI& tui, netd::client::netconf::NetconfClient& client);

    /**
     * @brief Destructor
     */
    ~CommandProcessor();

    /**
     * @brief Processes a command string using yacc/lex parser
     * @param command The command string to process
     * @return true if command was processed successfully, false to exit
     */
    bool processCommand(const std::string& command);

  private:
    netd::client::tui::TUI& tui_;
    netd::client::netconf::NetconfClient& client_;

    /**
     * @brief Handles parsed command execution
     * @param command_type The type of command that was parsed
     * @return true if command was handled successfully
     */
    bool handleParsedCommand(const Command& command);
  };

} // namespace netd::client::processor

#endif // NETD_CLIENT_PROCESSOR_PARSER_HPP