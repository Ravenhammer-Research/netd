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
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>

namespace netd {

class NetconfClient {
public:
    NetconfClient() : socketFd_(-1), connected_(false) {}
    
    ~NetconfClient() {
        if (connected_) {
            disconnect();
        }
    }

    bool connect(const std::string& socketPath = "/var/run/netd.sock") {
        auto& logger = Logger::getInstance();
        
        // Create Unix domain socket
        socketFd_ = socket(AF_UNIX, SOCK_STREAM, 0);
        if (socketFd_ == -1) {
            logger.error("Failed to create socket: " + std::string(strerror(errno)));
            return false;
        }

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

        if (::connect(socketFd_, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            logger.error("Failed to connect to " + socketPath + ": " + std::string(strerror(errno)));
            close(socketFd_);
            socketFd_ = -1;
            return false;
        }

        connected_ = true;
        logger.info("Connected to NETD server at " + socketPath);
        return true;
    }

    void disconnect() {
        if (socketFd_ != -1) {
            close(socketFd_);
            socketFd_ = -1;
        }
        connected_ = false;
    }

    bool isConnected() const {
        return connected_;
    }

    // Send a NETCONF request and receive response
    std::string sendRequest(const std::string& request) {
        if (!connected_) {
            throw std::runtime_error("Not connected to server");
        }

        auto& logger = Logger::getInstance();
        
        // Send request
        ssize_t sent = send(socketFd_, request.c_str(), request.length(), 0);
        if (sent == -1) {
            logger.error("Failed to send request: " + std::string(strerror(errno)));
            throw std::runtime_error("Failed to send request");
        }

        // Receive response
        char buffer[4096];
        ssize_t received = recv(socketFd_, buffer, sizeof(buffer) - 1, 0);
        if (received == -1) {
            logger.error("Failed to receive response: " + std::string(strerror(errno)));
            throw std::runtime_error("Failed to receive response");
        }

        buffer[received] = '\0';
        std::string response(buffer);
        
        logger.debug("Received response: " + response);
        return response;
    }

    // Convenience methods for common NETCONF operations
    std::string getConfig(const std::string& source = "running") {
        GetConfigRequest request(source);
        std::string netconfRequest = request.toNetconfRequest();
        return sendRequest(netconfRequest);
    }

    std::string editConfig(const std::string& target = "candidate", const std::string& config = "") {
        EditConfigRequest request(target, config);
        std::string netconfRequest = request.toNetconfRequest();
        return sendRequest(netconfRequest);
    }

    std::string commit() {
        CommitRequest request;
        std::string netconfRequest = request.toNetconfRequest();
        return sendRequest(netconfRequest);
    }

private:
    int socketFd_;
    bool connected_;
};

// Global NETCONF client instance
static NetconfClient g_netconfClient;

// Public interface functions
bool connectToServer(const std::string& socketPath) {
    return g_netconfClient.connect(socketPath);
}

void disconnectFromServer() {
    g_netconfClient.disconnect();
}

bool isConnectedToServer() {
    return g_netconfClient.isConnected();
}

std::string sendNetconfRequest(const std::string& request) {
    return g_netconfClient.sendRequest(request);
}

std::string getConfig(const std::string& source) {
    return g_netconfClient.getConfig(source);
}

std::string editConfig(const std::string& target, const std::string& config) {
    return g_netconfClient.editConfig(target, config);
}

std::string commit() {
    return g_netconfClient.commit();
}

} // namespace netd
