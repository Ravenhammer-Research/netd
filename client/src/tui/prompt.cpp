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

namespace netd::client {

  // Prompt management
  int TUI::getPromptLength() {
    return static_cast<int>(prompt_.length());
  }

  int TUI::getPromptRow() {
    return getScreenSizeY() - 1;
  }

  void TUI::putPrompt() {
    move(getPromptRow(), 0);
    clrtoeol();
    printw("%s", prompt_.c_str());
    refresh();
  }

  void TUI::redrawPrompt() {
    putPrompt();
  }

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
      
      if (key == '\n' || key == '\r') {
        break;
      } else if (key == ERR) {
        // Check if this is EOF by trying to read again with a timeout
        nodelay(stdscr, TRUE);
        int next_key = getch();
        nodelay(stdscr, FALSE);
        
        if (next_key == ERR) {
          // EOF (Ctrl+D) - return special marker to signal exit
          return "\x04"; // CTRL_D_MARKER
        }
        // Otherwise put the key back and continue
        ungetch(next_key);
        continue;
      } else if (key == KEY_BACKSPACE || key == 127) {
        if (!line.empty()) {
          line.pop_back();
          backspaceAtCursor();
        }
      } else if (key >= 32 && key <= 126) {
        line += static_cast<char>(key);
        addch(key);
        refresh();
      }
    }
    
    return line;
  }

  int TUI::scanKeyInput() {
    return getch();
  }

  std::string TUI::formatReturnValue(bool ctrl_d_exit, const std::string &result) {
    return ctrl_d_exit ? std::string(1, 0x04) : result; // 0x04 is CTRL_D_MARKER
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
      default:
        break;
    }
  }

} // namespace netd::client
