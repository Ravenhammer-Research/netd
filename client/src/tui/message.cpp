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

namespace netd::client::tui {

  constexpr int MAX_MESSAGES = 1000;

  // Message history management
  void TUI::addToDisplayHistory(const std::string &message) {
    displayHistory_.push_back(message);
    // Trim history if it gets too large
    while (displayHistory_.size() > MAX_MESSAGES) {
      displayHistory_.erase(displayHistory_.begin());
    }
  }

  void TUI::clearDisplayHistory() {
    displayHistory_.clear();
    scrollOffset_ = 0;
  }

  void TUI::removeFromDisplayHistory(size_t index) {
    if (index < displayHistory_.size()) {
      displayHistory_.erase(displayHistory_.begin() + index);
    }
  }

  size_t TUI::getDisplayHistorySize() const {
    return displayHistory_.size();
  }

  const std::string& TUI::getDisplayHistoryAt(size_t index) const {
    return displayHistory_[index];
  }

  const std::vector<std::string>& TUI::getDisplayHistory() const {
    return displayHistory_;
  }

  // Message display functions
  void TUI::putLine(const std::string &text) {
    // Store the original message without any modification
    addToDisplayHistory(text);
    
    // Clear screen and redraw all messages from bottom up
    clearCurses();
    putStatusBar();
    putMessages();
    putPrompt();
    refreshCurses();
  }

  void TUI::putFormattedText(const std::string &format, const std::string &text) {
    (void)format; // Suppress unused parameter warning
    addToDisplayHistory(text);
    redrawScreen();
  }

  // Scroll management
  int TUI::getScrollOffset() {
    return scrollOffset_;
  }

  void TUI::setScrollOffset(int offset) {
    scrollOffset_ = std::max(0, offset);
  }

  void TUI::scrollMessages() {
    // Scroll up (show older messages) - limit to one screen height
    int screenHeight = getScreenSizeY() - 1; // Leave room for prompt
    scrollOffset_ += screenHeight;
  }

  void TUI::scrollMessagesDown() {
    // Scroll down (show newer messages) - limit to one screen height
    int screenHeight = getScreenSizeY() - 1; // Leave room for prompt
    if (scrollOffset_ >= screenHeight) {
      scrollOffset_ -= screenHeight;
    } else {
      scrollOffset_ = 0;
    }
  }


} // namespace netd::client::tui