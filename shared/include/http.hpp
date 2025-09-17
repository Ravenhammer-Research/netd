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

#ifndef NETD_SHARED_HTTP_HPP
#define NETD_SHARED_HTTP_HPP

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <regex>
#include <shared/include/transport.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// Forward declarations
namespace netd::shared {
  class QuicTransport;
  struct QuicConnectionId;
} // namespace netd::shared

namespace netd::shared {

  // HTTP Protocol versions
  enum class HttpVersion { HTTP_1_0, HTTP_1_1, HTTP_2_0, HTTP_3_0 };

  // HTTP Request structure
  struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    HttpVersion protocol_version = HttpVersion::HTTP_1_1;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    std::unordered_map<std::string, std::string> query_params;

    // HTTP/2 specific fields
    int stream_id = 0;
    bool end_stream = false;
    std::unordered_map<std::string, std::string> pseudo_headers;

    // HTTP/3 specific fields
    uint64_t connection_id = 0;
    bool is_push_promise = false;
  };

  // HTTP Response structure
  struct HttpResponse {
    int status_code = 200;
    std::string status_message = "OK";
    HttpVersion protocol_version = HttpVersion::HTTP_1_1;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    // HTTP/2 specific fields
    int stream_id = 0;
    bool end_stream = false;
    std::unordered_map<std::string, std::string> pseudo_headers;

    // HTTP/3 specific fields
    uint64_t connection_id = 0;
    bool is_push_promise = false;

    HttpResponse() {
      headers["Content-Type"] = "text/plain";
      headers["Server"] = "netd-http/2.0";
    }
  };

  // Route handler function type
  using RouteHandler = std::function<HttpResponse(const HttpRequest &)>;

  // Route specification
  struct Route {
    std::string method;
    std::string path_pattern;
    RouteHandler handler;
    std::regex compiled_pattern;

    Route(const std::string &method, const std::string &path_pattern,
          RouteHandler handler)
        : method(method), path_pattern(path_pattern),
          handler(std::move(handler)) {
      // Convert path pattern to regex (simple implementation)
      std::string regex_pattern = path_pattern;
      // Replace :param with ([^/]+) for path parameters
      regex_pattern =
          std::regex_replace(regex_pattern, std::regex(":([^/]+)"), "([^/]+)");
      // Escape other regex special characters
      regex_pattern =
          std::regex_replace(regex_pattern, std::regex("\\*"), ".*");
      regex_pattern = "^" + regex_pattern + "$";
      compiled_pattern = std::regex(regex_pattern);
    }
  };

  class HTTPTransport : public BaseTransport {
  private:
    std::string address_;
    int port_;
    std::atomic<bool> listening_;
    std::atomic<bool> should_stop_;

    // Threading
    std::vector<std::thread> worker_threads_;
    std::mutex routes_mutex_;
    std::mutex server_mutex_;

    // Routes
    std::vector<Route> routes_;

    // Server socket
    int server_socket_ = -1;

    // Protocol support flags
    bool http2_enabled_ = true;
    bool http3_enabled_ = true;

    // HTTP/2 settings
    std::unordered_map<std::string, std::string> http2_settings_;

    // HTTP/3 settings
    std::unordered_map<std::string, std::string> http3_settings_;

    // Configuration
    static constexpr size_t MAX_WORKER_THREADS = 10;
    static constexpr size_t MAX_REQUEST_SIZE = 8192;

    // Helper methods
    bool setupServerSocket();
    void acceptConnections();
    void handleClient(int client_socket);
    HttpRequest parseRequest(const std::string &raw_request);
    HttpResponse processRequest(const HttpRequest &request);
    std::string generateResponse(const HttpResponse &response);
    void sendResponse(int client_socket, const HttpResponse &response);
    bool matchesRoute(const Route &route, const std::string &method,
                      const std::string &path);
    std::unordered_map<std::string, std::string>
    extractPathParams(const Route &route, const std::string &path);

    // Protocol detection and negotiation
    HttpVersion detectProtocol(const std::string &raw_request);
    bool supportsHttp2() const;
    bool supportsHttp3() const;

    // HTTP/2 specific methods
    HttpRequest parseHttp2Request(const std::string &raw_request);
    std::string generateHttp2Response(const HttpResponse &response);
    bool handleHttp2Connection(int client_socket);
    bool sendHttp2Frame(int client_socket, const std::string &frame_data);

    // HTTP/3 specific methods (delegates to QUIC)
    HttpRequest parseHttp3Request(const std::string &raw_request);
    std::string generateHttp3Response(const HttpResponse &response);
    bool handleHttp3Connection(int client_socket);

  public:
    HTTPTransport();
    ~HTTPTransport();

    // BaseTransport interface
    bool start(const std::string &address) override;
    void stop() override;
    bool isListening() const override;
    int acceptConnection() override;
    void closeConnection(int socket_fd) override;
    bool connect(const std::string &address) override;
    void disconnect() override;
    int getSocket() const override;
    bool sendData(int socket_fd, const std::string &data) override;
    std::string receiveData(int socket_fd) override;

    // Cancellation support
    void cancelOperation(int socket_fd) override;

    const std::string &getAddress() const override;

    // HTTP-specific interface
    bool start(const std::string &address, int port);
    int getPort() const;

    // Route management
    void addRoute(const std::string &method, const std::string &path,
                  RouteHandler handler);
    void get(const std::string &path, RouteHandler handler);
    void post(const std::string &path, RouteHandler handler);
    void put(const std::string &path, RouteHandler handler);
    void del(const std::string &path, RouteHandler handler);
    void patch(const std::string &path, RouteHandler handler);

    // Utility methods
    void setMaxThreads(size_t max_threads);
    size_t getActiveThreads() const;

    // Protocol configuration
    void enableHttp2(bool enable = true);
    void enableHttp3(bool enable = true);
    void setHttp2Settings(
        const std::unordered_map<std::string, std::string> &settings);
    void setHttp3Settings(
        const std::unordered_map<std::string, std::string> &settings);

    // Protocol information
    bool isHttp2Enabled() const;
    bool isHttp3Enabled() const;
    std::vector<HttpVersion> getSupportedProtocols() const;
  };

  // HTTP Client class
  class HTTPClient {
  private:
    // Connection settings
    std::string host_;
    int port_;
    bool use_ssl_;
    HttpVersion preferred_version_;

    // Connection state
    int socket_fd_ = -1;
    bool connected_ = false;

    // HTTP/2 specific
    int next_stream_id_ = 1;
    std::unordered_map<int, std::string> stream_responses_;

    // HTTP/3 specific (QUIC)
    std::unique_ptr<QuicTransport> quic_transport_;
    std::unique_ptr<QuicConnectionId> quic_connection_id_;

    // Configuration
    uint32_t timeout_ms_ = 30000; // 30 seconds
    size_t max_redirects_ = 5;
    bool follow_redirects_ = true;
    std::unordered_map<std::string, std::string> default_headers_;

    // Helper methods
    bool connect();
    bool sendRequest(const HttpRequest &request);
    HttpResponse receiveResponse();
    HttpResponse
    makeRequest(const std::string &method, const std::string &path,
                const std::unordered_map<std::string, std::string> &headers,
                const std::string &body = "");

    // Protocol-specific methods
    HttpResponse makeHttp1Request(
        const std::string &method, const std::string &path,
        const std::unordered_map<std::string, std::string> &headers,
        const std::string &body);
    HttpResponse makeHttp2Request(
        const std::string &method, const std::string &path,
        const std::unordered_map<std::string, std::string> &headers,
        const std::string &body);
    HttpResponse makeHttp3Request(
        const std::string &method, const std::string &path,
        const std::unordered_map<std::string, std::string> &headers,
        const std::string &body);

    // HTTP/2 specific
    bool setupHttp2Connection();
    int getNextStreamId();
    bool sendHttp2Frame(const std::string &frame_data);

    // HTTP/3 specific
    bool setupHttp3Connection();
    bool sendHttp3Request(const HttpRequest &request);
    HttpResponse receiveHttp3Response();

    // Utility methods
    std::string buildRequestLine(const std::string &method,
                                 const std::string &path, HttpVersion version);
    std::string
    buildHeaders(const std::unordered_map<std::string, std::string> &headers);
    HttpResponse parseResponse(const std::string &raw_response);
    std::string urlEncode(const std::string &str);
    std::string urlDecode(const std::string &str);

  public:
    HTTPClient();
    ~HTTPClient();

    // Connection management
    bool connect(const std::string &host, int port, bool use_ssl = false);
    void disconnect();
    bool isConnected() const;

    // Protocol configuration
    void setPreferredVersion(HttpVersion version);
    void setTimeout(uint32_t timeout_ms);
    void setMaxRedirects(size_t max_redirects);
    void setFollowRedirects(bool follow);
    void setDefaultHeader(const std::string &name, const std::string &value);
    void setDefaultHeaders(
        const std::unordered_map<std::string, std::string> &headers);

    // HTTP methods
    HttpResponse
    get(const std::string &path,
        const std::unordered_map<std::string, std::string> &headers = {});
    HttpResponse
    post(const std::string &path, const std::string &body,
         const std::unordered_map<std::string, std::string> &headers = {});
    HttpResponse
    put(const std::string &path, const std::string &body,
        const std::unordered_map<std::string, std::string> &headers = {});
    HttpResponse
    del(const std::string &path,
        const std::unordered_map<std::string, std::string> &headers = {});
    HttpResponse
    patch(const std::string &path, const std::string &body,
          const std::unordered_map<std::string, std::string> &headers = {});

    // Advanced methods
    HttpResponse
    request(const std::string &method, const std::string &path,
            const std::unordered_map<std::string, std::string> &headers = {},
            const std::string &body = "");

    // Utility methods
    std::string getHost() const;
    int getPort() const;
    bool isSSL() const;
    HttpVersion getPreferredVersion() const;
    uint32_t getTimeout() const;
    size_t getMaxRedirects() const;
    bool getFollowRedirects() const;
  };

} // namespace netd::shared

#endif // NETD_SHARED_HTTP_HPP
