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

#ifndef NETD_SHARED_STREAM_HPP
#define NETD_SHARED_STREAM_HPP

#include <functional>
#include <iostream>
#include <memory>
#include <shared/include/netconf/session.hpp>
#include <shared/include/socket.hpp>
#include <streambuf>
#include <string>

namespace netd::shared {

  /**
   * @brief Custom streambuf for receiving RPC data
   */
  class RpcRxStreamBuf : public std::streambuf {
  public:
    RpcRxStreamBuf(ClientSocket &socket);
    virtual ~RpcRxStreamBuf() = default;

    void rewind();
    bool hasData();
    std::string readToEnd();

    /**
     * @brief Get the socket reference
     * @return Reference to the underlying socket
     */
    ClientSocket &getSocket();

  protected:
    virtual int_type underflow() override;
    virtual int_type uflow() override;

  private:
    ClientSocket &socket_;
    std::string buffer_;
    static const size_t BUFFER_SIZE = 4096;
  };

  /**
   * @brief Custom streambuf for transmitting RPC data
   */
  class RpcTxStreamBuf : public std::streambuf {
  public:
    RpcTxStreamBuf(ClientSocket &socket);
    virtual ~RpcTxStreamBuf();

    int flush();

  protected:
    virtual int_type overflow(int_type c) override;
    virtual int sync() override;

  private:
    ClientSocket &socket_;
    std::string buffer_;
    static const size_t BUFFER_SIZE = 4096;
  };

  /**
   * @brief Stream class for receiving RPC data
   *
   * This class provides a standard C++ istream interface for receiving RPC data
   * from a transport layer.
   */
  class RpcRxStream : public std::istream {
  public:
    /**
     * @brief Constructs an RpcRxStream
     * @param socket The socket to use for receiving data
     */
    RpcRxStream(ClientSocket &socket);

    /**
     * @brief Destructor
     */
    virtual ~RpcRxStream() = default;

    /**
     * @brief Rewinds the stream to the beginning of the current message
     */
    void rewind();

    /**
     * @brief Checks if data is available to read without consuming it
     * @return true if data is available, false otherwise
     */
    bool hasData();

    /**
     * @brief Reads all available data from the stream
     * @return string containing all data read from the stream
     */
    std::string readToEnd();

    /**
     * @brief Get the socket reference
     * @return Reference to the underlying socket
     */
    ClientSocket &getSocket();

  private:
    RpcRxStreamBuf streambuf_;
  };

  /**
   * @brief Stream class for transmitting RPC data
   *
   * This class provides a standard C++ ostream interface for sending RPC data
   * through a transport layer.
   */
  class RpcTxStream : public std::ostream {
  public:
    /**
     * @brief Constructs an RpcTxStream
     * @param socket The socket to use for sending data
     */
    RpcTxStream(ClientSocket &socket);

    /**
     * @brief Destructor
     */
    virtual ~RpcTxStream() = default;

  private:
    RpcTxStreamBuf streambuf_;
  };

} // namespace netd::shared

#endif // NETD_SHARED_STREAM_HPP
