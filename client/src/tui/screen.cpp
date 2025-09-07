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
#include <algorithm>

namespace netd::client {

  // Screen dimension helpers
  int TUI::getScreenSizeX() {
    return COLS;
  }

  int TUI::getScreenSizeY() {
    return LINES;
  }

  int TUI::getMaxLines() {
    return getScreenSizeY() - 1; // Leave room for prompt
  }


  void TUI::redrawScreen() {
    clear();
    putMessages();
    putPrompt();
    refresh();
  }

  // Screen clearing
  void TUI::clear() {
    ::clear();
  }

  void TUI::clearToEndOfLine() {
    clrtoeol();
  }

  // Message display
  void TUI::putMessages() {
    int promptRow = getPromptRow();
    int maxLines = promptRow;
    
    int startIdx = std::max(0, static_cast<int>(displayHistory_.size()) - maxLines + scrollOffset_);
    int endIdx = static_cast<int>(displayHistory_.size());
    
    // Display messages from bottom to top
    for (int i = startIdx; i < endIdx && (i - startIdx) < maxLines; i++) {
      int row = promptRow - 1 - (i - startIdx); // Start from bottom and work up
      move(row, 0);
      clrtoeol();
      printw("%s", displayHistory_[i].c_str());
    }
  }


  // Utility functions
  void TUI::sleepMs(int ms) {
    napms(ms);
  }

  void TUI::resizeTerminal() {
    endwin();
    refresh();
    redrawScreen();
  }

  void TUI::handleResize() {
    resizeTerminal();
  }

} // namespace netd::client
