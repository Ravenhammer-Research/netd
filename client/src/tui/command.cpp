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
#include <algorithm>
#include <thread>

namespace netd::client::tui {

  constexpr size_t MAX_HISTORY_SIZE = 10240;

  TUI::TUI() : initialized_(false), prompt_("netc> "), commandHistoryPosition_(-1), scrollOffset_(0), connectionStatus_("Not connected"), debugLevel_(0), destroying_(false), client_(nullptr) {}

  TUI::~TUI() { 
    destroying_ = true;
    cleanup(); 
  }
  bool TUI::initialize() {
    if (initialized_) {
      return true;
    }

    setupCurses();
    configureSignalHandler();
    
    setLoggerInstance(this);
    initializeLogger();
    
    netd::shared::Logger::getInstance().debug("TUI Logger initialized successfully");
    
    initialized_ = true;
    return true;
  }


  void TUI::addToCommandHistory(const std::string &command) {
    if (commandHistory_.empty() || commandHistory_.back() != command) {
      commandHistory_.push_back(command);
      while (commandHistory_.size() > MAX_HISTORY_SIZE) {
        commandHistory_.erase(commandHistory_.begin());
      }
    }
    setHistoryPosition(-1);
  }

  const std::vector<std::string>& TUI::getCommandHistory() const {
    return commandHistory_;
  }

  int TUI::getHistoryPosition() {
    return commandHistoryPosition_;
  }

  void TUI::setHistoryPosition(int position) {
    commandHistoryPosition_ = position;
  }

  void TUI::advanceHistoryPosition() {
    if (commandHistoryPosition_ < static_cast<int>(commandHistory_.size()) - 1) {
      commandHistoryPosition_++;
    }
  }

} // namespace netd::client::tui