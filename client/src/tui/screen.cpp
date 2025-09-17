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
#include <client/include/tui.hpp>
#include <curses.h>
#include <regex>

namespace netd::client::tui {

  // Screen dimension helpers - use getmaxyx directly, no state
  int TUI::getScreenSizeX() {
    int y, x;
    getmaxyx(stdscr, y, x);
    (void)y; // Suppress unused variable warning
    return x;
  }

  int TUI::getScreenSizeY() {
    int y, x;
    getmaxyx(stdscr, y, x);
    (void)x; // Suppress unused variable warning
    return y;
  }

  int TUI::getMaxLines() {
    int screenHeight = getScreenSizeY();
    // Leave room for status bar (1 line) and prompt (1 line)
    return screenHeight - 2;
  }

  void TUI::redrawScreen() {
    clear();
    putStatusBar();
    putMessages();
    putPrompt();
    refresh();
  }

  // Screen clearing
  void TUI::clear() { ::clear(); }

  void TUI::clearToEndOfLine() { clrtoeol(); }

  // Message display - no state, calculate everything on the fly
  void TUI::putMessages() {
    int promptRow = getPromptRow();
    int maxLines = promptRow - 1; // Account for status bar
    int screenWidth = getScreenSizeX();

    if (displayHistory_.empty()) {
      return;
    }

    // Clear all message lines first (starting from line 1, after status bar)
    for (int row = 1; row <= maxLines; row++) {
      move(row, 0);
      clrtoeol();
    }

    // Calculate total lines needed for all messages (newest to oldest)
    std::vector<std::vector<std::string>> all_wrapped_messages;
    int totalLines = 0;

    // Process messages from newest to oldest
    for (int i = static_cast<int>(displayHistory_.size()) - 1; i >= 0; i--) {
      std::vector<std::string> wrapped =
          wrapText(displayHistory_[i], screenWidth);
      all_wrapped_messages.push_back(wrapped);
      totalLines += static_cast<int>(wrapped.size());
    }

    // Limit scroll offset to valid range
    int maxScroll = std::max(0, totalLines - maxLines);
    scrollOffset_ = std::min(scrollOffset_, maxScroll);

    // Display messages from bottom up, accounting for scroll
    int displayRow = 0;
    int linesSkipped = 0;

    for (const auto &wrapped_lines : all_wrapped_messages) {
      for (int j = static_cast<int>(wrapped_lines.size()) - 1; j >= 0; j--) {
        if (linesSkipped < scrollOffset_) {
          linesSkipped++;
          continue;
        }

        if (displayRow >= maxLines) {
          return; // No more space
        }

        int targetRow = maxLines - displayRow; // Account for status bar offset
        move(targetRow, 0);
        clrtoeol();

        // Check if this is a log message and apply colors using regex
        std::string line = wrapped_lines[j];
        // Pattern matches both [timestamp][LEVEL]: and [LEVEL]: formats
        std::regex logPatternWithTimestamp(R"(\[([^\]]+)\]\[([TDIWE])\]:)");
        std::regex logPatternWithoutTimestamp(R"(\[([TDIWE])\]:)");
        std::smatch matches;

        char level = 0;
        if (std::regex_search(line, matches, logPatternWithTimestamp)) {
          // Format: [timestamp][LEVEL]:
          level = matches[2].str()[0];
        } else if (std::regex_search(line, matches,
                                     logPatternWithoutTimestamp)) {
          // Format: [LEVEL]:
          level = matches[1].str()[0];
        }

        if (level != 0) {
          int color = 0;

          switch (level) {
          case 'T':
            color = 12;
            break; // Bright White
          case 'D':
            color = 11;
            break; // Bright Cyan
          case 'I':
            color = 10;
            break; // Bright Green
          case 'W':
            color = 9;
            break; // Bright Yellow
          case 'E':
            color = 8;
            break; // Bright Red
          }

          if (color > 0) {
            // Find the position of the level letter in the original string
            size_t levelPos;
            if (std::regex_search(line, matches, logPatternWithTimestamp)) {
              // Format: [timestamp][LEVEL]: - find position after "]["
              levelPos = line.find("][" + std::string(1, level) + "]:");
              if (levelPos != std::string::npos) {
                levelPos += 2; // Move past "]["
              }
            } else {
              // Format: [LEVEL]: - find position after "["
              levelPos = line.find("[" + std::string(1, level) + "]:");
              if (levelPos != std::string::npos) {
                levelPos += 1; // Move past "["
              }
            }

            if (levelPos != std::string::npos) {
              // Print everything before the level letter
              printw("%s", line.substr(0, levelPos).c_str());

              // Print level letter with color and bold
              attron(COLOR_PAIR(color) | A_BOLD);
              printw("%c", level);
              attroff(COLOR_PAIR(color) | A_BOLD);

              // Print everything after the level letter
              printw("%s", line.substr(levelPos + 1).c_str());
            } else {
              printw("%s", line.c_str());
            }
          } else {
            printw("%s", line.c_str());
          }
        } else {
          printw("%s", line.c_str());
        }

        displayRow++;
      }
    }
  }

  // Utility functions
  void TUI::sleepMs(int ms) { napms(ms); }

  void TUI::resizeTerminal() {
    // Use proper curses resize handling
    resizeterm(0, 0);
    refresh();
    redrawScreen();
  }

  void TUI::handleResize() { resizeTerminal(); }

} // namespace netd::client::tui