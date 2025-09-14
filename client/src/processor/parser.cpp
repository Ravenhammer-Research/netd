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

#include <shared/include/expect/base.hpp>
#include <client/include/processor/parser.hpp>
#include <client/include/processor/command.hpp>
#include <client/include/processor/completion.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <cstdio>
#include <cstring>
#include <sstream>

// Do not change these.
#include <client/parser/parser.h>
#include <client/parser/parser_lex.h>

// Only declare yyparse since it's from yacc (not lex)
extern "C" int yyparse();

// Global variables for parser communication
static std::string g_current_command;
static int g_parsed_command_type = 0;
static bool g_parse_success = false;

// Global command object for parsed commands
static netd::client::processor::Command g_parsed_command;

// Action functions called by the yacc parser
extern "C" {
    void set_command_action() {
        g_parsed_command.setCommandType(netd::client::processor::CommandType::SET_CMD);
        g_parse_success = true;
    }
    
    void delete_command_action() {
        g_parsed_command.setCommandType(netd::client::processor::CommandType::DELETE_CMD);
        g_parse_success = true;
    }
    
    void show_command_action() {
        g_parsed_command.setCommandType(netd::client::processor::CommandType::SHOW_CMD);
        g_parse_success = true;
    }
    
    void commit_command_action() {
        g_parsed_command.setCommandType(netd::client::processor::CommandType::COMMIT_CMD);
        g_parse_success = true;
    }
    
    void edit_command_action() {
        g_parsed_command.setCommandType(netd::client::processor::CommandType::EDIT_CMD);
        g_parse_success = true;
    }
    
    void quit_command_action() {
        g_parsed_command.setCommandType(netd::client::processor::CommandType::QUIT_CMD);
        g_parse_success = true;
    }
    
    void set_interface_name(const char* name) {
        g_parsed_command.setInterfaceName(name);
    }
    
    void set_unit_number(int unit) {
        g_parsed_command.setUnitNumber(unit);
    }
    
    void set_ip_address(const char* ip) {
        g_parsed_command.setIpAddress(ip);
    }
    
    void set_description(const char* desc) {
        g_parsed_command.setDescription(desc);
    }
    
    void set_vlan_id(int vlan) {
        g_parsed_command.setVlanId(vlan);
    }
    
    void set_speed_value(const char* speed) {
        g_parsed_command.setSpeedValue(speed);
    }
    
    void set_identifier(const char* id) {
        g_parsed_command.setIdentifier(id);
    }
    
    void set_string_value(const char* str) {
        g_parsed_command.setStringValue(str);
    }
    
    void set_vlan_tagging() {
        g_parsed_command.setVlanTagging(true);
    }
    
    void set_brief_mode() {
        g_parsed_command.setDisplayMode(netd::client::processor::DisplayMode::BRIEF_MODE);
    }
    
    void set_detail_mode() {
        g_parsed_command.setDisplayMode(netd::client::processor::DisplayMode::DETAIL_MODE);
    }
    
    void set_extensive_mode() {
        g_parsed_command.setDisplayMode(netd::client::processor::DisplayMode::EXTENSIVE_MODE);
    }
    
    void set_terse_mode() {
        g_parsed_command.setDisplayMode(netd::client::processor::DisplayMode::TERSE_MODE);
    }
}

// yyerror function - defined in generated parser.c, but we can override it
void yyerror(const char* s) {
    auto &logger = netd::shared::Logger::getInstance();
    logger.error("Parse error: " + std::string(s));
    g_parse_success = false;
}

namespace netd::client::processor {

  CommandProcessor::CommandProcessor(netd::client::tui::TUI& tui, netd::client::netconf::NetconfClient& client)
      : tui_(tui), client_(client) {
    // Set up tab completion with parser-generated keywords
    tui_.setCompletions(CommandCompletion::getAllKeywords());
    
    // Set the NETCONF client for dynamic interface lookup
    CommandCompletion::setNetconfClient(&client_);
  }

  CommandProcessor::~CommandProcessor() {
  }

  bool CommandProcessor::processCommand(const std::string& command) {
    if (command.empty()) {
      return true;
    }

    auto& logger = netd::shared::Logger::getInstance();
    logger.debug("Processing command: " + command);

    // Set up parser input
    g_current_command = command;
    g_parsed_command_type = 0;
    g_parse_success = false;
    
    // Reset parsed command structure
    g_parsed_command.reset();

    // Create a temporary file to feed the command to the parser
    FILE* temp_file = tmpfile();
    if (!temp_file) {
      tui_.putLine("Failed to create temporary file for parsing");
      return true;
    }

    // Write the command to the temporary file
    fprintf(temp_file, "%s\n", command.c_str());
    rewind(temp_file);

    // Set yyin to our temporary file
    yyin = temp_file;

    // Call the yacc parser
    int parse_result = yyparse();

    // Clean up
    fclose(temp_file);

    if (parse_result == 0 && g_parse_success) {
      // Command successfully parsed by yacc/lex
      return handleParsedCommand(g_parsed_command);
    } else {
      tui_.putLine("Syntax error or unknown command: " + command);
      return true;
    }
  }

  bool CommandProcessor::handleParsedCommand(const netd::client::processor::Command& command) {
    try {
      auto& logger = netd::shared::Logger::getInstance();
      
      // Connect to server for command execution
      client_.connect();
      
      bool result = false;
      
      switch (command.getCommandType()) {
      case netd::client::processor::CommandType::SHOW_CMD:
        logger.info("Executing command: " + g_current_command);
        result = true;
        break;
      case netd::client::processor::CommandType::SET_CMD:
        logger.info("Executing command: " + g_current_command);
        result = true;
        break;
      case netd::client::processor::CommandType::DELETE_CMD:
        logger.info("Executing command: " + g_current_command);
        result = true;
        break;
      case netd::client::processor::CommandType::COMMIT_CMD:
        logger.info("Executing command: " + g_current_command);
        result = true;
        break;
      case netd::client::processor::CommandType::EDIT_CMD:
        logger.info("Executing command: " + g_current_command);
        result = true;
        break;
      case netd::client::processor::CommandType::QUIT_CMD:
        tui_.putLine("Exiting...");
        result = false; // Return false to exit the main loop
        break;
      default:
        tui_.putLine("Unknown command type");
        result = false;
      }
      
      // Disconnect from server
      client_.disconnect(false);
      return result;
      
    } catch (const std::exception& e) {
      tui_.putLine("Command failed: " + std::string(e.what()));
      client_.disconnect(false);
      return true;
    }
  }

} // namespace netd::client::processor
