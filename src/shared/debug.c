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

#include <debug.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <libyang/log.h>
#include <libnetconf2/log.h>

static debug_level_t current_debug_level = NONE;

/**
 * Initialize debug logging with specified level
 */
void debug_init(debug_level_t level) {
    current_debug_level = level;
    
    /* Initialize libyang logging */
    LY_LOG_LEVEL ly_level;
    switch (level) {
        case ERROR:
            ly_level = LY_LLERR;
            break;
        case WARN:
            ly_level = LY_LLWRN;
            break;
        case INFO:
            ly_level = LY_LLVRB;
            break;
        case DEBUG:
        case DEBUG1:
        case DEBUG2:
            ly_level = LY_LLDBG;
            break;
        default:
            ly_level = LY_LLERR;
            break;
    }
    ly_log_level(ly_level);
    
    /* Initialize libnetconf2 logging */
    NC_VERB_LEVEL nc_level;
    switch (level) {
        case ERROR:
            nc_level = NC_VERB_ERROR;
            break;
        case WARN:
            nc_level = NC_VERB_WARNING;
            break;
        case INFO:
            nc_level = NC_VERB_VERBOSE;
            break;
        case DEBUG:
        case DEBUG1:
        case DEBUG2:
            nc_level = NC_VERB_DEBUG;
            break;
        default:
            nc_level = NC_VERB_ERROR;
            break;
    }
    nc_verbosity(nc_level);
}

/**
 * Log debug message if level is enabled
 */
void debug_log(debug_level_t level, const char *format, ...) {
    if (level > current_debug_level) {
        return;
    }

    time_t now;
    struct tm *tm_info;
    char time_str[26];

    time(&now);
    tm_info = localtime(&now);
    strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    printf("[%s] ", time_str);
    
    switch (level) {
        case ERROR:
            printf("ERROR: ");
            break;
        case WARN:
            printf("WARN:  ");
            break;
        case INFO:
            printf("INFO:  ");
            break;
        case DEBUG:
            printf("DEBUG: ");
            break;
        case DEBUG1:
            printf("DEBUG1: ");
            break;
        case DEBUG2:
            printf("DEBUG2: ");
            break;
        default:
            printf("UNKNOWN: ");
            break;
    }

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

/**
 * libyang logging callback
 */
static void yang_log_callback(LY_LOG_LEVEL level, const char *msg, const char *data_path, const char *schema_path, uint64_t line) {
    debug_level_t debug_level;
    
    switch (level) {
        case LY_LLERR:
            debug_level = ERROR;
            break;
        case LY_LLWRN:
            debug_level = WARN;
            break;
        case LY_LLVRB:
            debug_level = INFO;
            break;
        case LY_LLDBG:
            debug_level = DEBUG;
            break;
        default:
            debug_level = DEBUG;
            break;
    }
    
    debug_log(debug_level, "libyang: %s (path: %s, schema: %s, line: %lu)", 
             msg, data_path ? data_path : "none", schema_path ? schema_path : "none", (unsigned long)line);
}

/**
 * libnetconf2 logging callback
 */
static void netconf_log_callback(const struct nc_session *session, NC_VERB_LEVEL level, const char *msg) {
    debug_level_t debug_level;
    
    switch (level) {
        case NC_VERB_ERROR:
            debug_level = ERROR;
            break;
        case NC_VERB_WARNING:
            debug_level = WARN;
            break;
        case NC_VERB_VERBOSE:
            debug_level = INFO;
            break;
        case NC_VERB_DEBUG:
        case NC_VERB_DEBUG_LOWLVL:
            debug_level = DEBUG;
            break;
        default:
            debug_level = DEBUG;
            break;
    }
    
    debug_log(debug_level, "libnetconf2: %s (session: %p, level: %d)", msg, (void*)session, level);
}

/**
 * Initialize libyang and libnetconf2 logging callbacks
 */
void debug_init_libraries(debug_level_t level) {
    debug_init(level);
    
    /* Set libyang logging callback */
    ly_set_log_clb(yang_log_callback);
    
    /* Set libnetconf2 logging callback */
    nc_set_print_clb_session(netconf_log_callback);
} 