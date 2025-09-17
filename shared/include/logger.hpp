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

#ifndef NETD_LOGGER_HPP
#define NETD_LOGGER_HPP

#include <functional>
#include <memory>
#include <mutex>
#include <shared/include/exception.hpp>
#include <string>

namespace netd::shared {

  // Bitmask-based logging system
  enum class LogMask : uint32_t {
    ERROR = 1U << 0,   // 0x00000001
    WARNING = 1U << 1, // 0x00000002
    INFO = 1U << 2,    // 0x00000004
    DEBUG = 1U << 3,   // 0x00000008
#ifdef HAVE_LLDP
    DEBUG_LLDP = 1U << 4, // 0x00000010
#endif
    DEBUG_YANG = 1U << 5,         // 0x00000020
    DEBUG_YANG_DICT = 1U << 6,    // 0x00000040
    DEBUG_YANG_XPATH = 1U << 7,   // 0x00000080
    DEBUG_YANG_DEPSETS = 1U << 8, // 0x00000100
    DEBUG_TRACE = 1U << 9         // 0x00000200
  };

  // Convenience combinations
  constexpr uint32_t LOG_DEFAULT = static_cast<uint32_t>(LogMask::ERROR) |
                                   static_cast<uint32_t>(LogMask::WARNING) |
                                   static_cast<uint32_t>(LogMask::INFO);

  constexpr uint32_t LOG_DEBUG_ALL =
      static_cast<uint32_t>(LogMask::DEBUG) |
#ifdef HAVE_LLDP
      static_cast<uint32_t>(LogMask::DEBUG_LLDP) |
#endif
      static_cast<uint32_t>(LogMask::DEBUG_YANG);

  constexpr uint32_t LOG_YANG_ALL =
      static_cast<uint32_t>(LogMask::DEBUG_YANG) |
      static_cast<uint32_t>(LogMask::DEBUG_YANG_DICT) |
      static_cast<uint32_t>(LogMask::DEBUG_YANG_XPATH) |
      static_cast<uint32_t>(LogMask::DEBUG_YANG_DEPSETS);

  // Legacy enum for backward compatibility
  enum class LogLevel { TRACE, DEBUG, INFO, WARNING, ERROR, YANG };

  class Logger {
  public:
    using Callback = std::function<void(LogLevel, const std::string &)>;

    static Logger &getInstance();

    void setCallback(Callback callback);
    void log(LogLevel level, const std::string &message);
    void log(LogMask mask, const std::string &message);

    void trace(const netd::shared::NetdError &error);
    void trace(const std::vector<void *> &stackTrace);
    void debug(const std::string &message);
#ifdef HAVE_LLDP
    void debug_lldp(const std::string &message);
#endif
    void debug_yang(const std::string &message);
    void info(const std::string &message);
    void warning(const std::string &message);
    void error(const std::string &message);
    void yang(const std::string &message);

    // New bitmask-based methods
    void setLogMask(uint32_t mask);
    uint32_t getLogMask() const;
    bool isLogEnabled(LogMask mask) const;

    // Legacy methods for backward compatibility
    void setYangDebugGroups(uint32_t groups);
    void setTimestampEnabled(bool enabled);

  private:
    Logger();
    ~Logger() = default;
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    Callback callback_;
    uint32_t currentLogMask_ = LOG_DEFAULT;
    LogLevel currentLogLevel_ = LogLevel::ERROR; // Legacy support
    bool timestampEnabled_ = false;
    mutable std::mutex mutex_;
  };

} // namespace netd::shared

#endif // NETD_LOGGER_HPP
