#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <regex>
#include <shared/include/logger.hpp>
#include <shared/include/unix.hpp>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

namespace netd::shared {

  static constexpr const char *CHUNK_END_MARKER = "\n##\n";
  static constexpr const char *NETCONF_SEPARATOR = "]]>]]>";
  static constexpr const char *CHUNK_HEADER_PATTERN = "\n#";
  static constexpr const char *CHUNK_START_PATTERN = "^\n#";
  static constexpr const char *CHUNK_HEADER_PREFIX = "\n#";
  static constexpr const char *NEWLINE = "\n";
  static constexpr const char *NULL_TERMINATOR = "\0";
  static constexpr const char *PATH_SEPARATOR = "/";
  static constexpr const char *CHUNK_SIZE_PATTERN = "\n#(\\d+)\n";
  static constexpr const char *HEADER_EXTRACT_PATTERN = "\n#.*?\n";
  static constexpr const char *AFTER_HEADER_PATTERN = "\n#.*?\n(.*)";
  static constexpr const char *BEFORE_SEPARATOR_PATTERN = "^(.*?)]]>]]>";
  static constexpr const char *CHUNK_EXTRACT_PATTERN = "^(.{0,%zu})(.*)";
  static constexpr size_t CHUNK_SIZE = 4096;
  static constexpr size_t BUFFER_SIZE = 4096;
  static constexpr int SOCKET_BACKLOG = 10;
  static constexpr mode_t SOCKET_PERMISSIONS = 0666;

  UnixTransport::UnixTransport()
      : listening_(false), server_socket_(-1), client_socket_(-1),
        use_chunking_(true) {}

  UnixTransport::~UnixTransport() { stop(); }

  bool UnixTransport::start(const std::string &address) {
    socketPath_ = address;

    if (geteuid() != 0) {
      return false;
    }

    if (!checkSocketDirectory()) {
      return false;
    }

    if (!prepareSocketFile()) {
      return false;
    }

    if (!createServerSocket()) {
      return false;
    }

    listening_ = true;
    return true;
  }

  void UnixTransport::stop() {
    if (!listening_) {
      return;
    }

    listening_ = false;

    if (!socketPath_.empty()) {
      unlink(socketPath_.c_str());
    }

    if (server_socket_ >= 0) {
      close(server_socket_);
      server_socket_ = -1;
    }
  }

  bool UnixTransport::isListening() const { return listening_; }

  const std::string &UnixTransport::getSocketPath() const {
    return socketPath_;
  }

  int UnixTransport::getServerSocket() const { return server_socket_; }

  int UnixTransport::acceptConnection() {
    if (server_socket_ < 0) {
      return -1;
    }

    int flags = fcntl(server_socket_, F_GETFL, 0);
    fcntl(server_socket_, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_socket =
        accept(server_socket_, (struct sockaddr *)&client_addr, &client_len);

    fcntl(server_socket_, F_SETFL, flags);

    if (client_socket < 0 && errno == EAGAIN) {
      return -1;
    }

    return client_socket;
  }

  void UnixTransport::closeConnection(int socket_fd) {
    if (socket_fd >= 0) {
      close(socket_fd);
    }
  }

  const std::string &UnixTransport::getAddress() const { return socketPath_; }

  bool UnixTransport::sendData(int socket_fd, const std::string &data) {
    if (socket_fd < 0) {
      return false;
    }

    if (data.length() > 200) {
      Logger::getInstance().debug(
          "UnixTransport: Sending data: " + data.substr(0, 100) + "..." +
          data.substr(data.length() - 100));
    } else {
      Logger::getInstance().debug("UnixTransport: Sending data: " + data);
    }

    if (use_chunking_) {
      size_t offset = 0;

      while (offset < data.length()) {
        size_t current_chunk_size =
            std::min(CHUNK_SIZE, data.length() - offset);

        std::string chunk_header = std::string(CHUNK_HEADER_PREFIX) +
                                   std::to_string(current_chunk_size) + NEWLINE;

        if (send(socket_fd, chunk_header.c_str(), chunk_header.length(), 0) <
            0) {
          return false;
        }

        if (send(socket_fd, data.c_str() + offset, current_chunk_size, 0) < 0) {
          return false;
        }

        offset += current_chunk_size;
      }

      if (send(socket_fd, CHUNK_END_MARKER, strlen(CHUNK_END_MARKER), 0) < 0) {
        return false;
      }
    } else {
      if (send(socket_fd, data.c_str(), data.length(), 0) < 0) {
        return false;
      }

      if (send(socket_fd, NETCONF_SEPARATOR, strlen(NETCONF_SEPARATOR), 0) <
          0) {
        return false;
      }
    }

    return true;
  }

  std::string UnixTransport::receiveData(int socket_fd) {
    if (socket_fd < 0) {
      return "";
    }

    char peek_buffer[16];
    ssize_t bytes_received =
        recv(socket_fd, peek_buffer, sizeof(peek_buffer) - 1, 0);
    if (bytes_received <= 0) {
      return "";
    }

    peek_buffer[bytes_received] = *NULL_TERMINATOR;
    std::string peek_data(peek_buffer);

    std::regex chunk_start_pattern(CHUNK_START_PATTERN);
    bool is_chunked = std::regex_search(peek_data, chunk_start_pattern);

    std::string result;
    if (is_chunked) {
      result = receiveChunkedDataFromBuffer(socket_fd, peek_data);
    } else {
      result = receiveFramedDataFromBuffer(socket_fd, peek_data);
    }

    if (result.length() > 200) {
      Logger::getInstance().debug(
          "UnixTransport: Received data: " + result.substr(0, 100) + "..." +
          result.substr(result.length() - 100));
    } else {
      Logger::getInstance().debug("UnixTransport: Received data: " + result);
    }
    return result;
  }

  std::string UnixTransport::readNextMessage(int socket_fd) {
    if (socket_fd < 0) {
      return "";
    }

    char peek_buffer[16];
    ssize_t bytes_received =
        recv(socket_fd, peek_buffer, sizeof(peek_buffer) - 1, 0);
    if (bytes_received <= 0) {
      return "";
    }

    peek_buffer[bytes_received] = *NULL_TERMINATOR;
    std::string peek_data(peek_buffer);

    std::regex chunk_start_pattern(CHUNK_START_PATTERN);
    bool is_chunked = std::regex_search(peek_data, chunk_start_pattern);

    std::string result;
    if (is_chunked) {
      result = readNextChunkedMessage(socket_fd, peek_data);
    } else {
      result = readNextFramedMessage(socket_fd, peek_data);
    }

    Logger::getInstance().debug("UnixTransport: Read next message: " + result);
    return result;
  }

  std::string
  UnixTransport::readNextChunkedMessage(int socket_fd,
                                        const std::string &initial_data) {
    std::string result;
    char buffer[BUFFER_SIZE];
    std::string current_data = initial_data;
    std::regex header_pattern(CHUNK_HEADER_PATTERN);

    while (true) {
      std::smatch match;
      if (!std::regex_search(current_data, match, header_pattern)) {
        ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
          return "";
        }
        buffer[bytes_received] = *NULL_TERMINATOR;
        current_data += buffer;
        continue;
      }

      size_t header_start = match.position();
      std::regex newline_pattern(NEWLINE);
      std::sregex_iterator iter(current_data.begin() + header_start + 2,
                                current_data.end(), newline_pattern);
      std::sregex_iterator end;
      size_t header_end = (iter != end) ? iter->position() + header_start + 2
                                        : std::string::npos;
      if (header_end == std::string::npos) {
        ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
          return "";
        }
        buffer[bytes_received] = *NULL_TERMINATOR;
        current_data += buffer;
        continue;
      }

      std::regex header_extract_pattern(HEADER_EXTRACT_PATTERN);
      std::smatch header_match;
      if (std::regex_search(current_data, header_match,
                            header_extract_pattern)) {
        std::string header = header_match.str();

        if (header == CHUNK_END_MARKER) {
          break;
        }

        std::regex chunk_size_pattern(CHUNK_SIZE_PATTERN);
        std::smatch size_match;
        if (!std::regex_search(header, size_match, chunk_size_pattern)) {
          return "";
        }

        size_t chunk_size = std::stoul(size_match[1].str());

        std::regex after_header_pattern(AFTER_HEADER_PATTERN);
        std::smatch after_match;
        if (std::regex_search(current_data, after_match,
                              after_header_pattern)) {
          current_data = after_match[1].str();
        }

        while (current_data.length() < chunk_size) {
          size_t needed = chunk_size - current_data.length();
          size_t to_read = std::min(needed, sizeof(buffer));

          ssize_t bytes_received = recv(socket_fd, buffer, to_read, 0);
          if (bytes_received <= 0) {
            return "";
          }

          current_data.append(buffer, bytes_received);
        }

        char chunk_pattern[256];
        snprintf(chunk_pattern, sizeof(chunk_pattern), CHUNK_EXTRACT_PATTERN,
                 chunk_size);
        std::regex chunk_extract_pattern(chunk_pattern);
        std::smatch chunk_match;
        if (std::regex_search(current_data, chunk_match,
                              chunk_extract_pattern)) {
          std::string chunk_data = chunk_match[1].str();
          result += chunk_data;
          current_data = chunk_match[2].str();
        }
      }
    }

    return result;
  }

  std::string
  UnixTransport::readNextFramedMessage(int socket_fd,
                                       const std::string &initial_data) {
    std::string result = initial_data;
    char buffer[BUFFER_SIZE];
    std::regex separator_pattern(NETCONF_SEPARATOR);

    while (true) {
      if (std::regex_search(result, separator_pattern)) {
        break;
      }

      ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
      if (bytes_received <= 0) {
        return "";
      }

      buffer[bytes_received] = *NULL_TERMINATOR;
      result += buffer;
    }

    std::smatch match;
    if (std::regex_search(result, match, separator_pattern)) {
      std::regex before_separator_pattern(BEFORE_SEPARATOR_PATTERN);
      std::smatch before_match;
      if (std::regex_search(result, before_match, before_separator_pattern)) {
        result = before_match[1].str();
      }
    }

    return result;
  }

  std::string UnixTransport::receiveChunkedData(int socket_fd) {
    return receiveChunkedDataFromBuffer(socket_fd, "");
  }

  std::string UnixTransport::receiveFramedData(int socket_fd) {
    return receiveFramedDataFromBuffer(socket_fd, "");
  }

  std::string
  UnixTransport::receiveChunkedDataFromBuffer(int socket_fd,
                                              const std::string &initial_data) {
    std::string result;
    char buffer[BUFFER_SIZE];
    std::string current_data = initial_data;
    std::regex header_pattern(CHUNK_HEADER_PATTERN);

    while (true) {
      std::smatch match;
      if (!std::regex_search(current_data, match, header_pattern)) {
        ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
          return "";
        }
        buffer[bytes_received] = *NULL_TERMINATOR;
        current_data += buffer;
        continue;
      }

      size_t header_start = match.position();
      std::regex newline_pattern(NEWLINE);
      std::sregex_iterator iter(current_data.begin() + header_start + 2,
                                current_data.end(), newline_pattern);
      std::sregex_iterator end;
      size_t header_end = (iter != end) ? iter->position() + header_start + 2
                                        : std::string::npos;
      if (header_end == std::string::npos) {
        ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
          return "";
        }
        buffer[bytes_received] = *NULL_TERMINATOR;
        current_data += buffer;
        continue;
      }

      std::regex header_extract_pattern(HEADER_EXTRACT_PATTERN);
      std::smatch header_match;
      if (std::regex_search(current_data, header_match,
                            header_extract_pattern)) {
        std::string header = header_match.str();

        if (header == CHUNK_END_MARKER) {
          break;
        }

        std::regex chunk_size_pattern(CHUNK_SIZE_PATTERN);
        std::smatch size_match;
        if (!std::regex_search(header, size_match, chunk_size_pattern)) {
          return "";
        }

        size_t chunk_size = std::stoul(size_match[1].str());

        std::regex after_header_pattern(AFTER_HEADER_PATTERN);
        std::smatch after_match;
        if (std::regex_search(current_data, after_match,
                              after_header_pattern)) {
          current_data = after_match[1].str();
        }

        while (current_data.length() < chunk_size) {
          size_t needed = chunk_size - current_data.length();
          size_t to_read = std::min(needed, sizeof(buffer));

          ssize_t bytes_received = recv(socket_fd, buffer, to_read, 0);
          if (bytes_received <= 0) {
            return "";
          }

          current_data.append(buffer, bytes_received);
        }

        char chunk_pattern[256];
        snprintf(chunk_pattern, sizeof(chunk_pattern), CHUNK_EXTRACT_PATTERN,
                 chunk_size);
        std::regex chunk_extract_pattern(chunk_pattern);
        std::smatch chunk_match;
        if (std::regex_search(current_data, chunk_match,
                              chunk_extract_pattern)) {
          std::string chunk_data = chunk_match[1].str();
          result += chunk_data;
          current_data = chunk_match[2].str();
        }
      }
    }

    return result;
  }

  std::string
  UnixTransport::receiveFramedDataFromBuffer(int socket_fd,
                                             const std::string &initial_data) {
    std::string result = initial_data;
    char buffer[BUFFER_SIZE];
    std::regex separator_pattern(NETCONF_SEPARATOR);

    while (true) {
      if (std::regex_search(result, separator_pattern)) {
        break;
      }

      ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
      if (bytes_received <= 0) {
        return "";
      }

      buffer[bytes_received] = *NULL_TERMINATOR;
      result += buffer;
    }

    std::smatch match;
    if (std::regex_search(result, match, separator_pattern)) {
      std::regex before_separator_pattern(BEFORE_SEPARATOR_PATTERN);
      std::smatch before_match;
      if (std::regex_search(result, before_match, before_separator_pattern)) {
        result = before_match[1].str();
      }
    }

    return result;
  }

  bool UnixTransport::hasData(int socket_fd) {
    if (socket_fd < 0) {
      Logger::getInstance().debug("UnixTransport::hasData: Invalid socket fd");
      return false;
    }

    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(socket_fd, &readfds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    int result = select(socket_fd + 1, &readfds, nullptr, nullptr, &timeout);
    if (result <= 0 || !FD_ISSET(socket_fd, &readfds)) {
      Logger::getInstance().debug(
          "UnixTransport::hasData: No data available from select");
      return false;
    }

    char peek_buffer[1];
    ssize_t bytes_received = recv(socket_fd, peek_buffer, 1, MSG_PEEK);
    bool has_data = bytes_received > 0;
    Logger::getInstance().debug("UnixTransport::hasData: Peek result=" +
                                std::to_string(bytes_received) +
                                ", hasData=" + std::to_string(has_data));
    return has_data;
  }

  void UnixTransport::cancelOperation(int socket_fd) {
    std::lock_guard<std::mutex> lock(cancellation_mutex_);
    cancellation_flags_[socket_fd] = true;
  }

  bool UnixTransport::createServerSocket() {
    server_socket_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
      return false;
    }

    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt)) < 0) {
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath_.c_str(), sizeof(addr.sun_path) - 1);

    if (bind(server_socket_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }

    chmod(socketPath_.c_str(), SOCKET_PERMISSIONS);

    if (listen(server_socket_, SOCKET_BACKLOG) < 0) {
      close(server_socket_);
      server_socket_ = -1;
      return false;
    }

    return true;
  }

  bool UnixTransport::prepareSocketFile() {
    struct stat st;
    if (stat(socketPath_.c_str(), &st) == 0) {
      if (!S_ISSOCK(st.st_mode)) {
        return false;
      }

      if (unlink(socketPath_.c_str()) != 0) {
        return false;
      }
    }

    return true;
  }

  bool UnixTransport::checkSocketDirectory() {
    std::regex slash_pattern(PATH_SEPARATOR);
    std::sregex_token_iterator end;
    std::sregex_token_iterator iter(socketPath_.begin(), socketPath_.end(),
                                    slash_pattern, -1);
    std::vector<std::string> parts(iter, end);

    if (parts.empty()) {
      return false;
    }

    std::string dirPath = parts[0];
    for (size_t i = 1; i < parts.size() - 1; ++i) {
      dirPath += PATH_SEPARATOR + parts[i];
    }

    struct stat st;
    if (stat(dirPath.c_str(), &st) != 0) {
      return false;
    }

    if (!S_ISDIR(st.st_mode)) {
      return false;
    }

    if (access(dirPath.c_str(), W_OK) != 0) {
      return false;
    }

    return true;
  }

  bool UnixTransport::connect(const std::string &address) {
    socketPath_ = address;

    client_socket_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket_ < 0) {
      return false;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath_.c_str(), sizeof(addr.sun_path) - 1);

    if (::connect(client_socket_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      close(client_socket_);
      client_socket_ = -1;
      return false;
    }

    return true;
  }

  void UnixTransport::disconnect() {
    if (client_socket_ >= 0) {
      close(client_socket_);
      client_socket_ = -1;
    }
  }

  int UnixTransport::getSocket() const { return client_socket_; }

} // namespace netd::shared