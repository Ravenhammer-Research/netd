/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "netd.h"
#include <libyang/libyang.h>
#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include <time.h>

static debug_level_t current_debug_level = DEBUG_NONE;
static bool debug_mode = false;

/**
 * Custom libyang logger callback
 */
void yang_log_callback(LY_LOG_LEVEL level, const char *msg,
                       const char *data_path, const char *schema_path,
                       uint64_t line) {
  debug_level_t debug_level;

  /* Convert libyang log level to our debug level */
  switch (level) {
  case LY_LLERR:
    debug_level = DEBUG_ERROR;
    break;
  case LY_LLWRN:
    debug_level = DEBUG_WARN;
    break;
  case LY_LLVRB:
    debug_level = DEBUG_INFO;
    break;
  case LY_LLDBG:
    debug_level = DEBUG_DEBUG;
    break;
  default:
    debug_level = DEBUG_INFO;
    break;
  }

  /* Log the message with all available context */
  debug_log(debug_level, "libyang: %s (data: %s, schema: %s, line: %lu)", msg,
            data_path ? data_path : "none", schema_path ? schema_path : "none",
            (unsigned long)line);
}

/**
 * Initialize debug logging
 * @param level Debug level
 */
void debug_init(debug_level_t level) {
  current_debug_level = level;
  debug_mode = (level > DEBUG_NONE);

  if (debug_mode) {
    /* In debug mode, log to stderr */
    fprintf(stderr, "Debug logging initialized at level %d\n", level);
  } else {
    /* In normal mode, initialize syslog */
    openlog("netd", LOG_PID | LOG_CONS, LOG_DAEMON);
  }
}

/**
 * Log message with specified debug level
 * @param level Debug level
 * @param format Format string
 * @param ... Variable arguments
 */
void debug_log(debug_level_t level, const char *format, ...) {
  va_list args;
  char timestamp[64];
  time_t now;
  struct tm *tm_info;

  if (level > current_debug_level) {
    return;
  }

  /* Get timestamp */
  time(&now);
  tm_info = localtime(&now);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

  va_start(args, format);

  if (debug_mode) {
    /* Debug mode: log to stderr with timestamps and levels */
    const char *level_str;
    switch (level) {
    case DEBUG_ERROR:
      level_str = "ERROR";
      break;
    case DEBUG_WARN:
      level_str = "WARN";
      break;
    case DEBUG_INFO:
      level_str = "INFO";
      break;
    case DEBUG_DEBUG:
      level_str = "DEBUG";
      break;
    case DEBUG_TRACE:
      level_str = "TRACE";
      break;
    default:
      level_str = "UNKNOWN";
      break;
    }

    fprintf(stderr, "[%s] [%s] ", timestamp, level_str);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    fflush(stderr);
  } else {
    /* Normal mode: log to syslog */
    int syslog_priority;
    switch (level) {
    case DEBUG_ERROR:
      syslog_priority = LOG_ERR;
      break;
    case DEBUG_WARN:
      syslog_priority = LOG_WARNING;
      break;
    case DEBUG_INFO:
      syslog_priority = LOG_INFO;
      break;
    case DEBUG_DEBUG:
      syslog_priority = LOG_DEBUG;
      break;
    case DEBUG_TRACE:
      syslog_priority = LOG_DEBUG;
      break;
    default:
      syslog_priority = LOG_INFO;
      break;
    }

    vsyslog(syslog_priority, format, args);
  }

  va_end(args);
}