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

#include <shared/include/exception.hpp>
#include <shared/include/http.hpp>
#include <shared/include/quic.hpp>

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

  HTTPTransport::~HTTPTransport() { stop(); }

  // BaseTransport interface implementation
  bool HTTPTransport::start(const std::string &address) {
    (void)address;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::start not implemented");
  }

  bool HTTPTransport::start(const std::string &address, int port) {
    (void)address;
    (void)port;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::start not implemented");
  }

  void HTTPTransport::stop() {
    throw netd::shared::NotImplementedError(
        "HTTPTransport::stop not implemented");
  }

  bool HTTPTransport::isListening() const {
    throw netd::shared::NotImplementedError(
        "HTTPTransport::isListening not implemented");
  }

  const std::string &HTTPTransport::getAddress() const {
    throw netd::shared::NotImplementedError(
        "HTTPTransport::getAddress not implemented");
  }

  int HTTPTransport::getPort() const {
    throw netd::shared::NotImplementedError(
        "HTTPTransport::getPort not implemented");
  }

  int HTTPTransport::acceptConnection() {
    throw netd::shared::NotImplementedError(
        "HTTPTransport::acceptConnection not implemented");
  }

  void HTTPTransport::closeConnection(int socket_fd) {
    (void)socket_fd;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::closeConnection not implemented");
  }

  bool HTTPTransport::sendData(int socket_fd, const std::string &data) {
    (void)socket_fd;
    (void)data;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::sendData not implemented");
  }

  std::string HTTPTransport::receiveData(int socket_fd) {
    (void)socket_fd;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::receiveData not implemented");
  }

  void HTTPTransport::cancelOperation(int socket_fd) {
    (void)socket_fd;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::cancelOperation not implemented");
  }

  void HTTPTransport::disconnect() {
    throw netd::shared::NotImplementedError(
        "HTTPTransport::disconnect not implemented");
  }

  int HTTPTransport::getSocket() const {
    throw netd::shared::NotImplementedError(
        "HTTPTransport::getSocket not implemented");
  }

  bool HTTPTransport::connect(const std::string &address) {
    (void)address;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::connect not implemented");
  }

  // HTTP-specific interface
  void HTTPTransport::addRoute(const std::string &method,
                               const std::string &path, RouteHandler handler) {
    (void)method;
    (void)path;
    (void)handler;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::addRoute not implemented");
  }

  void HTTPTransport::get(const std::string &path, RouteHandler handler) {
    (void)path;
    (void)handler;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::get not implemented");
  }

  void HTTPTransport::post(const std::string &path, RouteHandler handler) {
    (void)path;
    (void)handler;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::post not implemented");
  }

  void HTTPTransport::put(const std::string &path, RouteHandler handler) {
    (void)path;
    (void)handler;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::put not implemented");
  }

  void HTTPTransport::del(const std::string &path, RouteHandler handler) {
    (void)path;
    (void)handler;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::del not implemented");
  }

  void HTTPTransport::patch(const std::string &path, RouteHandler handler) {
    (void)path;
    (void)handler;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::patch not implemented");
  }

  void HTTPTransport::setMaxThreads(size_t max_threads) {
    (void)max_threads;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::setMaxThreads not implemented");
  }

  size_t HTTPTransport::getActiveThreads() const {
    throw netd::shared::NotImplementedError(
        "HTTPTransport::getActiveThreads not implemented");
  }

  void HTTPTransport::enableHttp2(bool enable) {
    (void)enable;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::enableHttp2 not implemented");
  }

  void HTTPTransport::enableHttp3(bool enable) {
    (void)enable;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::enableHttp3 not implemented");
  }

  void HTTPTransport::setHttp2Settings(
      const std::unordered_map<std::string, std::string> &settings) {
    (void)settings;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::setHttp2Settings not implemented");
  }

  void HTTPTransport::setHttp3Settings(
      const std::unordered_map<std::string, std::string> &settings) {
    (void)settings;
    throw netd::shared::NotImplementedError(
        "HTTPTransport::setHttp3Settings not implemented");
  }

  bool HTTPTransport::isHttp2Enabled() const {
    throw netd::shared::NotImplementedError(
        "HTTPTransport::isHttp2Enabled not implemented");
  }

  bool HTTPTransport::isHttp3Enabled() const {
    throw netd::shared::NotImplementedError(
        "HTTPTransport::isHttp3Enabled not implemented");
  }

  std::vector<HttpVersion> HTTPTransport::getSupportedProtocols() const {
    throw netd::shared::NotImplementedError(
        "HTTPTransport::getSupportedProtocols not implemented");
  }

  // HTTP Client Implementation
  HTTPClient::HTTPClient()
      : host_(""), port_(80), use_ssl_(false),
        preferred_version_(HttpVersion::HTTP_1_1) {
    // Set default headers
    default_headers_["User-Agent"] = "netd-http-client/1.0";
    default_headers_["Accept"] = "*/*";
    default_headers_["Connection"] = "keep-alive";
  }

  HTTPClient::~HTTPClient() { disconnect(); }

  bool HTTPClient::connect(const std::string &host, int port, bool use_ssl) {
    (void)host;
    (void)port;
    (void)use_ssl;
    throw netd::shared::NotImplementedError(
        "HTTPClient::connect not implemented");
  }

  void HTTPClient::disconnect() {
    throw netd::shared::NotImplementedError(
        "HTTPClient::disconnect not implemented");
  }

  bool HTTPClient::isConnected() const {
    throw netd::shared::NotImplementedError(
        "HTTPClient::isConnected not implemented");
  }

  void HTTPClient::setPreferredVersion(HttpVersion version) {
    (void)version;
    throw netd::shared::NotImplementedError(
        "HTTPClient::setPreferredVersion not implemented");
  }

  void HTTPClient::setTimeout(uint32_t timeout_ms) {
    (void)timeout_ms;
    throw netd::shared::NotImplementedError(
        "HTTPClient::setTimeout not implemented");
  }

  void HTTPClient::setMaxRedirects(size_t max_redirects) {
    (void)max_redirects;
    throw netd::shared::NotImplementedError(
        "HTTPClient::setMaxRedirects not implemented");
  }

  void HTTPClient::setFollowRedirects(bool follow) {
    (void)follow;
    throw netd::shared::NotImplementedError(
        "HTTPClient::setFollowRedirects not implemented");
  }

  void HTTPClient::setDefaultHeader(const std::string &name,
                                    const std::string &value) {
    (void)name;
    (void)value;
    throw netd::shared::NotImplementedError(
        "HTTPClient::setDefaultHeader not implemented");
  }

  void HTTPClient::setDefaultHeaders(
      const std::unordered_map<std::string, std::string> &headers) {
    (void)headers;
    throw netd::shared::NotImplementedError(
        "HTTPClient::setDefaultHeaders not implemented");
  }

  HttpResponse
  HTTPClient::get(const std::string &path,
                  const std::unordered_map<std::string, std::string> &headers) {
    (void)path;
    (void)headers;
    throw netd::shared::NotImplementedError("HTTPClient::get not implemented");
  }

  HttpResponse HTTPClient::post(
      const std::string &path, const std::string &body,
      const std::unordered_map<std::string, std::string> &headers) {
    (void)path;
    (void)body;
    (void)headers;
    throw netd::shared::NotImplementedError("HTTPClient::post not implemented");
  }

  HttpResponse
  HTTPClient::put(const std::string &path, const std::string &body,
                  const std::unordered_map<std::string, std::string> &headers) {
    (void)path;
    (void)body;
    (void)headers;
    throw netd::shared::NotImplementedError("HTTPClient::put not implemented");
  }

  HttpResponse
  HTTPClient::del(const std::string &path,
                  const std::unordered_map<std::string, std::string> &headers) {
    (void)path;
    (void)headers;
    throw netd::shared::NotImplementedError("HTTPClient::del not implemented");
  }

  HttpResponse HTTPClient::patch(
      const std::string &path, const std::string &body,
      const std::unordered_map<std::string, std::string> &headers) {
    (void)path;
    (void)body;
    (void)headers;
    throw netd::shared::NotImplementedError(
        "HTTPClient::patch not implemented");
  }

  HttpResponse HTTPClient::request(
      const std::string &method, const std::string &path,
      const std::unordered_map<std::string, std::string> &headers,
      const std::string &body) {
    (void)method;
    (void)path;
    (void)headers;
    (void)body;
    throw netd::shared::NotImplementedError(
        "HTTPClient::request not implemented");
  }

  std::string HTTPClient::getHost() const {
    throw netd::shared::NotImplementedError(
        "HTTPClient::getHost not implemented");
  }

  int HTTPClient::getPort() const {
    throw netd::shared::NotImplementedError(
        "HTTPClient::getPort not implemented");
  }

  bool HTTPClient::isSSL() const {
    throw netd::shared::NotImplementedError(
        "HTTPClient::isSSL not implemented");
  }

  HttpVersion HTTPClient::getPreferredVersion() const {
    throw netd::shared::NotImplementedError(
        "HTTPClient::getPreferredVersion not implemented");
  }

  uint32_t HTTPClient::getTimeout() const {
    throw netd::shared::NotImplementedError(
        "HTTPClient::getTimeout not implemented");
  }

  size_t HTTPClient::getMaxRedirects() const {
    throw netd::shared::NotImplementedError(
        "HTTPClient::getMaxRedirects not implemented");
  }

  bool HTTPClient::getFollowRedirects() const {
    throw netd::shared::NotImplementedError(
        "HTTPClient::getFollowRedirects not implemented");
  }

} // namespace netd::shared
