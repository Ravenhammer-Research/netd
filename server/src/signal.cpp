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

#include <csignal>
#include <shared/include/logger.hpp>
#include <atomic>

namespace netd::server {

// Global flag for graceful shutdown
static std::atomic<bool> g_running{true};

bool isRunning() {
    return g_running.load();
}

void signalHandler(int signal) {
    auto &logger = netd::shared::Logger::getInstance();
    
    switch (signal) {
    case SIGINT:
        logger.info("Received SIGINT, initiating graceful shutdown...");
        g_running.store(false);
        break;
        
    case SIGTERM:
        logger.info("Received SIGTERM, initiating graceful shutdown...");
        g_running.store(false);
        break;
        
    case SIGHUP:
        logger.info("Received SIGHUP, reloading configuration...");
        // TODO: Implement configuration reload
        break;
        
    case SIGPIPE:
        logger.debug("Received SIGPIPE, ignoring...");
        // Ignore SIGPIPE - this happens when client disconnects
        break;
        
    default:
        logger.warning("Received unknown signal: " + std::to_string(signal));
        break;
    }
}

void setupSignalHandlers() {
    auto &logger = netd::shared::Logger::getInstance();
    
    // Set up signal handlers for graceful shutdown
    if (signal(SIGINT, signalHandler) == SIG_ERR) {
        logger.error("Failed to set SIGINT handler");
    }
    
    if (signal(SIGTERM, signalHandler) == SIG_ERR) {
        logger.error("Failed to set SIGTERM handler");
    }
    
    if (signal(SIGHUP, signalHandler) == SIG_ERR) {
        logger.error("Failed to set SIGHUP handler");
    }
    
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        logger.error("Failed to ignore SIGPIPE");
    }
    
    logger.debug("Signal handlers set up successfully");
}

void cleanupSignalHandlers() {
    auto &logger = netd::shared::Logger::getInstance();
    
    // Restore default signal handlers
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
    
    logger.debug("Signal handlers cleaned up");
}

} // namespace netd::server
