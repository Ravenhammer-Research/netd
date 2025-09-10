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
#include <chrono>
#include <ctime>
#include <shared/include/logger.hpp>
#include <shared/include/exception.hpp>
#include <libyang/log.h>
#include <libnetconf2/log.h>
#include <execinfo.h>

namespace netd::shared {

  // libyang log callback
  void libyang_log_callback(LY_LOG_LEVEL level, const char *msg, const char *data_path, const char *schema_path, uint64_t line) {
    auto &logger = Logger::getInstance();
    LogLevel logLevel;
    
    // Check if this is a directory-related error that should be debug level
    std::string message = std::string(msg);
    if (message.find("Unable to use search directory") != std::string::npos) {
      logLevel = LogLevel::DEBUG;
    } else {
      switch (level) {
      case LY_LLERR:
        logLevel = LogLevel::ERROR;
        break;
      case LY_LLWRN:
        logLevel = LogLevel::WARNING;
        break;
      case LY_LLVRB:
      case LY_LLDBG:
        logLevel = LogLevel::YANG;
        break;
      default:
        logLevel = LogLevel::INFO;
        break;
      }
    }
    
    if (data_path) {
      message += " (data: " + std::string(data_path) + ")";
    }
    if (schema_path) {
      message += " (schema: " + std::string(schema_path) + ")";
    }
    if (line > 0) {
      message += " (line: " + std::to_string(line) + ")";
    }

    logger.log(logLevel, message);
    
    // Capture and print stack trace for errors
    if (level == LY_LLERR) {
      void *array[20];
      size_t size = backtrace(array, 20);
      std::vector<void*> stackTrace(array, array + size);
      logger.trace(netd::shared::NetdException::getStackTraceString(stackTrace));
    }
  }


  Logger::Logger() {
    // Set up default console logging
    setCallback([this](LogLevel level, const std::string &message) {
      const char *levelStr = "UNK";
      switch (level) {
      case LogLevel::TRACE:
        levelStr = "[T]: ";
        break;
      case LogLevel::DEBUG:
        levelStr = "[D]: ";
        break;
      case LogLevel::INFO:
        levelStr = "[I]: ";
        break;
      case LogLevel::WARNING:
        levelStr = "[W]: ";
        break;
      case LogLevel::ERROR:
        levelStr = "[E]: ";
        break;
      case LogLevel::NETCONF:
        levelStr = "[N]: ";
        break;
      case LogLevel::YANG:
        levelStr = "[Y]: ";
        break;
      }
      
      if (timestampEnabled_) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::cerr << std::put_time(std::localtime(&time_t), "%Y%m%d%H%M%S");
        std::cerr << std::setfill('0') << std::setw(3) << ms.count() << " ";
      }
      
      std::cerr << levelStr << message << std::endl;
    });
    
    // Set up libyang log callbacks
    ly_set_log_clb(libyang_log_callback);
    
    // Set default log levels for libyang (will be overridden by main.cpp)
    ly_log_level(LY_LLERR);  // Default to error only
    ly_log_dbg_groups(0);  // No debug groups by default
    ly_log_options(LY_LOLOG | LY_LOSTORE);  // Log messages and store all errors/warnings
    
    // libssh verbosity removed - no longer using libnetconf2
  }

  Logger &Logger::getInstance() {
    static Logger instance;
    return instance;
  }

  void Logger::setCallback(Callback callback) { callback_ = callback; }

  void Logger::log(LogLevel level, const std::string &message) {
    // Only log if the message level is at or above the current log level
    if (level >= currentLogLevel_ && callback_) {
      callback_(level, message);
    }
  }

  void Logger::trace(const std::string &message) {
    log(LogLevel::TRACE, message);
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

  void Logger::netconf(const std::string &message) {
    log(LogLevel::NETCONF, message);
  }

  void Logger::yang(const std::string &message) {
    log(LogLevel::YANG, message);
  }

  void Logger::setLogLevel(LogLevel level) {
    currentLogLevel_ = level;
  }

  void Logger::setYangDebugGroups(uint32_t groups) {
    ly_log_dbg_groups(groups);
  }

  void Logger::setTimestampEnabled(bool enabled) {
    timestampEnabled_ = enabled;
  }

} // namespace netd::shared
