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

#include <chrono>
#include <ctime>
#include <execinfo.h>
#include <iomanip>
#include <iostream>
#include <libnetconf2/log.h>
#include <libyang/log.h>
#include <mutex>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#ifdef HAVE_LLDP
#include <lldpctl.h>
#endif

namespace netd::shared {

  // libyang log callback
  void libyang_log_callback(LY_LOG_LEVEL level, const char *msg,
                            const char *data_path, const char *schema_path,
                            uint64_t line) {
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
      std::vector<void *> stackTrace(array, array + size);
      logger.trace(stackTrace);
    }
  }

#ifdef HAVE_LLDP
  // LLDP log callback
  void lldp_log_callback(int severity, const char *msg) {
    auto &logger = Logger::getInstance();
    LogLevel logLevel;

    // LLDP severity levels: 1=ALERT, 2=CRIT, 3=ERR, 4=WARNING, 5=NOTICE,
    // 6=INFO, 7=DEBUG
    switch (severity) {
    case 1:
      logLevel = LogLevel::ERROR; // LOG_ALERT
      break;
    case 2:
      logLevel = LogLevel::ERROR; // LOG_CRIT
      break;
    case 3:
      logLevel = LogLevel::ERROR; // LOG_ERR
      break;
    case 4:
      logLevel = LogLevel::WARNING; // LOG_WARNING
      break;
    case 5:
      logLevel = LogLevel::INFO; // LOG_NOTICE
      break;
    case 6:
      logLevel = LogLevel::INFO; // LOG_INFO
      break;
    case 7:
      logLevel = LogLevel::DEBUG; // LOG_DEBUG
      break;
    default:
      throw netd::shared::ArgumentError("Invalid LLDP severity level");
      break;
    }

    logger.log(logLevel, msg);
  }
#endif

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
      case LogLevel::YANG:
        levelStr = "[Y]: ";
        break;
      }

      if (timestampEnabled_) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;

        std::cerr << std::put_time(std::localtime(&time_t), "%Y%m%d%H%M%S");
        std::cerr << std::setfill('0') << std::setw(3) << ms.count() << " ";
      }

      std::cerr << levelStr << message << std::endl;
    });

    // Set up libyang log callbacks
    ly_set_log_clb(libyang_log_callback);

    // Set default log levels for libyang (will be overridden by main.cpp)

#ifdef HAVE_LLDP
    // Set up LLDP log callbacks
    lldpctl_log_callback(lldp_log_callback);
    // LLDP log level will be set when setLogLevel is called
#endif

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
      std::lock_guard<std::mutex> lock(mutex_);
      callback_(level, message);
    }
  }

  void Logger::log(LogMask mask, const std::string &message) {
    // Only log if the mask is enabled
    if (isLogEnabled(mask) && callback_) {
      std::lock_guard<std::mutex> lock(mutex_);
      // Convert LogMask to LogLevel for callback
      LogLevel level;
      switch (mask) {
      case LogMask::ERROR:
        level = LogLevel::ERROR;
        break;
      case LogMask::WARNING:
        level = LogLevel::WARNING;
        break;
      case LogMask::INFO:
        level = LogLevel::INFO;
        break;
      case LogMask::DEBUG:
#ifdef HAVE_LLDP
      case LogMask::DEBUG_LLDP:
#endif
        level = LogLevel::DEBUG;
        break;
      case LogMask::DEBUG_YANG:
        level = LogLevel::YANG;
        break;
      case LogMask::DEBUG_TRACE:
        level = LogLevel::TRACE;
        break;
      default:
        level = LogLevel::DEBUG;
        break;
      }
      callback_(level, message);
    }
  }

  void Logger::trace(const netd::shared::NetdError &error) {
    log(LogMask::DEBUG_TRACE,
        NetdError::getStackTraceString(error.getStackTrace()));
  }

  void Logger::trace(const std::vector<void *> &stackTrace) {
    log(LogMask::DEBUG_TRACE, NetdError::getStackTraceString(stackTrace));
  }

  void Logger::debug(const std::string &message) {
    log(LogMask::DEBUG, message);
  }

#ifdef HAVE_LLDP
  void Logger::debug_lldp(const std::string &message) {
    log(LogMask::DEBUG_LLDP, message);
  }
#endif

  void Logger::debug_yang(const std::string &message) {
    log(LogMask::DEBUG_YANG, message);
  }

  void Logger::info(const std::string &message) { log(LogMask::INFO, message); }

  void Logger::warning(const std::string &message) {
    log(LogMask::WARNING, message);
  }

  void Logger::error(const std::string &message) {
    log(LogMask::ERROR, message);
  }

  void Logger::yang(const std::string &message) {
    log(LogMask::DEBUG_YANG, message);
  }

  // New bitmask-based methods
  void Logger::setLogMask(uint32_t mask) {
    std::lock_guard<std::mutex> lock(mutex_);
    currentLogMask_ = mask;

    // Update libyang debug groups based on YANG debug flags
    uint32_t yang_groups = 0;
    if (mask & static_cast<uint32_t>(LogMask::DEBUG_YANG_DICT)) {
      yang_groups |= LY_LDGDICT;
    }
    if (mask & static_cast<uint32_t>(LogMask::DEBUG_YANG_XPATH)) {
      yang_groups |= LY_LDGXPATH;
    }
    if (mask & static_cast<uint32_t>(LogMask::DEBUG_YANG_DEPSETS)) {
      yang_groups |= LY_LDGDEPSETS;
    }

    ly_log_dbg_groups(yang_groups);

    // Set libyang log level based on whether any YANG debug is enabled
    if (mask & (static_cast<uint32_t>(LogMask::DEBUG_YANG) |
                static_cast<uint32_t>(LogMask::DEBUG_YANG_DICT) |
                static_cast<uint32_t>(LogMask::DEBUG_YANG_XPATH) |
                static_cast<uint32_t>(LogMask::DEBUG_YANG_DEPSETS))) {
      ly_log_level(LY_LLDBG);
    } else if (mask & static_cast<uint32_t>(LogMask::INFO)) {
      ly_log_level(LY_LLWRN);
    } else {
      ly_log_level(LY_LLERR);
    }

#ifdef HAVE_LLDP
    // Set LLDP log level based on whether LLDP debug is enabled
    if (mask & static_cast<uint32_t>(LogMask::DEBUG_LLDP)) {
      lldpctl_log_level(3); // debug
    } else if (mask & static_cast<uint32_t>(LogMask::DEBUG)) {
      lldpctl_log_level(2); // info
    } else if (mask & static_cast<uint32_t>(LogMask::INFO)) {
      lldpctl_log_level(1); // warnings
    } else {
      lldpctl_log_level(1); // warnings
    }
#endif
  }

  uint32_t Logger::getLogMask() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentLogMask_;
  }

  bool Logger::isLogEnabled(LogMask mask) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return (currentLogMask_ & static_cast<uint32_t>(mask)) != 0;
  }

  void Logger::setYangDebugGroups(uint32_t groups) {
    ly_log_dbg_groups(groups);
  }

  void Logger::setTimestampEnabled(bool enabled) {
    timestampEnabled_ = enabled;
  }

} // namespace netd::shared
