#include <atomic>
#include <cstring>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/netconf/rpc/stream.hpp>

namespace netd::shared {

  const size_t RpcRxStreamBuf::BUFFER_SIZE;
  const size_t RpcTxStreamBuf::BUFFER_SIZE;

  RpcRxStreamBuf::RpcRxStreamBuf(ClientSocket &socket) : socket_(socket) {
    buffer_.resize(BUFFER_SIZE);
    setg(nullptr, nullptr, nullptr);
  }

  std::streambuf::int_type RpcRxStreamBuf::underflow() {
    if (gptr() < egptr()) {
      return traits_type::to_int_type(*gptr());
    }

    if (!socket_.hasData()) {
      return traits_type::eof();
    }

    std::string data = socket_.receiveData();
    if (data.empty()) {
      return traits_type::eof();
    }

    buffer_ = data;
    setg(buffer_.data(), buffer_.data(), buffer_.data() + buffer_.size());

    return traits_type::to_int_type(*gptr());
  }

  std::streambuf::int_type RpcRxStreamBuf::uflow() {
    int_type result = underflow();
    if (result != traits_type::eof()) {
      gbump(1);
    }
    return result;
  }

  void RpcRxStreamBuf::rewind() {
    if (!buffer_.empty()) {
      setg(buffer_.data(), buffer_.data(), buffer_.data() + buffer_.size());
    }
  }

  void RpcRxStreamBuf::rewindOne() {
    if (message_starts_.empty()) {
      return;
    }

    message_starts_.pop_back();
    size_t prev_start = message_starts_.empty() ? 0 : message_starts_.back();
    setg(buffer_.data(), buffer_.data() + prev_start,
         buffer_.data() + buffer_.size());
  }

  bool RpcRxStreamBuf::hasData() {
    size_t current_pos = gptr() - eback();

    for (size_t start : message_starts_) {
      if (start > current_pos) {
        Logger::getInstance().debug(
            "RpcRxStreamBuf::hasData: Found unprocessed message at position " +
            std::to_string(start));
        return true;
      }
    }

    bool socket_has_data = socket_.hasData();
    Logger::getInstance().debug(
        "RpcRxStreamBuf::hasData: No unprocessed messages, socket_hasData=" +
        std::to_string(socket_has_data));
    return socket_has_data;
  }

  std::string RpcRxStreamBuf::readNextMessage() {
    if (gptr() >= egptr()) {
      std::string new_message = socket_.receiveData();
      if (new_message.empty()) {
        throw EndOfStreamError("No data available from socket");
      }
      message_starts_.push_back(buffer_.length());
      buffer_ += new_message;
      setg(buffer_.data(),
           buffer_.data() + buffer_.length() - new_message.length(),
           buffer_.data() + buffer_.size());
    }

    size_t current_pos = gptr() - eback();
    size_t next_message_start = buffer_.length();

    for (size_t start : message_starts_) {
      if (start > current_pos) {
        next_message_start = start;
        break;
      }
    }

    std::string result =
        buffer_.substr(current_pos, next_message_start - current_pos);

    setg(buffer_.data(), buffer_.data() + next_message_start,
         buffer_.data() + buffer_.size());

    return result;
  }

  ClientSocket &RpcRxStreamBuf::getSocket() { return socket_; }

  RpcTxStreamBuf::RpcTxStreamBuf(ClientSocket &socket) : socket_(socket) {
    buffer_.resize(BUFFER_SIZE);
    setp(buffer_.data(), buffer_.data() + BUFFER_SIZE - 1);
  }

  RpcTxStreamBuf::~RpcTxStreamBuf() { sync(); }

  std::streambuf::int_type RpcTxStreamBuf::overflow(int_type c) {
    if (c != traits_type::eof()) {
      *pptr() = traits_type::to_char_type(c);
      pbump(1);
    }

    if (sync() == 0) {
      return traits_type::not_eof(c);
    } else {
      return traits_type::eof();
    }
  }

  int RpcTxStreamBuf::sync() {
    if (pptr() > pbase()) {
      std::string data(pbase(), pptr() - pbase());
      bool success = socket_.sendData(data);

      if (success) {
        setp(buffer_.data(), buffer_.data() + BUFFER_SIZE - 1);
        return 0;
      } else {
        return -1;
      }
    }
    return 0;
  }

  int RpcTxStreamBuf::flush() { return sync(); }

  RpcRxStream::RpcRxStream(ClientSocket &socket)
      : std::istream(&streambuf_), streambuf_(socket) {}

  void RpcRxStream::rewind() { streambuf_.rewind(); }

  void RpcRxStream::rewindOne() { streambuf_.rewindOne(); }

  bool RpcRxStream::hasData() { return streambuf_.hasData(); }

  std::string RpcRxStream::readNextMessage() {
    return streambuf_.readNextMessage();
  }

  ClientSocket &RpcRxStream::getSocket() { return streambuf_.getSocket(); }

  RpcTxStream::RpcTxStream(ClientSocket &socket)
      : std::ostream(&streambuf_), streambuf_(socket) {}

} // namespace netd::shared