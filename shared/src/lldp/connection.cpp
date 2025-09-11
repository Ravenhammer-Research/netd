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

#include <shared/include/lldp/connection.hpp>
#include <shared/include/logger.hpp>
#include <unistd.h>
#include <errno.h>
#include <cstring>

namespace netd::shared::lldp {

Connection::Connection() 
    : connection_(nullptr)
    , initialized_(false)
{
}

Connection::~Connection() {
    cleanup();
}

bool Connection::initialize() {
    auto& logger = Logger::getInstance();
    
    // Check if lldpd socket exists and is accessible
    const char* lldpd_socket = "/var/run/lldpd.socket";
    if (access(lldpd_socket, R_OK | W_OK) != 0) {
        if (errno == ENOENT) {
            logger.error("lldpd socket not found at " + std::string(lldpd_socket) + 
                        " - make sure lldpd daemon is running");
        } else if (errno == EACCES) {
            logger.error("Permission denied accessing " + std::string(lldpd_socket) + 
                        " - try running with sudo or check lldpd permissions");
        } else {
            logger.error("Cannot access " + std::string(lldpd_socket) + 
                        " (" + std::string(strerror(errno)) + ")");
        }
        return false;
    }
    
    try {
        // Create connection with default callbacks (NULL parameters)
        connection_ = lldpctl_new(nullptr, nullptr, nullptr);
        if (!connection_) {
            logger.warning("Failed to create LLDP connection");
            return false;
        }

        initialized_ = true;
        logger.info("LLDP connection initialized successfully");
        return true;
    } catch (const std::exception& e) {
        logger.error("Exception during LLDP connection initialization: " + std::string(e.what()));
        return false;
    }
}

void Connection::cleanup() {
    if (connection_) {
        lldpctl_release(connection_);
        connection_ = nullptr;
    }
    initialized_ = false;
}

} // namespace netd::shared::lldp
