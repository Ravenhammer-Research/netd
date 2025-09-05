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

#include <libnetconf2/netconf.h>
#include <libnetconf2/server_config.h>
#include <libnetconf2/session_server.h>
#include <memory>
#include <shared/include/logger.hpp>
#include <string>

namespace netd::server::netconf {

  using netd::shared::Logger;

  class UnixTransport {
  private:
    std::string socketPath_;
    bool listening_;

  public:
    UnixTransport() : listening_(false) {}

    ~UnixTransport() { stop(); }

    bool start(const std::string &socketPath) {
      auto &logger = Logger::getInstance();

      socketPath_ = socketPath;

      // Add Unix socket endpoint
      if (nc_server_add_endpt_unix_socket_listen("netd", socketPath.c_str(),
                                                 0666, -1, -1) != 0) {
        logger.error("Failed to add Unix socket endpoint: " + socketPath);
        return false;
      }

      listening_ = true;
      logger.info("Unix transport started on " + socketPath);
      return true;
    }

    void stop() {
      if (!listening_)
        return;

      auto &logger = Logger::getInstance();
      listening_ = false;
      logger.info("Unix transport stopped");
    }

    bool isListening() const { return listening_; }

    const std::string &getSocketPath() const { return socketPath_; }
  };

} // namespace netd::server::netconf
