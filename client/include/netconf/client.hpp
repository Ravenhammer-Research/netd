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

#ifndef NETD_CLIENT_NETCONF_CLIENT_HPP
#define NETD_CLIENT_NETCONF_CLIENT_HPP

#include <shared/include/netconf/session.hpp>
#include <shared/include/transport.hpp>
#include <shared/include/exception.hpp>
#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>
#include <functional>


namespace netd::client::netconf {

  class NetconfClient {
  public:
    // Singleton access
    static NetconfClient& getInstance();
    
    // Delete copy constructor and assignment operator
    NetconfClient(const NetconfClient&) = delete;
    NetconfClient& operator=(const NetconfClient&) = delete;

    // Transport management
    void setTransport(std::unique_ptr<netd::shared::BaseTransport> transport);
    netd::shared::BaseTransport* getTransport() const;
    
    // Connection management
    void connect(const std::string& address) noexcept(false);
    void connectWithRetry(const std::string& address, int max_retries = 5, int initial_delay_ms = 1000) noexcept(false);
    void disconnect();
    bool isConnected() const;
    void startAutoReconnect(const std::string& address, int max_retries = 5, int initial_delay_ms = 1000);
    void stopAutoReconnect();
    
    // Communication methods
    void sendData(const std::string& data) noexcept(false);
    std::string receiveData() noexcept(false);
    
    // NETCONF-specific methods
    void sendRpc(std::istream& rpc_stream) noexcept(false);
    std::string receiveRpcReply() noexcept(false);
    
  private:
    NetconfClient();
    ~NetconfClient();
    
    std::unique_ptr<netd::shared::BaseTransport> transport_;
    std::unique_ptr<netd::shared::netconf::NetconfSession> session_;
    std::atomic<bool> connected_;
    std::atomic<bool> auto_reconnect_enabled_;
    std::atomic<bool> should_stop_reconnect_;
    std::thread reconnect_thread_;
    std::string connection_address_;
    int connection_socket_fd_;
    int max_retries_;
    int initial_delay_ms_;
    
    void reconnectLoop();
    void sleepWithBackoff(int attempt, int base_delay_ms);
    void validateTransport() const noexcept(false);
    void validateConnection() const noexcept(false);
  };

} // namespace netd::client::netconf

#endif // NETD_CLIENT_NETCONF_CLIENT_HPP
