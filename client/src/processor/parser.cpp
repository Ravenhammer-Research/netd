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

#include <client/include/processor/command.hpp>
#include <client/include/processor/completion.hpp>
#include <client/include/processor/parser.hpp>
#include <cstdio>
#include <cstring>
#include <shared/include/exception.hpp>
#include <shared/include/expect/base.hpp>
#include <shared/include/logger.hpp>
#include <sstream>

#include <client/parser/parser.h>
#include <client/parser/parser_lex.h>

extern "C" int yyparse();

static std::string g_current_command;
static int g_parsed_command_type = 0;
static bool g_parse_success = false;

static netd::client::processor::Command g_parsed_command;

static const char *g_input_buffer = nullptr;
static size_t g_input_pos = 0;
static size_t g_input_len = 0;
static std::string g_input_string;

static int g_help_topic = 0;
extern "C" int custom_input(char *buf, int max_size) {
  if (g_input_pos >= g_input_len) {
    return 0;
  }

  int bytes_to_read =
      std::min(max_size, static_cast<int>(g_input_len - g_input_pos));
  if (bytes_to_read > 0) {
    std::memcpy(buf, g_input_buffer + g_input_pos, bytes_to_read);
    g_input_pos += bytes_to_read;

    auto &logger = netd::shared::Logger::getInstance();
    logger.debug("Custom input: read " + std::to_string(bytes_to_read) +
                 " bytes, pos=" + std::to_string(g_input_pos) + "/" +
                 std::to_string(g_input_len));
  }
  return bytes_to_read;
}

static void setupStringInput(const std::string &command) {
  g_input_string = command + "\n";
  g_input_buffer = g_input_string.c_str();
  g_input_pos = 0;
  g_input_len = g_input_string.length();

  g_help_topic = 0;

  auto &logger = netd::shared::Logger::getInstance();
  logger.debug("Setup string input: '" + command +
               "' (length: " + std::to_string(g_input_len) + ")");
}

extern "C" {
void set_command_action() {
  g_parsed_command.setCommandType(
      netd::client::processor::CommandType::SET_CMD);
  g_parse_success = true;
}

void delete_command_action() {
  g_parsed_command.setCommandType(
      netd::client::processor::CommandType::DELETE_CMD);
  g_parse_success = true;
}

void show_command_action() {
  g_parsed_command.setCommandType(
      netd::client::processor::CommandType::SHOW_CMD);
  g_parse_success = true;
}

void commit_command_action() {
  g_parsed_command.setCommandType(
      netd::client::processor::CommandType::COMMIT_CMD);
  g_parse_success = true;
}

void edit_command_action() {
  g_parsed_command.setCommandType(
      netd::client::processor::CommandType::EDIT_CMD);
  g_parse_success = true;
}

void quit_command_action() {
  g_parsed_command.setCommandType(
      netd::client::processor::CommandType::QUIT_CMD);
  g_parse_success = true;
}

void help_command_action() {
  g_parsed_command.setCommandType(
      netd::client::processor::CommandType::HELP_CMD);
  g_parse_success = true;
}

void set_help_topic(int topic) { g_help_topic = topic; }

void set_interface_name(const char *name) {
  g_parsed_command.setInterfaceName(name);
}

void set_unit_number(int unit) { g_parsed_command.setUnitNumber(unit); }

void set_ip_address(const char *ip) { g_parsed_command.setIpAddress(ip); }

void set_description(const char *desc) {
  g_parsed_command.setDescription(desc);
}

void set_vlan_id(int vlan) { g_parsed_command.setVlanId(vlan); }

void set_speed_value(const char *speed) {
  g_parsed_command.setSpeedValue(speed);
}

void set_identifier(const char *id) { g_parsed_command.setIdentifier(id); }

void set_string_value(const char *str) { g_parsed_command.setStringValue(str); }

void set_vlan_tagging() { g_parsed_command.setVlanTagging(true); }

void set_brief_mode() {
  g_parsed_command.setDisplayMode(
      netd::client::processor::DisplayMode::BRIEF_MODE);
}

void set_detail_mode() {
  g_parsed_command.setDisplayMode(
      netd::client::processor::DisplayMode::DETAIL_MODE);
}

void set_extensive_mode() {
  g_parsed_command.setDisplayMode(
      netd::client::processor::DisplayMode::EXTENSIVE_MODE);
}

void set_terse_mode() {
  g_parsed_command.setDisplayMode(
      netd::client::processor::DisplayMode::TERSE_MODE);
}
}

void yyerror(const char *s) {
  auto &logger = netd::shared::Logger::getInstance();
  logger.error("Parse error: " + std::string(s));
  g_parse_success = false;
}

namespace netd::client::processor {

  CommandProcessor::CommandProcessor(
      netd::client::tui::TUI &tui, netd::client::netconf::NetconfClient &client)
      : tui_(tui), client_(client) {
    tui_.setCompletions(CommandCompletion::getAllKeywords());

    CommandCompletion::setNetconfClient(&client_);
  }

  CommandProcessor::~CommandProcessor() {}

  bool CommandProcessor::processCommand(const std::string &command) {
    if (command.empty()) {
      return true;
    }

    auto &logger = netd::shared::Logger::getInstance();
    logger.debug("Processing command: " + command);

    g_current_command = command;
    g_parsed_command_type = 0;
    g_parse_success = false;

    g_parsed_command.reset();

    setupStringInput(command);

    int parse_result = yyparse();

    if (parse_result == 0 && g_parse_success) {
      return handleParsedCommand(g_parsed_command);
    } else {
      tui_.putLine("Syntax error or unknown command: " + command);
      return true;
    }
  }

  bool CommandProcessor::handleParsedCommand(
      const netd::client::processor::Command &command) {
    try {
      auto &logger = netd::shared::Logger::getInstance();

      bool result = false;

      switch (command.getCommandType()) {
      case netd::client::processor::CommandType::SHOW_CMD:
        logger.info("Executing command: " + g_current_command);
        try {
          client_.connect();
          result = true;
        } catch (const std::exception &e) {
          tui_.putLine("Command failed: " + std::string(e.what()));
          result = true;
        }
        break;
      case netd::client::processor::CommandType::SET_CMD:
        logger.info("Executing command: " + g_current_command);
        try {
          client_.connect();
          result = true;
        } catch (const std::exception &e) {
          tui_.putLine("Command failed: " + std::string(e.what()));
          result = true;
        }
        break;
      case netd::client::processor::CommandType::DELETE_CMD:
        logger.info("Executing command: " + g_current_command);
        try {
          client_.connect();
          result = true;
        } catch (const std::exception &e) {
          tui_.putLine("Command failed: " + std::string(e.what()));
          result = true;
        }
        break;
      case netd::client::processor::CommandType::COMMIT_CMD:
        logger.info("Executing command: " + g_current_command);
        try {
          client_.connect();
          result = true;
        } catch (const std::exception &e) {
          tui_.putLine("Command failed: " + std::string(e.what()));
          result = true;
        }
        break;
      case netd::client::processor::CommandType::EDIT_CMD:
        logger.info("Executing command: " + g_current_command);
        try {
          client_.connect();
          result = true;
        } catch (const std::exception &e) {
          tui_.putLine("Command failed: " + std::string(e.what()));
          result = true;
        }
        break;
      case netd::client::processor::CommandType::QUIT_CMD:
        tui_.putLine("Exiting...");
        result = false;
        break;
      case netd::client::processor::CommandType::HELP_CMD:
        if (g_help_topic == 0) {
          tui_.putLine("Available commands:");
          tui_.putLine(
              "  show <config>     - Display configuration information");
          tui_.putLine("  set <config>      - Set configuration values");
          tui_.putLine("  delete <config>   - Delete configuration values");
          tui_.putLine("  commit            - Commit configuration changes");
          tui_.putLine("  edit <config>     - Edit configuration");
          tui_.putLine("  quit/exit         - Exit the program");
          tui_.putLine("  help [topic]      - Show help for specific command");
          tui_.putLine("");
          tui_.putLine("Examples:");
          tui_.putLine("  help show         - Help for show command");
          tui_.putLine(
              "  help interfaces   - Help for interface configuration");
        } else {
          switch (g_help_topic) {
          case 1:
            tui_.putLine("SET command - Configure system parameters");
            tui_.putLine("Usage: set <config>");
            tui_.putLine("Examples:");
            tui_.putLine("  set interfaces xe-0/0/0 unit 0 family inet address "
                         "192.168.1.1/24");
            tui_.putLine("  set interfaces xe-0/0/0 vlan-tagging");
            break;
          case 2:
            tui_.putLine("DELETE command - Remove configuration");
            tui_.putLine("Usage: delete <config>");
            tui_.putLine("Examples:");
            tui_.putLine("  delete interfaces xe-0/0/0 unit 0 family inet "
                         "address 192.168.1.1/24");
            tui_.putLine("  delete interfaces xe-0/0/0 vlan-tagging");
            break;
          case 3:
            tui_.putLine("SHOW command - Display configuration and status");
            tui_.putLine("Usage: show <config> [display-option]");
            tui_.putLine("Display options: terse, brief, detail, extensive");
            tui_.putLine("Examples:");
            tui_.putLine("  show interfaces");
            tui_.putLine("  show interfaces xe-0/0/0");
            tui_.putLine("  show version brief");
            break;
          case 4:
            tui_.putLine("COMMIT command - Apply configuration changes");
            tui_.putLine("Usage: commit");
            tui_.putLine("Note: Commits all pending configuration changes");
            break;
          case 5:
            tui_.putLine("EDIT command - Enter configuration edit mode");
            tui_.putLine("Usage: edit <config>");
            tui_.putLine("Examples:");
            tui_.putLine("  edit interfaces xe-0/0/0 unit 0");
            tui_.putLine("  edit routing-instances VRF1");
            break;
          case 6:
            tui_.putLine("QUIT/EXIT command - Exit the program");
            tui_.putLine("Usage: quit or exit");
            break;
          case 7:
            tui_.putLine("INTERFACES configuration:");
            tui_.putLine("  Configure network interfaces");
            tui_.putLine("Examples:");
            tui_.putLine("  set interfaces xe-0/0/0 unit 0 family inet address "
                         "192.168.1.1/24");
            tui_.putLine("  set interfaces xe-0/0/0 vlan-tagging");
            tui_.putLine("  show interfaces xe-0/0/0");
            break;
          case 8:
            tui_.putLine("ROUTING-INSTANCES configuration:");
            tui_.putLine("  Configure virtual routing instances (VRFs)");
            tui_.putLine("Examples:");
            tui_.putLine("  set routing-instances VRF1 instance-type vrf");
            tui_.putLine("  show routing-instances VRF1");
            break;
          case 9:
            tui_.putLine("ROUTING-OPTIONS configuration:");
            tui_.putLine("  Configure routing protocols and static routes");
            tui_.putLine("Examples:");
            tui_.putLine("  set routing-options static route 0.0.0.0/0 "
                         "next-hop 192.168.1.1");
            break;
          default:
            tui_.putLine("Unknown help topic");
          }
        }
        g_help_topic = 0;
        result = true;
        break;
      default:
        tui_.putLine("Unknown command type");
        result = false;
      }

      client_.disconnect(false);
      return result;

    } catch (const std::exception &e) {
      tui_.putLine("Command failed: " + std::string(e.what()));
      client_.disconnect(false);
      return true;
    }
  }

} // namespace netd::client::processor
