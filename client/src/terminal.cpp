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
#include <client/include/terminal.hpp>
#include <curses.h>
#include <shared/include/logger.hpp>
#include <string>

namespace netd::client {

  constexpr char KEY_NEWLINE_CONST = 0x0A;
  constexpr char KEY_CARRIAGE_RETURN_CONST = 0x0D;
  constexpr char KEY_TAB_CONST = 0x09;
  constexpr char KEY_DELETE_CONST = 0x7F;
  constexpr char KEY_BACKSPACE_CONST = 0x08;

  constexpr char KEY_LEFT_CONST = 0x44;
  constexpr char KEY_RIGHT_CONST = 0x43;
  constexpr char KEY_UP_CONST = 0x41;
  constexpr char KEY_DOWN_CONST = 0x42;

  [[maybe_unused]] constexpr int MAX_HISTORY_SIZE = 100;
  [[maybe_unused]] constexpr int PROMPT_ROW = 0;
  [[maybe_unused]] constexpr int PROMPT_COL = 0;

  Terminal::Terminal()
      : initialized_(false), prompt_("net> "), historyPosition_(-1),
        cursorPosition_(0) {}

  Terminal::~Terminal() { cleanup(); }

  bool Terminal::initialize() {
    if (initialized_) {
      return true;
    }

    setupCurses();
    initialized_ = true;
    return true;
  }

  void Terminal::cleanup() {
    if (initialized_) {
      endwin();
      initialized_ = false;
    }
  }

  void Terminal::setupCurses() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Position cursor at top initially, bottom line is reserved for prompt
    move(0, 0);

    refresh();
  }

  std::string Terminal::readLine() {
    if (!initialized_) {
      return "";
    }

    currentLine_.clear();
    cursorPosition_ = 0;
    historyPosition_ = -1;

    char key;
    while ((key = getch()) != KEY_NEWLINE_CONST &&
           key != KEY_CARRIAGE_RETURN_CONST) {
      handleKeyInput(key);
    }

    std::string result = currentLine_;
    if (!result.empty()) {
      addToHistory(result);
    }

    return result;
  }

  void Terminal::handleKeyInput(char key) {
    switch (key) {
    case KEY_BACKSPACE_CONST:
    case KEY_DELETE_CONST:
      if (cursorPosition_ > 0) {
        currentLine_.erase(cursorPosition_ - 1, 1);
        cursorPosition_--;
        updateDisplay();
      }
      break;

    case KEY_LEFT_CONST:
      if (cursorPosition_ > 0) {
        cursorPosition_--;
        updateDisplay();
      }
      break;

    case KEY_RIGHT_CONST:
      if (cursorPosition_ < static_cast<int>(currentLine_.length())) {
        cursorPosition_++;
        updateDisplay();
      }
      break;

    case KEY_UP_CONST:
      if (!history_.empty()) {
        if (historyPosition_ == -1) {
          historyPosition_ = history_.size() - 1;
        } else if (historyPosition_ > 0) {
          historyPosition_--;
        }
        currentLine_ = history_[historyPosition_];
        cursorPosition_ = currentLine_.length();
        updateDisplay();
      }
      break;

    case KEY_DOWN_CONST:
      if (historyPosition_ != -1) {
        if (historyPosition_ < static_cast<int>(history_.size()) - 1) {
          historyPosition_++;
          currentLine_ = history_[historyPosition_];
        } else {
          historyPosition_ = -1;
          currentLine_.clear();
        }
        cursorPosition_ = currentLine_.length();
        updateDisplay();
      }
      break;

    case KEY_TAB_CONST: {
      std::string completed = completeCommand(currentLine_);
      if (!completed.empty() && completed != currentLine_) {
        currentLine_ = completed;
        cursorPosition_ = currentLine_.length();
        updateDisplay();
      }
    } break;

    default:
      if (isprint(key)) {
        currentLine_.insert(cursorPosition_, 1, static_cast<char>(key));
        cursorPosition_++;
        updateDisplay();
      }
      break;
    }
  }

  void Terminal::updateDisplay() {
    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);
    move(maxy - 1, 0);
    clrtoeol();
    printw("%s%s", prompt_.c_str(), currentLine_.c_str());
    move(maxy - 1, prompt_.length() + cursorPosition_);
    refresh();
  }

  void Terminal::redrawPrompt() {
    if (initialized_) {
      // Save current cursor position
      int cury, curx;
      getyx(stdscr, cury, curx);

      // Redraw the prompt
      int maxy, maxx;
      getmaxyx(stdscr, maxy, maxx);
      move(maxy - 1, 0);
      clrtoeol();
      printw("%s", prompt_.c_str());
      move(maxy - 1, prompt_.length());
      refresh();

      // Restore original cursor position
      move(cury, curx);
    }
  }

  void Terminal::write(const std::string &text) {
    if (initialized_) {
      int maxy, maxx;
      getmaxyx(stdscr, maxy, maxx);

      // Save current cursor position
      int cury, curx;
      getyx(stdscr, cury, curx);

      // If we're at the bottom line, move up one line
      if (cury >= maxy - 1) {
        move(maxy - 2, 0);
      }

      printw("%s", text.c_str());
      refresh();
    }
  }

  void Terminal::writeLine(const std::string &text) {
    if (initialized_) {
      int maxy, maxx;
      getmaxyx(stdscr, maxy, maxx);

      // Save current cursor position
      int cury, curx;
      getyx(stdscr, cury, curx);

      // If we're at the bottom line, move up one line
      if (cury >= maxy - 1) {
        move(maxy - 2, 0);
      }

      printw("%s\n", text.c_str());
      refresh();
    }
  }

  void Terminal::clear() {
    if (initialized_) {
      clear();
      refresh();
    }
  }

  void Terminal::refresh() {
    if (initialized_) {
      ::refresh();
    }
  }

  void Terminal::addToHistory(const std::string &command) {
    // Don't add duplicate consecutive commands
    if (history_.empty() || history_.back() != command) {
      history_.push_back(command);
      // Limit history size
      if (history_.size() > 100) {
        history_.erase(history_.begin());
      }
    }
    resetHistoryPosition();
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

  void Terminal::resetHistoryPosition() { historyPosition_ = -1; }

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
      write(prompt_);
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

} // namespace netd::client