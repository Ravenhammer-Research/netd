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

#ifndef NETD_SERVER_NETCONF_SCTP_HPP
#define NETD_SERVER_NETCONF_SCTP_HPP

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <vector>
#include <shared/include/transport.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

namespace netd::shared {

  enum class SCTPMode {
    SERVER,
    CLIENT
  };


  struct SCTPAddress {
    std::string address;
    int port;
    bool is_primary;
    
    SCTPAddress(const std::string& addr = "", int p = 0, bool primary = false)
      : address(addr), port(p), is_primary(primary) {}
  };

  struct SCTPConnection {
    int socket_fd;
    struct sockaddr_in address;
    std::string peer_address;
    uint16_t peer_port;
    bool connected;
    std::vector<SCTPAddress> local_addresses;
    std::vector<SCTPAddress> remote_addresses;
    
    SCTPConnection() : socket_fd(-1), peer_port(0), connected(false) {
      memset(&address, 0, sizeof(address));
    }
  };

  class SCTPTransport : public BaseTransport {
  private:
    SCTPMode mode_;
    std::vector<SCTPAddress> local_addresses_;
    std::string primary_address_;
    int port_;
    std::atomic<bool> listening_;
    std::atomic<bool> running_;
    
    int server_socket_;
    std::vector<std::unique_ptr<SCTPConnection>> connections_;
    mutable std::mutex connections_mutex_;
    
    std::unique_ptr<std::thread> accept_thread_;

    bool setupServerSocket();
    bool setupClientSocket();
    void acceptConnections();
    void handleConnection(std::unique_ptr<SCTPConnection> conn);
    bool sendMessage(int socket_fd, const std::string& message);
    std::string receiveMessage(int socket_fd);
    void closeConnection(std::unique_ptr<SCTPConnection> conn);
    
    // Multihoming support
    bool addLocalAddress(const std::string& address, int port, bool is_primary = false);
    bool setupMultihomedSocket();
    bool bindToAllAddresses();

  public:
    SCTPTransport(SCTPMode mode = SCTPMode::SERVER);
    ~SCTPTransport();

    // BaseTransport interface implementation
    bool start(const std::string& address) override;
    void stop() override;
    bool isListening() const override;
    int acceptConnection() override;
    void closeConnection(int socket_fd) override;
    bool sendData(int socket_fd, const std::string& data) override;
    std::string receiveData(int socket_fd) override;
    const std::string& getAddress() const override;
    
    // Additional SCTP-specific methods
    bool start(const std::string &address, int port);
    bool isRunning() const;
    int getPort() const;
    std::vector<std::string> getConnectedPeers() const;
    
    // Multihoming methods
    bool addLocalAddress(const std::string& address, bool is_primary = false);
    bool removeLocalAddress(const std::string& address);
    std::vector<SCTPAddress> getLocalAddresses() const;
    std::vector<SCTPAddress> getRemoteAddresses(int socket_fd) const;
    
    // SCTP-AUTH methods (for use with DTLS wrapper)
    bool enableSCTPAuth();
    bool setSCTPAuthKey(const std::string& key);
    
    // Stream management (RFC 6083)
    bool sendDataOnStream(int socket_fd, const std::string& data, uint16_t stream_id, bool ordered = true);
    std::string receiveDataFromStream(int socket_fd, uint16_t& stream_id, bool& ordered);
  };

} // namespace netd::shared

#endif // NETD_SERVER_NETCONF_SCTP_HPP
