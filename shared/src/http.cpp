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

#include <shared/include/http.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/quic.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>

namespace netd::shared {

  HTTPTransport::HTTPTransport() 
    : address_(""), port_(0), listening_(false), should_stop_(false) {
    // Initialize default HTTP/2 settings
    http2_settings_["header_table_size"] = "4096";
    http2_settings_["enable_push"] = "1";
    http2_settings_["max_concurrent_streams"] = "100";
    http2_settings_["initial_window_size"] = "65535";
    http2_settings_["max_frame_size"] = "16384";
    http2_settings_["max_header_list_size"] = "8192";
    
    // Initialize default HTTP/3 settings
    http3_settings_["max_field_section_size"] = "8192";
    http3_settings_["qpack_max_table_capacity"] = "4096";
    http3_settings_["qpack_blocked_streams"] = "100";
  }

  HTTPTransport::~HTTPTransport() { 
    stop(); 
  }

  // BaseTransport interface implementation
  bool HTTPTransport::start(const std::string& address) {
    return start(address, 80); // Default to port 80
  }

  bool HTTPTransport::start(const std::string &address, int port) {
    std::lock_guard<std::mutex> lock(server_mutex_);
    
    if (listening_) {
      return false; // Already running
    }
    
    address_ = address;
    port_ = port;
    
    if (!setupServerSocket()) {
      return false;
    }
    
    listening_ = true;
    should_stop_ = false;
    
    // Start worker threads
    for (size_t i = 0; i < MAX_WORKER_THREADS; ++i) {
      worker_threads_.emplace_back([this]() {
        while (!should_stop_) {
          // Accept connections in a loop
          struct sockaddr_in client_addr;
          socklen_t client_len = sizeof(client_addr);
          
          int client_socket = accept(server_socket_, 
                                   reinterpret_cast<struct sockaddr*>(&client_addr), 
                                   &client_len);
          
          if (client_socket >= 0) {
            handleClient(client_socket);
            close(client_socket);
          }
        }
      });
    }
    
    return true;
  }

  void HTTPTransport::stop() {
    std::lock_guard<std::mutex> lock(server_mutex_);
    
    if (!listening_) {
      return;
    }
    
    should_stop_ = true;
    listening_ = false;
    
    // Close server socket to wake up accept() calls
    if (server_socket_ >= 0) {
      close(server_socket_);
      server_socket_ = -1;
    }
    
    // Wait for all worker threads to finish
    for (auto& thread : worker_threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    worker_threads_.clear();
  }

  bool HTTPTransport::isListening() const { 
    return listening_; 
  }

  const std::string &HTTPTransport::getAddress() const { 
    return address_; 
  }

  int HTTPTransport::getPort() const {
    return port_; 
  }

  int HTTPTransport::acceptConnection() {
    // HTTP transport doesn't use traditional accept() - it handles connections in worker threads
    // This method is required by BaseTransport but not used in HTTP context
    return -1; // Not applicable for HTTP
  }

  void HTTPTransport::closeConnection(int socket_fd) {
    if (socket_fd >= 0) {
      close(socket_fd);
    }
  }

  bool HTTPTransport::sendData(int socket_fd, const std::string& data) {
    if (socket_fd < 0) {
      return false;
    }
    
    ssize_t bytes_sent = send(socket_fd, data.c_str(), data.length(), 0);
    return bytes_sent == static_cast<ssize_t>(data.length());
  }

  std::string HTTPTransport::receiveData(int socket_fd) {
    if (socket_fd < 0) {
      return "";
    }
    
    char buffer[MAX_REQUEST_SIZE];
    ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0) {
      return "";
    }
    
    buffer[bytes_received] = '\0';
    return std::string(buffer);
  }

  bool HTTPTransport::setupServerSocket() {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
      return false;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }
    
    // Set non-blocking
    int flags = fcntl(server_socket_, F_GETFL, 0);
    fcntl(server_socket_, F_SETFL, flags | O_NONBLOCK);
    
    // Bind to address and port
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    
    if (address_.empty() || address_ == "0.0.0.0") {
      server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
      if (inet_pton(AF_INET, address_.c_str(), &server_addr.sin_addr) <= 0) {
        close(server_socket_);
        server_socket_ = -1;
        return false;
      }
    }
    
    if (bind(server_socket_, reinterpret_cast<struct sockaddr*>(&server_addr), 
             sizeof(server_addr)) < 0) {
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }
    
    if (listen(server_socket_, 128) < 0) {
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }
    
    return true;
  }

  void HTTPTransport::handleClient(int client_socket) {
    char buffer[MAX_REQUEST_SIZE];
    std::string request_data;
    
    // Read the request
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
      return;
    }
    
    buffer[bytes_read] = '\0';
    request_data = buffer;
    
    // Detect protocol version
    HttpVersion protocol = detectProtocol(request_data);
    
    // Parse and process the request based on protocol
    HttpRequest request;
    HttpResponse response;
    
    switch (protocol) {
      case HttpVersion::HTTP_2_0:
        request = parseHttp2Request(request_data);
        response = processRequest(request);
        response.protocol_version = HttpVersion::HTTP_2_0;
        break;
      case HttpVersion::HTTP_3_0:
        request = parseHttp3Request(request_data);
        response = processRequest(request);
        response.protocol_version = HttpVersion::HTTP_3_0;
        break;
      default: // HTTP/1.0 or HTTP/1.1
        request = parseRequest(request_data);
        response = processRequest(request);
        response.protocol_version = protocol;
        break;
    }
    
    // Send the response
    sendResponse(client_socket, response);
  }

  HttpRequest HTTPTransport::parseRequest(const std::string& raw_request) {
    HttpRequest request;
    std::istringstream stream(raw_request);
    std::string line;
    
    // Parse request line
    if (std::getline(stream, line)) {
      std::istringstream request_line(line);
      request_line >> request.method >> request.path >> request.version;
      
      // Remove trailing \r
      if (!request.version.empty() && request.version.back() == '\r') {
        request.version.pop_back();
      }
    }
    
    // Parse headers
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
      size_t colon_pos = line.find(':');
      if (colon_pos != std::string::npos) {
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t\r") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t\r") + 1);
        
        request.headers[key] = value;
      }
    }
    
    // Parse query parameters
    size_t query_pos = request.path.find('?');
    if (query_pos != std::string::npos) {
      std::string query_string = request.path.substr(query_pos + 1);
      request.path = request.path.substr(0, query_pos);
      
      std::istringstream query_stream(query_string);
      std::string param;
      while (std::getline(query_stream, param, '&')) {
        size_t equal_pos = param.find('=');
        if (equal_pos != std::string::npos) {
          std::string key = param.substr(0, equal_pos);
          std::string value = param.substr(equal_pos + 1);
          request.query_params[key] = value;
        }
      }
    }
    
    return request;
  }

  HttpResponse HTTPTransport::processRequest(const HttpRequest& request) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    
    // Find matching route
    for (const auto& route : routes_) {
      if (matchesRoute(route, request.method, request.path)) {
        try {
          return route.handler(request);
        } catch (const std::exception& e) {
          HttpResponse error_response;
          error_response.status_code = 500;
          error_response.status_message = "Internal Server Error";
          error_response.body = "Internal Server Error: " + std::string(e.what());
          return error_response;
        }
      }
    }
    
    // No route found
    HttpResponse not_found;
    not_found.status_code = 404;
    not_found.status_message = "Not Found";
    not_found.body = "Route not found: " + request.method + " " + request.path;
    return not_found;
  }

  std::string HTTPTransport::generateResponse(const HttpResponse& response) {
    std::ostringstream oss;
    
    oss << "HTTP/1.1 " << response.status_code << " " << response.status_message << "\r\n";
    
    for (const auto& header : response.headers) {
      oss << header.first << ": " << header.second << "\r\n";
    }
    
    oss << "Content-Length: " << response.body.length() << "\r\n";
    oss << "\r\n";
    oss << response.body;
    
    return oss.str();
  }

  void HTTPTransport::sendResponse(int client_socket, const HttpResponse& response) {
    std::string response_str = generateResponse(response);
    send(client_socket, response_str.c_str(), response_str.length(), 0);
  }

  bool HTTPTransport::matchesRoute(const Route& route, const std::string& method, const std::string& path) {
    if (route.method != method) {
      return false;
    }
    
    return std::regex_match(path, route.compiled_pattern);
  }

  std::unordered_map<std::string, std::string> HTTPTransport::extractPathParams(
      const Route& route, const std::string& path) {
    std::unordered_map<std::string, std::string> params;
    std::smatch matches;
    
    if (std::regex_match(path, matches, route.compiled_pattern)) {
      // Extract parameter names from the original pattern
      std::regex param_regex(":([^/]+)");
      std::sregex_iterator param_begin(route.path_pattern.begin(), route.path_pattern.end(), param_regex);
      std::sregex_iterator param_end;
      
      size_t match_index = 1; // First match is the whole string
      for (auto it = param_begin; it != param_end && match_index < matches.size(); ++it, ++match_index) {
        params[(*it)[1].str()] = matches[match_index].str();
      }
    }
    
    return params;
  }

  // Route management methods
  void HTTPTransport::addRoute(const std::string& method, const std::string& path, RouteHandler handler) {
    std::lock_guard<std::mutex> lock(routes_mutex_);
    routes_.emplace_back(method, path, std::move(handler));
  }

  void HTTPTransport::get(const std::string& path, RouteHandler handler) {
    addRoute("GET", path, std::move(handler));
  }

  void HTTPTransport::post(const std::string& path, RouteHandler handler) {
    addRoute("POST", path, std::move(handler));
  }

  void HTTPTransport::put(const std::string& path, RouteHandler handler) {
    addRoute("PUT", path, std::move(handler));
  }

  void HTTPTransport::del(const std::string& path, RouteHandler handler) {
    addRoute("DELETE", path, std::move(handler));
  }

  void HTTPTransport::patch(const std::string& path, RouteHandler handler) {
    addRoute("PATCH", path, std::move(handler));
  }

  void HTTPTransport::setMaxThreads(size_t max_threads) {
    // This would require restarting the server to take effect
    // For now, we use the compile-time constant MAX_WORKER_THREADS
    (void)max_threads; // Suppress unused parameter warning
  }

  size_t HTTPTransport::getActiveThreads() const {
    return worker_threads_.size();
  }

  // Protocol detection and negotiation
  HttpVersion HTTPTransport::detectProtocol(const std::string& raw_request) {
    // Check for HTTP/2 connection preface
    if (raw_request.substr(0, 24) == "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n") {
      return HttpVersion::HTTP_2_0;
    }
    
    // Check for HTTP/3 (would be detected at QUIC level)
    // For now, we'll detect it by checking for QUIC connection
    if (raw_request.find("HTTP/3") != std::string::npos) {
      return HttpVersion::HTTP_3_0;
    }
    
    // Check HTTP version in request line
    std::istringstream stream(raw_request);
    std::string line;
    if (std::getline(stream, line)) {
      if (line.find("HTTP/1.0") != std::string::npos) {
        return HttpVersion::HTTP_1_0;
      } else if (line.find("HTTP/1.1") != std::string::npos) {
        return HttpVersion::HTTP_1_1;
      }
    }
    
    // Default to HTTP/1.1
    return HttpVersion::HTTP_1_1;
  }

  bool HTTPTransport::supportsHttp2() const {
    return http2_enabled_;
  }

  bool HTTPTransport::supportsHttp3() const {
    return http3_enabled_;
  }

  // HTTP/2 specific methods
  HttpRequest HTTPTransport::parseHttp2Request(const std::string& raw_request) {
    (void)raw_request;
    
    HttpRequest request;
    
    // Simplified HTTP/2 parsing
    // In a real implementation, this would parse HPACK headers and HTTP/2 frames
    
    // For now, extract basic information
    request.protocol_version = HttpVersion::HTTP_2_0;
    request.method = "GET"; // Would be extracted from :method pseudo-header
    request.path = "/"; // Would be extracted from :path pseudo-header
    request.version = "HTTP/2.0";
    
    // Parse pseudo-headers (simplified)
    request.pseudo_headers[":method"] = "GET";
    request.pseudo_headers[":path"] = "/";
    request.pseudo_headers[":scheme"] = "https";
    request.pseudo_headers[":authority"] = "localhost";
    
    return request;
  }

  std::string HTTPTransport::generateHttp2Response(const HttpResponse& response) {
    // Simplified HTTP/2 response generation
    // In a real implementation, this would generate HPACK headers and HTTP/2 frames
    
    std::ostringstream oss;
    
    // HTTP/2 status line (simplified)
    oss << "HTTP/2.0 " << response.status_code << " " << response.status_message << "\r\n";
    
    // Headers (would be HPACK encoded in real implementation)
    for (const auto& header : response.headers) {
      oss << header.first << ": " << header.second << "\r\n";
    }
    
    oss << "Content-Length: " << response.body.length() << "\r\n";
    oss << "\r\n";
    oss << response.body;
    
    return oss.str();
  }

  bool HTTPTransport::handleHttp2Connection(int client_socket) {
    // Simplified HTTP/2 connection handling
    // In a real implementation, this would handle HTTP/2 frames, streams, etc.
    
    auto& logger = Logger::getInstance();
    logger.info("Handling HTTP/2 connection on socket " + std::to_string(client_socket));
    
    return true;
  }

  bool HTTPTransport::sendHttp2Frame(int client_socket, const std::string& frame_data) {
    // Simplified HTTP/2 frame sending
    // In a real implementation, this would send proper HTTP/2 frames
    
    return send(client_socket, frame_data.c_str(), frame_data.length(), 0) > 0;
  }

  // HTTP/3 specific methods (delegates to QUIC)
  HttpRequest HTTPTransport::parseHttp3Request(const std::string& raw_request) {
    (void)raw_request;
    
    HttpRequest request;
    
    // Simplified HTTP/3 parsing
    // In a real implementation, this would parse QPACK headers and HTTP/3 frames
    
    request.protocol_version = HttpVersion::HTTP_3_0;
    request.method = "GET"; // Would be extracted from :method pseudo-header
    request.path = "/"; // Would be extracted from :path pseudo-header
    request.version = "HTTP/3.0";
    
    // Parse pseudo-headers (simplified)
    request.pseudo_headers[":method"] = "GET";
    request.pseudo_headers[":path"] = "/";
    request.pseudo_headers[":scheme"] = "https";
    request.pseudo_headers[":authority"] = "localhost";
    
    return request;
  }

  std::string HTTPTransport::generateHttp3Response(const HttpResponse& response) {
    // Simplified HTTP/3 response generation
    // In a real implementation, this would generate QPACK headers and HTTP/3 frames
    
    std::ostringstream oss;
    
    // HTTP/3 status line (simplified)
    oss << "HTTP/3.0 " << response.status_code << " " << response.status_message << "\r\n";
    
    // Headers (would be QPACK encoded in real implementation)
    for (const auto& header : response.headers) {
      oss << header.first << ": " << header.second << "\r\n";
    }
    
    oss << "Content-Length: " << response.body.length() << "\r\n";
    oss << "\r\n";
    oss << response.body;
    
    return oss.str();
  }

  bool HTTPTransport::handleHttp3Connection(int client_socket) {
    // Simplified HTTP/3 connection handling
    // In a real implementation, this would handle QUIC streams and HTTP/3 frames
    
    auto& logger = Logger::getInstance();
    logger.info("Handling HTTP/3 connection on socket " + std::to_string(client_socket));
    
    return true;
  }

  // Protocol configuration methods
  void HTTPTransport::enableHttp2(bool enable) {
    http2_enabled_ = enable;
  }

  void HTTPTransport::enableHttp3(bool enable) {
    http3_enabled_ = enable;
  }

  void HTTPTransport::setHttp2Settings(const std::unordered_map<std::string, std::string>& settings) {
    http2_settings_ = settings;
  }

  void HTTPTransport::setHttp3Settings(const std::unordered_map<std::string, std::string>& settings) {
    http3_settings_ = settings;
  }

  bool HTTPTransport::isHttp2Enabled() const {
    return http2_enabled_;
  }

  bool HTTPTransport::isHttp3Enabled() const {
    return http3_enabled_;
  }

  std::vector<HttpVersion> HTTPTransport::getSupportedProtocols() const {
    std::vector<HttpVersion> protocols = {HttpVersion::HTTP_1_0, HttpVersion::HTTP_1_1};
    
    if (http2_enabled_) {
      protocols.push_back(HttpVersion::HTTP_2_0);
    }
    
    if (http3_enabled_) {
      protocols.push_back(HttpVersion::HTTP_3_0);
    }
    
    return protocols;
  }

  // HTTP Client Implementation
  HTTPClient::HTTPClient() 
    : host_(""), port_(80), use_ssl_(false), preferred_version_(HttpVersion::HTTP_1_1) {
    // Set default headers
    default_headers_["User-Agent"] = "netd-http-client/1.0";
    default_headers_["Accept"] = "*/*";
    default_headers_["Connection"] = "keep-alive";
  }

  HTTPClient::~HTTPClient() {
    disconnect();
  }

  bool HTTPClient::connect(const std::string& host, int port, bool use_ssl) {
    host_ = host;
    port_ = port;
    use_ssl_ = use_ssl;
    
    return connect();
  }

  void HTTPClient::disconnect() {
    if (connected_) {
      ::close(socket_fd_);
      socket_fd_ = -1;
      connected_ = false;
    }
    
    if (quic_transport_) {
      quic_transport_->stop();
      quic_transport_.reset();
    }
  }

  bool HTTPClient::isConnected() const {
    return connected_;
  }

  bool HTTPClient::connect() {
    if (connected_) {
      return true;
    }
    
    // Create socket
    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
      return false;
    }
    
    // Set up server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    
    if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
      ::close(socket_fd_);
      socket_fd_ = -1;
      return false;
    }
    
    // Connect to server
    if (::connect(socket_fd_, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
      ::close(socket_fd_);
      socket_fd_ = -1;
      return false;
    }
    
    connected_ = true;
    
    // Setup protocol-specific connection
    switch (preferred_version_) {
      case HttpVersion::HTTP_2_0:
        return setupHttp2Connection();
      case HttpVersion::HTTP_3_0:
        return setupHttp3Connection();
      default:
        return true; // HTTP/1.1 doesn't need special setup
    }
  }

  void HTTPClient::setPreferredVersion(HttpVersion version) {
    preferred_version_ = version;
  }

  void HTTPClient::setTimeout(uint32_t timeout_ms) {
    timeout_ms_ = timeout_ms;
  }

  void HTTPClient::setMaxRedirects(size_t max_redirects) {
    max_redirects_ = max_redirects;
  }

  void HTTPClient::setFollowRedirects(bool follow) {
    follow_redirects_ = follow;
  }

  void HTTPClient::setDefaultHeader(const std::string& name, const std::string& value) {
    default_headers_[name] = value;
  }

  void HTTPClient::setDefaultHeaders(const std::unordered_map<std::string, std::string>& headers) {
    default_headers_ = headers;
  }

  HttpResponse HTTPClient::get(const std::string& path, 
                              const std::unordered_map<std::string, std::string>& headers) {
    return makeRequest("GET", path, headers);
  }

  HttpResponse HTTPClient::post(const std::string& path, const std::string& body,
                               const std::unordered_map<std::string, std::string>& headers) {
    return makeRequest("POST", path, headers, body);
  }

  HttpResponse HTTPClient::put(const std::string& path, const std::string& body,
                              const std::unordered_map<std::string, std::string>& headers) {
    return makeRequest("PUT", path, headers, body);
  }

  HttpResponse HTTPClient::del(const std::string& path,
                              const std::unordered_map<std::string, std::string>& headers) {
    return makeRequest("DELETE", path, headers);
  }

  HttpResponse HTTPClient::patch(const std::string& path, const std::string& body,
                                const std::unordered_map<std::string, std::string>& headers) {
    return makeRequest("PATCH", path, headers, body);
  }

  HttpResponse HTTPClient::request(const std::string& method, const std::string& path,
                                  const std::unordered_map<std::string, std::string>& headers,
                                  const std::string& body) {
    return makeRequest(method, path, headers, body);
  }

  HttpResponse HTTPClient::makeRequest(const std::string& method, const std::string& path, 
                                      const std::unordered_map<std::string, std::string>& headers,
                                      const std::string& body) {
    if (!connected_ && !connect()) {
      HttpResponse error_response;
      error_response.status_code = 0;
      error_response.status_message = "Connection failed";
      error_response.body = "Failed to connect to " + host_ + ":" + std::to_string(port_);
      return error_response;
    }
    
    // Merge headers with defaults
    std::unordered_map<std::string, std::string> merged_headers = default_headers_;
    for (const auto& header : headers) {
      merged_headers[header.first] = header.second;
    }
    
    // Add Content-Length if body is provided
    if (!body.empty() && merged_headers.find("Content-Length") == merged_headers.end()) {
      merged_headers["Content-Length"] = std::to_string(body.length());
    }
    
    // Make request based on protocol version
    HttpResponse response;
    switch (preferred_version_) {
      case HttpVersion::HTTP_2_0:
        response = makeHttp2Request(method, path, merged_headers, body);
        break;
      case HttpVersion::HTTP_3_0:
        response = makeHttp3Request(method, path, merged_headers, body);
        break;
      default:
        response = makeHttp1Request(method, path, merged_headers, body);
        break;
    }
    
    // Handle redirects
    if (follow_redirects_ && (response.status_code == 301 || response.status_code == 302 || 
                             response.status_code == 303 || response.status_code == 307 || 
                             response.status_code == 308)) {
      auto location_it = response.headers.find("Location");
      if (location_it != response.headers.end()) {
        // Parse redirect URL and follow it
        // This is a simplified implementation
        std::string location = location_it->second;
        if (location.find("http://") == 0 || location.find("https://") == 0) {
          // Absolute URL - would need to parse and reconnect
          // For now, just return the redirect response
        } else {
          // Relative URL - make request to new path
          return makeRequest(method, location, headers, body);
        }
      }
    }
    
    return response;
  }

  HttpResponse HTTPClient::makeHttp1Request(const std::string& method, const std::string& path,
                                           const std::unordered_map<std::string, std::string>& headers,
                                           const std::string& body) {
    // Build HTTP/1.1 request
    std::string request_line = buildRequestLine(method, path, HttpVersion::HTTP_1_1);
    std::string headers_str = buildHeaders(headers);
    
    std::string full_request = request_line + "\r\n" + headers_str + "\r\n" + body;
    
    // Send request
    if (send(socket_fd_, full_request.c_str(), full_request.length(), 0) < 0) {
      HttpResponse error_response;
      error_response.status_code = 0;
      error_response.status_message = "Send failed";
      return error_response;
    }
    
    // Receive response
    return receiveResponse();
  }

  HttpResponse HTTPClient::makeHttp2Request(const std::string& method, const std::string& path,
                                           const std::unordered_map<std::string, std::string>& headers,
                                           const std::string& body) {
    // Simplified HTTP/2 request
    // In a real implementation, this would create proper HTTP/2 frames
    
    HttpRequest request;
    request.method = method;
    request.path = path;
    request.protocol_version = HttpVersion::HTTP_2_0;
    request.stream_id = getNextStreamId();
    request.headers = headers;
    request.body = body;
    
    // Add pseudo-headers
    request.pseudo_headers[":method"] = method;
    request.pseudo_headers[":path"] = path;
    request.pseudo_headers[":scheme"] = use_ssl_ ? "https" : "http";
    request.pseudo_headers[":authority"] = host_ + ":" + std::to_string(port_);
    
    // Send HTTP/2 request (simplified)
    std::string frame_data = "HTTP/2.0 " + method + " " + path + "\r\n";
    for (const auto& header : headers) {
      frame_data += header.first + ": " + header.second + "\r\n";
    }
    frame_data += "\r\n" + body;
    
    if (!sendHttp2Frame(frame_data)) {
      HttpResponse error_response;
      error_response.status_code = 0;
      error_response.status_message = "HTTP/2 send failed";
      return error_response;
    }
    
    // Receive HTTP/2 response
    return receiveResponse();
  }

  HttpResponse HTTPClient::makeHttp3Request(const std::string& method, const std::string& path,
                                           const std::unordered_map<std::string, std::string>& headers,
                                           const std::string& body) {
    // Simplified HTTP/3 request
    // In a real implementation, this would use QUIC streams
    
    HttpRequest request;
    request.method = method;
    request.path = path;
    request.protocol_version = HttpVersion::HTTP_3_0;
    request.headers = headers;
    request.body = body;
    
    // Add pseudo-headers
    request.pseudo_headers[":method"] = method;
    request.pseudo_headers[":path"] = path;
    request.pseudo_headers[":scheme"] = use_ssl_ ? "https" : "http";
    request.pseudo_headers[":authority"] = host_ + ":" + std::to_string(port_);
    
    if (!sendHttp3Request(request)) {
      HttpResponse error_response;
      error_response.status_code = 0;
      error_response.status_message = "HTTP/3 send failed";
      return error_response;
    }
    
    return receiveHttp3Response();
  }

  bool HTTPClient::setupHttp2Connection() {
    // Send HTTP/2 connection preface
    std::string preface = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
    if (send(socket_fd_, preface.c_str(), preface.length(), 0) < 0) {
      return false;
    }
    
    // Send SETTINGS frame (simplified)
    std::string settings = "HTTP/2.0 SETTINGS\r\n\r\n";
    return send(socket_fd_, settings.c_str(), settings.length(), 0) > 0;
  }

  int HTTPClient::getNextStreamId() {
    return next_stream_id_ += 2; // Client-initiated streams are odd
  }

  bool HTTPClient::sendHttp2Frame(const std::string& frame_data) {
    return send(socket_fd_, frame_data.c_str(), frame_data.length(), 0) > 0;
  }

  bool HTTPClient::setupHttp3Connection() {
    // Setup QUIC transport for HTTP/3
    quic_transport_ = std::make_unique<QuicTransport>();
    
    if (!quic_transport_->start("0.0.0.0", 0)) { // Use any available port
      return false;
    }
    
    // Generate connection ID
    auto connection_ids = quic_transport_->getConnectionIds();
    quic_connection_id_ = std::make_unique<QuicConnectionId>(
      connection_ids.empty() ? QuicConnectionId{} : connection_ids[0]
    );
    
    return true;
  }

  bool HTTPClient::sendHttp3Request(const HttpRequest& request) {
    if (!quic_transport_) {
      return false;
    }
    
    // Convert request to bytes and send via QUIC
    std::string request_data = request.method + " " + request.path + " HTTP/3.0\r\n";
    for (const auto& header : request.headers) {
      request_data += header.first + ": " + header.second + "\r\n";
    }
    request_data += "\r\n" + request.body;
    
    std::vector<uint8_t> data(request_data.begin(), request_data.end());
    return quic_transport_->sendData(*quic_connection_id_, 0, data);
  }

  HttpResponse HTTPClient::receiveHttp3Response() {
    if (!quic_transport_) {
      HttpResponse error_response;
      error_response.status_code = 0;
      error_response.status_message = "No QUIC transport";
      return error_response;
    }
    
    // Receive data from QUIC stream
    auto data = quic_transport_->receiveData(*quic_connection_id_, 0);
    std::string response_str(data.begin(), data.end());
    
    return parseResponse(response_str);
  }

  HttpResponse HTTPClient::receiveResponse() {
    char buffer[8192];
    std::string response_data;
    
    // Receive response data
    ssize_t bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
      HttpResponse error_response;
      error_response.status_code = 0;
      error_response.status_message = "Receive failed";
      return error_response;
    }
    
    buffer[bytes_received] = '\0';
    response_data = buffer;
    
    return parseResponse(response_data);
  }

  std::string HTTPClient::buildRequestLine(const std::string& method, const std::string& path, HttpVersion version) {
    std::string version_str;
    switch (version) {
      case HttpVersion::HTTP_1_0:
        version_str = "HTTP/1.0";
        break;
      case HttpVersion::HTTP_1_1:
        version_str = "HTTP/1.1";
        break;
      case HttpVersion::HTTP_2_0:
        version_str = "HTTP/2.0";
        break;
      case HttpVersion::HTTP_3_0:
        version_str = "HTTP/3.0";
        break;
    }
    
    return method + " " + path + " " + version_str;
  }

  std::string HTTPClient::buildHeaders(const std::unordered_map<std::string, std::string>& headers) {
    std::string headers_str;
    for (const auto& header : headers) {
      headers_str += header.first + ": " + header.second + "\r\n";
    }
    return headers_str;
  }

  HttpResponse HTTPClient::parseResponse(const std::string& raw_response) {
    HttpResponse response;
    std::istringstream stream(raw_response);
    std::string line;
    
    // Parse status line
    if (std::getline(stream, line)) {
      std::istringstream status_line(line);
      std::string version, status_code_str, status_message;
      status_line >> version >> status_code_str >> status_message;
      
      response.status_code = std::stoi(status_code_str);
      response.status_message = status_message;
      
      // Determine protocol version
      if (version == "HTTP/1.0") {
        response.protocol_version = HttpVersion::HTTP_1_0;
      } else if (version == "HTTP/1.1") {
        response.protocol_version = HttpVersion::HTTP_1_1;
      } else if (version == "HTTP/2.0") {
        response.protocol_version = HttpVersion::HTTP_2_0;
      } else if (version == "HTTP/3.0") {
        response.protocol_version = HttpVersion::HTTP_3_0;
      }
    }
    
    // Parse headers
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
      size_t colon_pos = line.find(':');
      if (colon_pos != std::string::npos) {
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t\r") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t\r") + 1);
        
        response.headers[key] = value;
      }
    }
    
    // Read body
    std::ostringstream body_stream;
    while (std::getline(stream, line)) {
      body_stream << line << "\n";
    }
    response.body = body_stream.str();
    
    // Remove trailing newline
    if (!response.body.empty() && response.body.back() == '\n') {
      response.body.pop_back();
    }
    
    return response;
  }

  std::string HTTPClient::urlEncode(const std::string& str) {
    std::ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex;
    
    for (char c : str) {
      if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
        encoded << c;
      } else {
        encoded << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
      }
    }
    
    return encoded.str();
  }

  std::string HTTPClient::urlDecode(const std::string& str) {
    std::ostringstream decoded;
    
    for (size_t i = 0; i < str.length(); ++i) {
      if (str[i] == '%' && i + 2 < str.length()) {
        std::string hex = str.substr(i + 1, 2);
        char c = static_cast<char>(std::stoi(hex, nullptr, 16));
        decoded << c;
        i += 2;
      } else if (str[i] == '+') {
        decoded << ' ';
      } else {
        decoded << str[i];
      }
    }
    
    return decoded.str();
  }

  // Utility methods
  std::string HTTPClient::getHost() const {
    return host_;
  }

  int HTTPClient::getPort() const {
    return port_;
  }

  bool HTTPClient::isSSL() const {
    return use_ssl_;
  }

  HttpVersion HTTPClient::getPreferredVersion() const {
    return preferred_version_;
  }

  uint32_t HTTPClient::getTimeout() const {
    return timeout_ms_;
  }

  size_t HTTPClient::getMaxRedirects() const {
    return max_redirects_;
  }

  bool HTTPClient::getFollowRedirects() const {
    return follow_redirects_;
  }

} // namespace netd::shared
