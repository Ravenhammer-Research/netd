#define _WANT_UCRED
#include <shared/include/socket.hpp>
#include <shared/include/unix.hpp>
#include <shared/include/logger.hpp>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ucred.h>
#include <sys/un.h>
#include <unistd.h>

namespace netd::shared {

  ClientSocket::ClientSocket(int socket_fd) : socket_fd_(socket_fd) {
  }

  bool ClientSocket::sendData(const std::string& data) {
    if (socket_fd_ < 0) {
      return false;
    }
    
    UnixTransport transport;
    return transport.sendData(socket_fd_, data);
  }

  std::string ClientSocket::receiveData() {
    if (socket_fd_ < 0) {
      return "";
    }
    
    UnixTransport transport;
    return transport.receiveData(socket_fd_);
  }

  bool ClientSocket::hasData() {
    if (socket_fd_ < 0) {
      return false;
    }

    UnixTransport transport;
    return transport.hasData(socket_fd_);
  }

  uid_t ClientSocket::getUserId() const {
    if (socket_fd_ < 0) {
      return 0;
    }

    struct xucred cred;
    socklen_t len = sizeof(cred);
    
    if (getsockopt(socket_fd_, SOL_LOCAL, LOCAL_PEERCRED, &cred, &len) == -1) {
      return 0;
    }
    
    return cred.cr_uid;
  }

  void ClientSocket::close() {
    if (socket_fd_ >= 0) {
      ::close(socket_fd_);
      socket_fd_ = -1;
    }
  }

} // namespace netd::shared
