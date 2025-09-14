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
#include <client/include/processor.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/exception.hpp>
#include <curses.h>
#include <thread>

namespace netd::client::tui {

  void TUI::runInteractive(std::function<bool(const std::string &)> commandHandler) {
    if (!initialized_) {
      throw std::runtime_error("TUI not initialized (runInteractive)");
    }

    commandHandler_ = commandHandler;
    redrawScreen();

    std::string line;
    while (true) {
      int key = getch();
      if (key == KEY_RESIZE) {
        handleResize();
        continue;
      }
      
      ungetch(key);
      
      putPrompt();
      line = readLine();
      
      if (line.empty()) {
        continue;
      }
      
      if (line == "\x04") {
        break;
      }
      
      addToCommandHistory(line);
      
      if (commandHandler_) {
        int promptRow = getPromptRow();
        move(promptRow, 0);
        clrtoeol();
        refresh();
        
        try {
          if (!commandHandler_(line)) {
            break;
          }
        } catch (const netd::shared::NetdError &e) {
          putLine("Error: " + std::string(e.what()));
          
          netd::shared::Logger::getInstance().trace(e);
        } 
      }
    }
  }

} // namespace netd::client::tui
