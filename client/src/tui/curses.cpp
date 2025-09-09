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

namespace netd::client {

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
      init_pair(1, COLOR_RED, COLOR_BLACK);
      init_pair(2, COLOR_YELLOW, COLOR_BLACK);
      init_pair(3, COLOR_BLUE, COLOR_BLACK);
      init_pair(4, COLOR_GREEN, COLOR_BLACK);
      init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    }
  }

  void TUI::configureSignalHandler() {
    signal(SIGINT, [](int) {
      // Clean exit on Ctrl-C
      exit(0);
    });
  }

  void TUI::initializeScreen() {
    refresh();
  }

  void TUI::enableRawMode() {
    raw();
  }

  void TUI::disableEcho() {
    noecho();
  }

  void TUI::enableKeypad() {
    keypad(stdscr, TRUE);
  }

  void TUI::cleanupScreen() {
    endwin();
  }

  void TUI::cleanup() {
    if (initialized_) {
      cleanupLogger();
      cleanupScreen();
      initialized_ = false;
    }
  }
  
  // Screen refresh
  void TUI::refreshCurses() {
    refresh();
  }

  // Terminal attributes
  void TUI::setAttribute(int attr) {
    attron(attr);
  }

  void TUI::addAttribute(int attr) {
    attron(attr);
  }

  void TUI::removeAttribute(int attr) {
    attroff(attr);
  }

  void TUI::clearCurses() {
    clear();
  }

  void TUI::doUpdateCurses() {
    doupdate();
  }

} // namespace netd::client