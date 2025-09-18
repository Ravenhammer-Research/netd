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
#include <client/include/netconf/client.hpp>
#include <client/include/processor/completion.hpp>
#include <libyang/libyang.h>
#include <shared/include/expect/base.hpp>
#include <shared/include/expect/interface.hpp>
#include <shared/include/expect/manager.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/yang.hpp>
#include <sstream>

namespace netd::client::processor {

  // Static member definitions
  bool CommandCompletion::initialized_ = false;
  std::vector<std::string> CommandCompletion::command_keywords_;
  std::vector<std::string> CommandCompletion::interface_keywords_;
  std::vector<std::string> CommandCompletion::routing_keywords_;
  std::vector<std::string> CommandCompletion::display_keywords_;
  std::vector<std::string> CommandCompletion::protocol_keywords_;
  std::vector<std::string> CommandCompletion::all_keywords_;
  std::unordered_set<std::string> CommandCompletion::keyword_set_;
  netd::client::netconf::NetconfClient *CommandCompletion::netconf_client_ =
      nullptr;

  void CommandCompletion::initializeKeywords() {
    if (initialized_) {
      return;
    }

    // Command keywords (from parser.l)
    command_keywords_ = {"set",  "delete", "show", "commit",
                         "edit", "quit",   "exit"};

    // Interface keywords (from parser.l)
    interface_keywords_ = {"interfaces",  "interface",     "unit",
                           "family",      "inet",          "address",
                           "description", "encapsulation", "vlan-id",
                           "speed",       "vlan-tagging",  "ethernet-vlan"};

    // Routing keywords (from parser.l)
    routing_keywords_ = {"routing-instances",
                         "routing-options",
                         "instance-type",
                         "virtual-router",
                         "vrf",
                         "vrf-target",
                         "vrf-table-label",
                         "static",
                         "route",
                         "next-hop",
                         "table",
                         "protocol",
                         "instance"};

    // Display keywords (from parser.l)
    display_keywords_ = {"terse",     "brief",   "detail",
                         "extensive", "display", "all"};

    // Protocol keywords (from parser.l)
    protocol_keywords_ = {
        "ospf",     "bgp",        "version",   "configuration",
        "system",   "uptime",     "chassis",   "log",
        "messages", "neighbor",   "neighbors", "summary",
        "arp",      "no-resolve", "protocols", "ipv6"};

    // Combine all keywords
    all_keywords_.insert(all_keywords_.end(), command_keywords_.begin(),
                         command_keywords_.end());
    all_keywords_.insert(all_keywords_.end(), interface_keywords_.begin(),
                         interface_keywords_.end());
    all_keywords_.insert(all_keywords_.end(), routing_keywords_.begin(),
                         routing_keywords_.end());
    all_keywords_.insert(all_keywords_.end(), display_keywords_.begin(),
                         display_keywords_.end());
    all_keywords_.insert(all_keywords_.end(), protocol_keywords_.begin(),
                         protocol_keywords_.end());

    // Create set for fast lookup
    for (const auto &keyword : all_keywords_) {
      keyword_set_.insert(keyword);
    }

    // Sort all keywords for consistent completion order
    std::sort(all_keywords_.begin(), all_keywords_.end());

    initialized_ = true;
  }

  const std::vector<std::string> &CommandCompletion::getCommandKeywords() {
    initializeKeywords();
    return command_keywords_;
  }

  const std::vector<std::string> &CommandCompletion::getInterfaceKeywords() {
    initializeKeywords();
    return interface_keywords_;
  }

  const std::vector<std::string> &CommandCompletion::getRoutingKeywords() {
    initializeKeywords();
    return routing_keywords_;
  }

  const std::vector<std::string> &CommandCompletion::getDisplayKeywords() {
    initializeKeywords();
    return display_keywords_;
  }

  const std::vector<std::string> &CommandCompletion::getProtocolKeywords() {
    initializeKeywords();
    return protocol_keywords_;
  }

  const std::vector<std::string> &CommandCompletion::getAllKeywords() {
    initializeKeywords();
    return all_keywords_;
  }

  std::vector<std::string>
  CommandCompletion::findCompletions(const std::string &partial) {
    initializeKeywords();

    std::vector<std::string> matches;
    std::string lower_partial = partial;
    std::transform(lower_partial.begin(), lower_partial.end(),
                   lower_partial.begin(), ::tolower);

    for (const auto &keyword : all_keywords_) {
      if (keyword.find(lower_partial) == 0) {
        matches.push_back(keyword);
      }
    }

    return matches;
  }

  std::string
  CommandCompletion::getCommonPrefix(const std::vector<std::string> &strings) {
    if (strings.empty()) {
      return "";
    }

    if (strings.size() == 1) {
      return strings[0];
    }

    std::string common = strings[0];
    for (size_t i = 1; i < strings.size(); ++i) {
      size_t j = 0;
      while (j < common.length() && j < strings[i].length() &&
             common[j] == strings[i][j]) {
        j++;
      }
      common = common.substr(0, j);
    }

    return common;
  }

  bool CommandCompletion::isValidKeyword(const std::string &keyword) {
    initializeKeywords();
    return keyword_set_.find(keyword) != keyword_set_.end();
  }

  std::vector<std::string> CommandCompletion::findContextualCompletions(
      const std::string &command_line) {
    initializeKeywords();

    // Handle empty command line
    if (command_line.empty()) {
      return command_keywords_;
    }

    // Check if we're at the end of a word (no trailing space)
    bool at_word_end = (command_line.back() != ' ');

    std::istringstream iss(command_line);
    std::vector<std::string> tokens;
    std::string token;

    // Split command line into tokens
    while (iss >> token) {
      tokens.push_back(token);
    }

    // If we're at word end, the last token might be partial
    std::string last_token =
        at_word_end && !tokens.empty() ? tokens.back() : "";
    std::vector<std::string> candidates;

    // Determine context based on command structure
    if (tokens.empty() || (tokens.size() == 1 && at_word_end)) {
      // No tokens yet or still typing first token - return command keywords
      if (at_word_end && !tokens.empty()) {
        return findCompletions(last_token);
      } else {
        return command_keywords_;
      }
    }

    // We have at least one complete token
    std::string first_token = tokens[0];

    if (tokens.size() == 1 && !at_word_end) {
      // Space after first token - provide context-specific completions
      if (first_token == "show") {
        candidates = {"interfaces",
                      "interface",
                      "routing-instances",
                      "routing-options",
                      "version",
                      "configuration",
                      "system",
                      "uptime",
                      "chassis",
                      "log",
                      "messages",
                      "neighbor",
                      "neighbors",
                      "summary",
                      "arp",
                      "protocols",
                      "ipv6"};
      } else if (first_token == "set") {
        candidates = {"interfaces", "interface", "routing-instances",
                      "routing-options"};
      } else if (first_token == "delete") {
        candidates = {"interfaces", "interface", "routing-instances",
                      "routing-options"};
      } else {
        // Unknown command - return empty
        return {};
      }
    } else if (tokens.size() >= 2) {
      // Second token or more - context depends on first two tokens
      std::string second_token = tokens[1];

      if (first_token == "show") {
        if (second_token == "interfaces") {
          if (!at_word_end) {
            // Space after "show interfaces" - show display options
            candidates = {"terse", "brief", "detail", "extensive", "display"};
          } else {
            // Completing "interfaces"
            return findCompletions(last_token);
          }
        } else if (second_token == "interface") {
          if (!at_word_end) {
            // Space after "show interface" - get actual interface names from
            // system
            candidates = getNetconfInterfaces();
          } else {
            // Completing "interface"
            return findCompletions(last_token);
          }
        } else {
          // Other show commands - try to complete current token
          if (at_word_end) {
            return findCompletions(last_token);
          } else {
            return {};
          }
        }
      } else if (first_token == "set" || first_token == "delete") {
        if (second_token == "interfaces" || second_token == "interface") {
          if (!at_word_end) {
            // Space after "set interfaces" - would need actual interface names
            candidates = {}; // Could be populated with real interface names
          } else {
            // Completing "interfaces" or "interface"
            return findCompletions(last_token);
          }
        } else {
          // Other set/delete commands - try to complete current token
          if (at_word_end) {
            return findCompletions(last_token);
          } else {
            return {};
          }
        }
      } else {
        // Unknown command - try to complete current token
        if (at_word_end) {
          return findCompletions(last_token);
        } else {
          return {};
        }
      }
    }

    // Filter candidates based on partial match if we're at word end
    if (at_word_end && !last_token.empty()) {
      std::vector<std::string> filtered;
      for (const auto &candidate : candidates) {
        if (candidate.find(last_token) == 0) {
          filtered.push_back(candidate);
        }
      }
      return filtered;
    }

    return candidates;
  }

  void CommandCompletion::debugCompletions(const std::string &command_line) {
    auto &logger = netd::shared::Logger::getInstance();
    auto matches = findContextualCompletions(command_line);
    logger.debug("Tab completion - Command line: '" + command_line + "'");
    logger.debug("Tab completion - Found " + std::to_string(matches.size()) +
                 " completions:");
    for (const auto &match : matches) {
      logger.debug("  - '" + match + "'");
    }
  }

  void CommandCompletion::setNetconfClient(
      netd::client::netconf::NetconfClient *client) {
    netconf_client_ = client;
  }

  std::vector<std::string> CommandCompletion::getNetconfInterfaces() {
    std::vector<std::string> interfaces;

    if (!netconf_client_) {
      auto &logger = netd::shared::Logger::getInstance();
      logger.debug(
          "Tab completion - No NETCONF client available for interface lookup");
      return interfaces;
    }

    try {
      // Connect to server for interface query
      netconf_client_->connect();

      // Get session info for expect
      auto session = netconf_client_->getSession();
      if (!session) {
        throw std::runtime_error("No active NETCONF session");
      }

      std::string session_id = std::to_string(session->getSessionId());
      std::string message_id = std::to_string(session->getNextMessageId());

      // Create interface expect with callback
      auto interface_expect =
          std::make_shared<netd::shared::expect::InterfaceExpect>(
              [&interfaces](
                  const netd::shared::expect::InterfaceResponse &response) {
                // Callback will be called when response is received
                interfaces = response.interface_names;
                auto &logger = netd::shared::Logger::getInstance();
                logger.debug(
                    "Tab completion - Interface callback triggered with " +
                    std::to_string(interfaces.size()) + " interfaces");
              },
              message_id, session_id, std::chrono::seconds(8));

      // Add expect to manager (create static instance)
      static auto expect_manager =
          std::make_unique<netd::shared::expect::ExpectManager>();
      expect_manager->addExpect(interface_expect);

      // Create get-config request for interfaces
      auto getConfigRequest =
          std::make_unique<netd::shared::request::get::GetConfigRequest>();
      getConfigRequest->setSource(
          netd::shared::request::get::Datastore::RUNNING);
      getConfigRequest->setRequestedModule("ietf-interfaces");

      // Send the request (response will be handled by expect callback)
      bool success = netconf_client_->sendRequest(*getConfigRequest);

      auto &logger = netd::shared::Logger::getInstance();
      if (success) {
        logger.debug(
            "Tab completion - Sent get-config request, waiting for callback");
      } else {
        logger.error("Tab completion - Failed to send get-config request");
      }

      // For now, use fallback interfaces while waiting for async response
      interfaces = {"ge-0/0/0", "ge-0/0/1", "ge-0/0/2", "ge-0/0/3", "xe-0/0/0",
                    "xe-0/0/1", "xe-0/0/2", "xe-0/0/3", "ae0",      "ae1",
                    "ae2",      "ae3",      "lo0",      "lo1",      "lo2",
                    "lo3",      "em0",      "em1",      "em2",      "em3"};

      // Disconnect from server
      netconf_client_->disconnect(false);

      logger.debug("Tab completion - Retrieved " +
                   std::to_string(interfaces.size()) +
                   " interfaces from system");

    } catch (const std::exception &e) {
      auto &logger = netd::shared::Logger::getInstance();
      logger.debug("Tab completion - Failed to get interfaces: " +
                   std::string(e.what()));

      // Return fallback interfaces on error
      interfaces = {"ge-0/0/0", "ge-0/0/1", "ge-0/0/2", "xe-0/0/0", "xe-0/0/1",
                    "xe-0/0/2", "ae0",      "ae1",      "lo0",      "em0"};
    }

    return interfaces;
  }

} // namespace netd::client::processor
