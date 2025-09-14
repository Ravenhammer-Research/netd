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

#include <client/include/tui.hpp>
#include <client/include/netconf/client.hpp>
#include <client/include/processor/parser.hpp>
#include <client/include/processor/completion.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/exception.hpp>
#include <curses.h>
#include <algorithm>
#include <thread>

namespace netd::client::tui {

  constexpr size_t MAX_HISTORY_SIZE = 10240;

  TUI::TUI() : initialized_(false), prompt_("netc> "), commandHistoryPosition_(-1), scrollOffset_(0), connectionStatus_("Not connected"), debugLevel_(0), destroying_(false), client_(nullptr) {}

  TUI::~TUI() { 
    destroying_ = true;
    cleanup(); 
  }
  bool TUI::initialize() {
    if (initialized_) {
      return true;
    }

    setupCurses();
    configureSignalHandler();
    
    setLoggerInstance(this);
    initializeLogger();
    
    netd::shared::Logger::getInstance().debug("TUI Logger initialized successfully");
    
    initialized_ = true;
    return true;
  }


  void TUI::addToCommandHistory(const std::string &command) {
    if (commandHistory_.empty() || commandHistory_.back() != command) {
      commandHistory_.push_back(command);
      while (commandHistory_.size() > MAX_HISTORY_SIZE) {
        commandHistory_.erase(commandHistory_.begin());
      }
    }
    setHistoryPosition(-1);
  }

  const std::vector<std::string>& TUI::getCommandHistory() const {
    return commandHistory_;
  }

  int TUI::getHistoryPosition() {
    return commandHistoryPosition_;
  }

  void TUI::setHistoryPosition(int position) {
    commandHistoryPosition_ = position;
  }

  void TUI::advanceHistoryPosition() {
    if (commandHistoryPosition_ < static_cast<int>(commandHistory_.size()) - 1) {
      commandHistoryPosition_++;
    }
  }

  void TUI::setCompletions(const std::vector<std::string> &completions) {
    completions_ = completions;
  }

  std::string TUI::completeCommand(const std::string &partial) {
    if (partial.empty()) {
      return partial;
    }

    // Use contextual completion if available, otherwise fall back to simple completion
    std::vector<std::string> matches;
    
    // Try contextual completion first
    if (!completions_.empty()) {
      // For now, use simple completion - contextual completion would need the full command line
      // which we don't have in this interface
      for (const auto &completion : completions_) {
        if (completion.find(partial) == 0) {
          matches.push_back(completion);
        }
      }
    }

    if (matches.empty()) {
      return partial;
    } else if (matches.size() == 1) {
      return matches[0];
    } else {
      // Multiple matches - find common prefix
      std::string common = matches[0];
      for (size_t i = 1; i < matches.size(); ++i) {
        size_t j = 0;
        while (j < common.length() && j < matches[i].length() &&
               common[j] == matches[i][j]) {
          j++;
        }
        common = common.substr(0, j);
      }
      return common;
    }
  }

  std::string TUI::completeCommandContextual(const std::string &command_line) {
    if (command_line.empty()) {
      return command_line;
    }

    // Debug output
    netd::client::processor::CommandCompletion::debugCompletions(command_line);

    // Use the processor's contextual completion
    auto matches = netd::client::processor::CommandCompletion::findContextualCompletions(command_line);
    
    if (matches.empty()) {
      return command_line;
    }
    
    // Check if we're at the end of a word (no trailing space)
    bool at_word_end = (command_line.back() != ' ');
    
    if (matches.size() == 1) {
      // Single match - complete it
      if (at_word_end) {
        // Find the last word and replace it
        size_t last_space = command_line.find_last_of(' ');
        if (last_space == std::string::npos) {
          return matches[0];
        } else {
          return command_line.substr(0, last_space + 1) + matches[0];
        }
      } else {
        // Add the match after the space
        return command_line + matches[0];
      }
    } else {
      // Multiple matches - show available options and find common prefix
      std::string common = netd::client::processor::CommandCompletion::getCommonPrefix(matches);
      
      // Show available completions (like bash does)
      auto& logger = netd::shared::Logger::getInstance();
      logger.info("Available completions:");
      for (const auto& match : matches) {
        logger.info("  " + match);
      }
      
      // If common prefix is empty, don't change anything but ensure we return the original
      if (common.empty()) {
        return command_line;
      }
      
      if (at_word_end) {
        // Find the last word and replace it with common prefix
        size_t last_space = command_line.find_last_of(' ');
        if (last_space == std::string::npos) {
          return common;
        } else {
          return command_line.substr(0, last_space + 1) + common;
        }
      } else {
        // Add the common prefix after the space
        return command_line + common;
      }
    }
  }

} // namespace netd::client::tui