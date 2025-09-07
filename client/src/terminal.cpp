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

#include <client/include/terminal.hpp>
#include <iostream>

namespace netd::client {

  Terminal::Terminal() : initialized_(false) {}

  Terminal::~Terminal() { cleanup(); }

  bool Terminal::initialize() {
    if (initialized_) {
      return true;
    }
    initialized_ = true;
    return true;
  }

  void Terminal::cleanup() {
    if (initialized_) {
      initialized_ = false;
    }
  }

  std::string Terminal::readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
  }

  void Terminal::write(const std::string &text) {
    std::cout << text;
    std::cout.flush();
  }

  void Terminal::writeLine(const std::string &text) {
    std::cout << text << std::endl;
  }

  void Terminal::clear() {
    // Simple terminal clear - could be enhanced for different platforms
    std::cout << "\033[2J\033[H";
    std::cout.flush();
  }

  void Terminal::refresh() {
    std::cout.flush();
  }

  void Terminal::addToHistory(const std::string &command) {
    // Basic history management - could be enhanced
    if (history_.empty() || history_.back() != command) {
      history_.push_back(command);
      if (history_.size() > 100) {
        history_.erase(history_.begin());
      }
    }
  }

  std::string Terminal::getHistoryUp() {
    if (history_.empty()) {
      return "";
    }
    if (historyPosition_ == -1) {
      historyPosition_ = history_.size() - 1;
    } else if (historyPosition_ > 0) {
      historyPosition_--;
    }
    return history_[historyPosition_];
  }

  std::string Terminal::getHistoryDown() {
    if (history_.empty()) {
      return "";
    }
    if (historyPosition_ < static_cast<int>(history_.size()) - 1) {
      historyPosition_++;
      return history_[historyPosition_];
    } else {
      historyPosition_ = -1;
      return "";
    }
  }

  void Terminal::resetHistoryPosition() { 
    historyPosition_ = -1; 
  }

  void Terminal::setCompletions(const std::vector<std::string> &completions) {
    completions_ = completions;
  }

  std::string Terminal::completeCommand(const std::string &partial) {
    if (partial.empty() || completions_.empty()) {
      return partial;
    }

    // Find commands that start with the partial string
    std::vector<std::string> matches;
    for (const auto &completion : completions_) {
      if (completion.find(partial) == 0) {
        matches.push_back(completion);
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

  void Terminal::runInteractive() {
    if (!initialized_) {
      return;
    }

    std::string line;
    while (true) {
      std::cout << prompt_;
      std::cout.flush();
      
      line = readLine();

      if (line.empty()) {
        continue;
      }

      // Handle built-in commands
      if (line == "quit" || line == "exit") {
        break;
      } else if (line == "help") {
        writeLine("Available commands:");
        writeLine("  show vrf                    - Show VRF information");
        writeLine("  show interface              - Show interface information");
        writeLine("  set interface <args>        - Configure interface");
        writeLine("  set vrf <args>              - Configure VRF");
        writeLine("  commit                      - Commit configuration");
        writeLine("  help                        - Show this help");
        writeLine("  quit/exit                   - Exit CLI");
        continue;
      }

      // Call command handler if set
      if (commandHandler_) {
        if (!commandHandler_(line)) {
          writeLine("Error: Command failed");
        }
      } else {
        writeLine("No command handler set");
      }
    }
  }

  void Terminal::redrawPrompt() {
    // Basic implementation - could be enhanced
    std::cout.flush();
  }

} // namespace netd::client