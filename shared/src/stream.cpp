#include <shared/include/stream.hpp>
#include <shared/include/logger.hpp>
#include <atomic>
#include <cstring>

namespace netd::shared {

  const size_t RpcRxStreamBuf::BUFFER_SIZE;
  const size_t RpcTxStreamBuf::BUFFER_SIZE;

  RpcRxStreamBuf::RpcRxStreamBuf(ClientSocket& socket)
      : socket_(socket) {
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

    Logger::getInstance().debug("RpcRxStreamBuf: Read data: " + data);
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

  bool RpcRxStreamBuf::hasData() {
    return socket_.hasData();
  }

  std::string RpcRxStreamBuf::readToEnd() {
    if (underflow() == traits_type::eof()) {
      return "";
    }
    
    std::string result;
    if (gptr() < egptr()) {
      result.assign(gptr(), egptr() - gptr());
    }
    
    return result;
  }

  ClientSocket& RpcRxStreamBuf::getSocket() {
    return socket_;
  }

  RpcTxStreamBuf::RpcTxStreamBuf(ClientSocket& socket)
      : socket_(socket) {
    buffer_.resize(BUFFER_SIZE);
    setp(buffer_.data(), buffer_.data() + BUFFER_SIZE - 1);
  }

  RpcTxStreamBuf::~RpcTxStreamBuf() {
    sync();
  }

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
      Logger::getInstance().debug("RpcTxStreamBuf: Write data: " + data);
      bool success = socket_.sendData(data);
      Logger::getInstance().debug("RpcTxStreamBuf: sendData returned " + std::string(success ? "true" : "false"));
      
      if (success) {
        setp(buffer_.data(), buffer_.data() + BUFFER_SIZE - 1);
        return 0;
      } else {
        return -1;
      }
    }
    return 0;
  }

  int RpcTxStreamBuf::flush() {
    Logger::getInstance().debug("RpcTxStreamBuf: Flush called");
    return sync();
  }

  RpcRxStream::RpcRxStream(ClientSocket& socket)
      : std::istream(&streambuf_), streambuf_(socket) {
  }

  void RpcRxStream::rewind() {
    streambuf_.rewind();
  }

  bool RpcRxStream::hasData() {
    return streambuf_.hasData();
  }

  std::string RpcRxStream::readToEnd() {
    return streambuf_.readToEnd();
  }

  ClientSocket& RpcRxStream::getSocket() {
    return streambuf_.getSocket();
  }

  RpcTxStream::RpcTxStream(ClientSocket& socket)
      : std::ostream(&streambuf_), streambuf_(socket) {
  }


} // namespace netd::shared