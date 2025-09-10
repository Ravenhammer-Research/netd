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

#include <server/include/netconf/server.hpp>
#include <server/include/netconf/base.hpp>
#include <shared/include/netconf/unix.hpp>
#include <server/include/signal.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/yang.hpp>
#include <shared/include/netconf/session.hpp>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <signal.h>
#include <algorithm>

namespace netd::server::netconf {

  NetconfServer::NetconfServer(const std::string& socket_path)
      : socket_path_(socket_path), running_(false) {
    rpc_handler_ = std::make_unique<netd::server::netconf::NetconfRpc>();
    transport_ = std::make_unique<netd::shared::netconf::UnixTransport>();
  }

  NetconfServer::~NetconfServer() {
    stop();
  }

  bool NetconfServer::start() {
    auto &logger = netd::shared::Logger::getInstance();

    // Start the transport layer
    if (!transport_->start(socket_path_)) {
      logger.error("Failed to start transport layer");
      return false;
    }

    running_ = true;
    logger.info("NETCONF server started successfully on socket: " + socket_path_);
    
    return true;
  }

  void NetconfServer::stop() {
    if (!running_) {
      return;
    }
        
    auto &logger = netd::shared::Logger::getInstance();
    running_ = false;

    // Close all sessions
    auto& session_manager = netd::shared::netconf::SessionManager::getInstance();
    session_manager.closeAllSessions();

    // Stop transport layer
    transport_->stop();

    // Wait for all session threads to finish
    for (auto& thread : session_threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    session_threads_.clear();

    logger.info("NETCONF server stopped");
  }

  void NetconfServer::run() {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Starting server main loop");

    while (running_ && netd::server::isRunning()) {
      acceptNewSessions();
      cleanupSessions();
      
      // Small sleep to prevent busy waiting
      usleep(10000); // 10ms
    }
    
    logger.info("Server main loop exiting");
  }

  void NetconfServer::acceptNewSessions() {
    int server_socket = transport_->getServerSocket();
    if (server_socket < 0) {
      return;
    }

    // Check for new connections (non-blocking)
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(server_socket, &readfds);
    
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0; // Non-blocking

    int result = select(server_socket + 1, &readfds, nullptr, nullptr, &timeout);
    if (result > 0 && FD_ISSET(server_socket, &readfds)) {
      struct sockaddr_un client_addr;
      socklen_t client_len = sizeof(client_addr);
      
      int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
      if (client_socket >= 0) {
        auto &logger = netd::shared::Logger::getInstance();
        logger.debug("New client connection accepted");
        
        // Start processing thread for this session
        session_threads_.emplace_back([this, client_socket]() {
          ly_ctx* ctx = netd::shared::Yang::getInstance().getContext();
          auto session = std::make_unique<netd::shared::netconf::NetconfSession>(ctx);
          
          processSession(std::move(session), client_socket);
        });
      }
    }
  }

  void NetconfServer::processSession(std::unique_ptr<netd::shared::netconf::NetconfSession> session, int client_socket) {
    auto &logger = netd::shared::Logger::getInstance();
    logger.debug("Processing session " + std::to_string(session->getSessionId()));

    while (session->isConnected() && running_) {
      try {
        // Receive XML message using transport
        std::string xml = transport_->receiveData(client_socket);
        if (xml.empty()) {
          break; // Connection closed
        }

        // Route all messages through handleRpc - it will handle hello, RPC, etc.
        std::string response = rpc_handler_->handleRpc(xml, *session);
        
        // Send response using transport (response is already the complete RPC reply)
        if (!response.empty()) {
          transport_->sendData(client_socket, response);
        }
        
      } catch (const std::exception& e) {
        logger.error("Error processing session: " + std::string(e.what()));
        break;
      }
    }

    // Clean up session and connection
    auto& session_manager = netd::shared::netconf::SessionManager::getInstance();
    session_manager.removeSession(session->getSessionId());
    session->close();
    transport_->closeConnection(client_socket);
    
    logger.debug("Session " + std::to_string(session->getSessionId()) + " ended");
  }

  void NetconfServer::cleanupSessions() {
    // Remove finished threads
    session_threads_.erase(
        std::remove_if(session_threads_.begin(), session_threads_.end(),
                      [](std::thread& t) {
                        return !t.joinable();
                      }),
        session_threads_.end());
  }

} // namespace netd::server::netconf
