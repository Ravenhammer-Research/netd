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
        logLevel = LogLevel::DEBUG;
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

  // libnetconf2 log callback
  void libnetconf2_log_callback(const struct nc_session * /* session */, NC_VERB_LEVEL level, const char *msg) {
    auto &logger = Logger::getInstance();

    switch (level) {
    case NC_VERB_ERROR:
    logger.log(LogLevel::ERROR, std::string(msg));
      break;
    case NC_VERB_WARNING:
      logger.log(LogLevel::WARNING, std::string(msg));
      break;
    case NC_VERB_VERBOSE:
    case NC_VERB_DEBUG:
    case NC_VERB_DEBUG_LOWLVL:
      logger.log(LogLevel::DEBUG, std::string(msg));
      break;
    default:
      logger.log(LogLevel::INFO, std::string(msg));
      break;
    }

    // Capture and print stack trace for errors
    if (level == NC_VERB_ERROR) {
      void *array[20];
      size_t size = backtrace(array, 20);
      std::vector<void*> stackTrace(array, array + size);
      logger.trace(netd::shared::NetdException::getStackTraceString(stackTrace));
    }
  }

  Logger::Logger() {
    // Set up default console logging
    setCallback([](LogLevel level, const std::string &message) {
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
      }
      
      std::cerr << levelStr << message << std::endl;
    });
    
    // Set up libyang and libnetconf2 log callbacks
    ly_set_log_clb(libyang_log_callback);
    nc_set_print_clb_session(libnetconf2_log_callback);
    
    // Set log levels for libyang and libnetconf2
    ly_log_level(LY_LLDBG);  // Enable all libyang messages including debug
    nc_verbosity(NC_VERB_DEBUG);  // Enable all libnetconf2 messages including debug
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

  void Logger::setLogLevel(LogLevel level) {
    currentLogLevel_ = level;
  }

} // namespace netd::shared
