#ifndef NETD_SHARED_SOCKET_HPP
#define NETD_SHARED_SOCKET_HPP

#include <string>
#include <sys/types.h>

namespace netd::shared {

  class ClientSocket {
  public:
    ClientSocket(int socket_fd);
    ~ClientSocket() = default;

    bool sendData(const std::string &data);
    std::string receiveData();
    bool hasData();
    int getSocket() const { return socket_fd_; }
    uid_t getUserId() const;
    void close();

  private:
    int socket_fd_;
  };

} // namespace netd::shared

#endif // NETD_SHARED_SOCKET_HPP
