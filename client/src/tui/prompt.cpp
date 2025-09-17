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
#include <curses.h>
#include <string>

namespace netd::client::tui {

  // Character constants
  constexpr char TAB_KEY = '\t';
  constexpr char NEWLINE_KEY = '\n';
  constexpr char CARRIAGE_RETURN_KEY = '\r';
  constexpr char CTRL_D_MARKER = '\x04';
  constexpr char BACKSPACE_KEY = 127;
  constexpr char PRINTABLE_CHAR_MIN = 32;
  constexpr char PRINTABLE_CHAR_MAX = 126;

  // String literals
  constexpr const char *PROMPT_PREFIX = "netc";
  constexpr const char *PROMPT_SUFFIX = "> ";

  // Prompt management
  int TUI::getPromptLength() { return static_cast<int>(prompt_.length()); }

  int TUI::getPromptRow() { return getScreenSizeY() - 1; }

  void TUI::putPrompt() {
    move(getPromptRow(), 0);
    clrtoeol();

    // Print "netc" in bold, then ">" in normal formatting
    attron(A_BOLD);
    printw(PROMPT_PREFIX);
    attroff(A_BOLD);
    printw(PROMPT_SUFFIX);

    refresh();
  }

  void TUI::redrawPrompt() { putPrompt(); }

  // Input line management
  void TUI::putCurrentLine(const std::string &line) {
    move(getPromptRow(), getPromptLength());
    clrtoeol();
    printw("%s", line.c_str());
    refresh();
  }

  void TUI::clearCurrentLine() {
    move(getPromptRow(), getPromptLength());
    clrtoeol();
    refresh();
  }

  // Input reading
  std::string TUI::readLine() {
    if (!initialized_) {
      throw std::runtime_error("TUI not initialized (readLine)");
    }

    std::string line;
    int key;

    while (true) {
      key = scanKeyInput();

      if (key == KEY_RESIZE) {
        handleResize();
        // Redraw the prompt and current input after resize
        putPrompt();
        putCurrentLine(line);
        continue;
      } else if (key == NEWLINE_KEY || key == CARRIAGE_RETURN_KEY) {
        break;
      } else if (key == ERR) {
        // Check if this is EOF by trying to read again with a timeout
        nodelay(stdscr, TRUE);
        int next_key = getch();
        nodelay(stdscr, FALSE);

        if (next_key == ERR) {
          // EOF (Ctrl+D) - return special marker to signal exit
          return std::string(1, CTRL_D_MARKER);
        }
        // Otherwise put the key back and continue
        ungetch(next_key);
        continue;
      } else if (key == KEY_PPAGE) {
        // Page up - scroll back through messages
        scrollMessages();
        redrawScreen();
        putPrompt();
        putCurrentLine(line);
      } else if (key == KEY_NPAGE) {
        // Page down - scroll forward through messages
        scrollMessagesDown();
        redrawScreen();
        putPrompt();
        putCurrentLine(line);
      } else if (key == KEY_BACKSPACE || key == BACKSPACE_KEY) {
        if (!line.empty()) {
          line.pop_back();
          backspaceAtCursor();
        }
      } else if (key == TAB_KEY) {
        // Handle tab completion
        if (!line.empty()) {
          std::string completed = completeCommandContextual(line);
          // Always redraw the line after tab completion to ensure it's visible
          clearCurrentLine();
          line = completed;
          putCurrentLine(line);
        }
      } else if (key >= PRINTABLE_CHAR_MIN && key <= PRINTABLE_CHAR_MAX) {
        line += static_cast<char>(key);
        addch(key);
        refresh();
      }
    }

    return line;
  }

  int TUI::scanKeyInput() { return getch(); }

  std::string TUI::formatReturnValue(bool ctrl_d_exit,
                                     const std::string &result) {
    return ctrl_d_exit ? std::string(1, CTRL_D_MARKER) : result;
  }

  void TUI::handleKeyInput(int key) {
    // Basic key handling - can be expanded for more complex input
    switch (key) {
    case KEY_UP:
      // Handle up arrow for command history
      break;
    case KEY_DOWN:
      // Handle down arrow for command history
      break;
    case KEY_LEFT:
      // Handle left arrow for cursor movement
      break;
    case KEY_RIGHT:
      // Handle right arrow for cursor movement
      break;
    case TAB_KEY: // Tab key
      // Handle tab completion
      break;
    default:
      break;
    }
  }

} // namespace netd::client::tui