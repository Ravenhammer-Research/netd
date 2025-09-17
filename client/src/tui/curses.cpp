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
#include <signal.h>
#include <unistd.h>

namespace netd::client::tui {

  // Terminal initialization and cleanup
  void TUI::setupCurses() {
    initscr();
    setupColors();
    enableRawMode();
    disableEcho();
    enableKeypad();
    initializeScreen();
  }

  void TUI::setupColors() {
    if (has_colors()) {
      start_color();
      // Use bright colors for better visibility
      init_pair(1, COLOR_RED, COLOR_BLACK);     // Error - Red
      init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Warning - Yellow
      init_pair(3, COLOR_BLUE, COLOR_BLACK);    // Blue
      init_pair(4, COLOR_GREEN, COLOR_BLACK);   // Info - Green
      init_pair(5, COLOR_MAGENTA, COLOR_BLACK); // Magenta
      init_pair(6, COLOR_WHITE, COLOR_BLACK);   // Trace - White
      init_pair(7, COLOR_CYAN, COLOR_BLACK);    // Debug - Cyan

      // Try to use bright color variants if available
      // Some terminals support bright colors (8-15) instead of regular (0-7)
      init_pair(8, COLOR_RED + 8, COLOR_BLACK);    // Bright red for errors
      init_pair(9, COLOR_YELLOW + 8, COLOR_BLACK); // Bright yellow for warnings
      init_pair(10, COLOR_GREEN + 8, COLOR_BLACK); // Bright green for info
      init_pair(11, COLOR_CYAN + 8, COLOR_BLACK);  // Bright cyan for debug
      init_pair(12, COLOR_WHITE + 8, COLOR_BLACK); // Bright white for trace
    }
  }

  void TUI::configureSignalHandler() {
    signal(SIGINT, [](int) {
      // Clean exit on Ctrl-C
      exit(0);
    });
  }

  void TUI::initializeScreen() { refresh(); }

  void TUI::enableRawMode() { raw(); }

  void TUI::disableEcho() { noecho(); }

  void TUI::enableKeypad() { keypad(stdscr, TRUE); }

  void TUI::cleanupScreen() { endwin(); }

  void TUI::cleanup() {
    if (initialized_) {
      cleanupLogger();
      cleanupScreen();
      initialized_ = false;
    }
  }

  // Screen refresh
  void TUI::refreshCurses() { refresh(); }

  // Terminal attributes
  void TUI::setAttribute(int attr) { attron(attr); }

  void TUI::addAttribute(int attr) { attron(attr); }

  void TUI::removeAttribute(int attr) { attroff(attr); }

  void TUI::clearCurses() { clear(); }

  void TUI::doUpdateCurses() { doupdate(); }

} // namespace netd::client::tui