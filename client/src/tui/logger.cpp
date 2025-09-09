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
#include <shared/include/logger.hpp>
#include <shared/include/exception.hpp>
#include <string>
#include <memory>
#include <stdexcept>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace netd::client {

  // Global TUI instance for logger callback
  static TUI* g_tui_instance = nullptr;

  // Get current timestamp as string
  std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()) % 1000000000;
    
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(9) << ns.count();
    return oss.str();
  }

  // Custom logger callback function
  void tui_logger_callback(netd::shared::LogLevel level, const std::string& message) {
    if (!g_tui_instance) {
      throw std::runtime_error("TUI instance not set");
    }

    // Format the log message with timestamp and level prefix
    std::string timestamp = getCurrentTimestamp();
    std::string formatted_message;
    switch (level) {
      case netd::shared::LogLevel::TRACE:
        formatted_message = "[" + timestamp + "][T]:" + message;
        break;
      case netd::shared::LogLevel::DEBUG:
        formatted_message = "[" + timestamp + "][D]:" + message;
        break;
      case netd::shared::LogLevel::INFO:
        formatted_message = "[" + timestamp + "][I]:" + message;
        break;
      case netd::shared::LogLevel::WARNING:
        formatted_message = "[" + timestamp + "][W]:" + message;
        break;
      case netd::shared::LogLevel::ERROR:
        formatted_message = "[" + timestamp + "][E]:" + message;
        break;
    }

    // Write to TUI using the appropriate method
    g_tui_instance->putLine(formatted_message);
  }

  // Set the TUI instance for logging
  void TUI::setLoggerInstance(TUI* tui) {
    g_tui_instance = tui;
  }

  // Initialize TUI logger
  void TUI::initializeLogger() {
    if (g_tui_instance) {
      netd::shared::Logger::getInstance().setCallback(tui_logger_callback);
    }
  }

  // Cleanup logger
  void TUI::cleanupLogger() {
    // Clear the global TUI instance reference
    g_tui_instance = nullptr;
  }

} // namespace netd::client
