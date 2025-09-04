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

#include <shared/include/logger.hpp>
#include <shared/include/request.hpp>
#include <shared/include/response.hpp>
#include <server/include/store.hpp>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>

namespace netd {

class NetconfServer {
public:
    NetconfServer() : socketFd_(-1), running_(false) {}
    
    ~NetconfServer() {
        if (running_) {
            stop();
        }
    }

    bool start(const std::string& socketPath = "/var/run/netd.sock") {
        auto& logger = Logger::getInstance();
        
        // Create Unix domain socket
        socketFd_ = socket(AF_UNIX, SOCK_STREAM, 0);
        if (socketFd_ == -1) {
            logger.error("Failed to create socket: " + std::string(strerror(errno)));
            return false;
        }

        // Set socket options
        int opt = 1;
        if (setsockopt(socketFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            logger.error("Failed to set socket options: " + std::string(strerror(errno)));
            close(socketFd_);
            socketFd_ = -1;
            return false;
        }

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

        // Remove existing socket file if it exists
        unlink(socketPath.c_str());

        if (bind(socketFd_, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            logger.error("Failed to bind socket: " + std::string(strerror(errno)));
            close(socketFd_);
            socketFd_ = -1;
            return false;
        }

        if (listen(socketFd_, 5) == -1) {
            logger.error("Failed to listen on socket: " + std::string(strerror(errno)));
            close(socketFd_);
            socketFd_ = -1;
            return false;
        }

        running_ = true;
        logger.info("NETCONF server started on " + socketPath);
        
        // Start listening thread
        listenThread_ = std::thread(&NetconfServer::listenLoop, this);
        
        return true;
    }

    void stop() {
        if (!running_) return;
        
        running_ = false;
        
        // Close socket to unblock accept
        if (socketFd_ != -1) {
            close(socketFd_);
            socketFd_ = -1;
        }
        
        // Wait for listen thread to finish
        if (listenThread_.joinable()) {
            listenThread_.join();
        }
        
        auto& logger = Logger::getInstance();
        logger.info("NETCONF server stopped");
    }

    bool isRunning() const {
        return running_;
    }

private:
    void listenLoop() {
        auto& logger = Logger::getInstance();
        
        while (running_) {
            struct sockaddr_un clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            
            int clientFd = accept(socketFd_, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if (clientFd == -1) {
                if (running_) {
                    logger.error("Failed to accept connection: " + std::string(strerror(errno)));
                }
                continue;
            }

            logger.info("Client connected");
            
            // Handle client in a separate thread
            std::thread clientThread(&NetconfServer::handleClient, this, clientFd);
            clientThread.detach();
        }
    }

    void handleClient(int clientFd) {
        auto& logger = Logger::getInstance();
        
        try {
            while (running_) {
                // Receive request
                char buffer[4096];
                ssize_t received = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
                if (received <= 0) {
                    break;
                }

                buffer[received] = '\0';
                std::string request(buffer);
                
                logger.debug("Received request: " + request);
                
                // Process request and generate response
                std::string response = processRequest(request);
                
                // Send response
                ssize_t sent = send(clientFd, response.c_str(), response.length(), 0);
                if (sent == -1) {
                    logger.error("Failed to send response: " + std::string(strerror(errno)));
                    break;
                }
            }
        } catch (const std::exception& e) {
            logger.error("Error handling client: " + std::string(e.what()));
        }
        
        close(clientFd);
        logger.info("Client disconnected");
    }

    std::string processRequest(const std::string& request) {
        auto& logger = Logger::getInstance();
        
        // TODO: Implement proper NETCONF request parsing
        // For now, handle basic operations using the configuration store
        
        if (request.find("<get-config>") != std::string::npos) {
            logger.info("Processing get-config request");
            
            // Extract source from request (default to running)
            std::string source = "running";
            if (request.find("<running/>") != std::string::npos) {
                source = "running";
            } else if (request.find("<candidate/>") != std::string::npos) {
                source = "candidate";
            } else if (request.find("<startup/>") != std::string::npos) {
                source = "startup";
            }
            
            // Get configuration from store
            netd::ConfigType configType;
            if (source == "running") {
                configType = netd::ConfigType::RUNNING;
            } else if (source == "candidate") {
                configType = netd::ConfigType::CANDIDATE;
            } else if (source == "startup") {
                configType = netd::ConfigType::STARTUP;
            } else {
                configType = netd::ConfigType::RUNNING;
            }
            
            std::string config = getConfiguration(configType);
            
            std::string response = R"(<?xml version="1.0" encoding="UTF-8"?>
<rpc-reply message-id="1" xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <data>
)";
            response += config;
            response += R"(
  </data>
</rpc-reply>)";
            return response;
            
        } else if (request.find("<edit-config>") != std::string::npos) {
            logger.info("Processing edit-config request");
            
            // TODO: Extract configuration data from request
            // For now, just set a placeholder configuration
            std::string config = R"(<interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces">
  <!-- Configuration data would go here -->
</interfaces>)";
            
            if (setCandidateConfiguration(config)) {
                return R"(<?xml version="1.0" encoding="UTF-8"?>
<rpc-reply message-id="1" xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <ok/>
</rpc-reply>)";
            } else {
                return R"(<?xml version="1.0" encoding="UTF-8"?>
<rpc-reply message-id="1" xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <rpc-error>
    <error-type>application</error-type>
    <error-tag>invalid-value</error-tag>
    <error-severity>error</error-severity>
    <error-message>Invalid configuration</error-message>
  </rpc-error>
</rpc-reply>)";
            }
            
        } else if (request.find("<commit>") != std::string::npos) {
            logger.info("Processing commit request");
            
            if (commitCandidateConfiguration()) {
                return R"(<?xml version="1.0" encoding="UTF-8"?>
<rpc-reply message-id="1" xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <ok/>
</rpc-reply>)";
            } else {
                return R"(<?xml version="1.0" encoding="UTF-8"?>
<rpc-reply message-id="1" xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <rpc-error>
    <error-type>application</error-type>
    <error-tag>operation-failed</error-tag>
    <error-severity>error</error-severity>
    <error-message>Failed to commit configuration</error-message>
  </rpc-error>
</rpc-reply>)";
            }
            
        } else if (request.find("<discard-changes>") != std::string::npos) {
            logger.info("Processing discard-changes request");
            
            if (discardCandidateConfiguration()) {
                return R"(<?xml version="1.0" encoding="UTF-8"?>
<rpc-reply message-id="1" xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <ok/>
</rpc-reply>)";
            } else {
                return R"(<?xml version="1.0" encoding="UTF-8"?>
<rpc-reply message-id="1" xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <rpc-error>
    <error-type>application</error-type>
    <error-tag>operation-failed</error-tag>
    <error-severity>error</error-severity>
    <error-message>Failed to discard changes</error-message>
  </rpc-error>
</rpc-reply>)";
            }
            
        } else {
            logger.warning("Unknown request type");
            return R"(<?xml version="1.0" encoding="UTF-8"?>
<rpc-reply message-id="1" xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <rpc-error>
    <error-type>rpc</error-type>
    <error-tag>unknown-operation</error-tag>
    <error-severity>error</error-severity>
    <error-message>Unknown operation</error-message>
  </rpc-error>
</rpc-reply>)";
        }
    }

    int socketFd_;
    bool running_;
    std::thread listenThread_;
};

// Global NETCONF server instance
static NetconfServer g_netconfServer;

// Public interface functions
bool startNetconfServer(const std::string& socketPath) {
    return g_netconfServer.start(socketPath);
}

void stopNetconfServer() {
    g_netconfServer.stop();
}

bool isNetconfServerRunning() {
    return g_netconfServer.isRunning();
}

} // namespace netd
