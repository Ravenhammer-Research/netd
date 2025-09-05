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

#include <iomanip>
#include <iostream>
#include <shared/include/logger.hpp>

namespace netd::shared {

  Logger &Logger::getInstance() {
    static Logger instance;
    if (!instance.callback_) {
      // Set up default console logging
      instance.setCallback([](LogLevel level, const std::string &message) {
        const char *levelStr = "";
        switch (level) {
        case LogLevel::DEBUG:
          levelStr = "DEBUG";
          break;
        case LogLevel::INFO:
          levelStr = "INFO ";
          break;
        case LogLevel::WARNING:
          levelStr = "WARN ";
          break;
        case LogLevel::ERROR:
          levelStr = "ERROR";
          break;
        }
        std::cerr << "[" << levelStr << "] " << message << std::endl;
      });
    }
    return instance;
  }

  void Logger::setCallback(Callback callback) { callback_ = callback; }

  void Logger::log(LogLevel level, const std::string &message) {
    if (callback_) {
      callback_(level, message);
    }
  }

  void Logger::debug(const std::string &message) {
    log(LogLevel::DEBUG, message);
  }

  void Logger::info(const std::string &message) {
    log(LogLevel::INFO, message);
  }

  void Logger::warning(const std::string &message) {
    log(LogLevel::WARNING, message);
  }

  void Logger::error(const std::string &message) {
    log(LogLevel::ERROR, message);
  }

} // namespace netd::shared
