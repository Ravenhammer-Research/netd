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

#ifndef NETD_SHARED_NETCONF_RPC_HPP
#define NETD_SHARED_NETCONF_RPC_HPP

#include <istream>
#include <libyang/libyang.h>
#include <map>
#include <memory>
#include <shared/include/exception.hpp>
#include <shared/include/netconf/session.hpp>
#include <shared/include/socket.hpp>
#include <shared/include/stream.hpp>
#include <string>

namespace netd::shared::netconf {

  enum class NetconfVersion { VERSION_10, VERSION_11 };

  enum class NetconfOperation {
    GET,
    GET_CONFIG,
    EDIT_CONFIG,
    COPY_CONFIG,
    DELETE_CONFIG,
    LOCK,
    UNLOCK,
    CLOSE_SESSION,
    KILL_SESSION,
    COMMIT,
    DISCARD_CHANGES,
    VALIDATE,
    CANCEL_COMMIT
  };

  // Convert enum to string for XML operation names
  constexpr const char *operationToString(NetconfOperation op) {
    switch (op) {
    case NetconfOperation::GET:
      return "get";
    case NetconfOperation::GET_CONFIG:
      return "get-config";
    case NetconfOperation::EDIT_CONFIG:
      return "edit-config";
    case NetconfOperation::COPY_CONFIG:
      return "copy-config";
    case NetconfOperation::DELETE_CONFIG:
      return "delete-config";
    case NetconfOperation::LOCK:
      return "lock";
    case NetconfOperation::UNLOCK:
      return "unlock";
    case NetconfOperation::CLOSE_SESSION:
      return "close-session";
    case NetconfOperation::KILL_SESSION:
      return "kill-session";
    case NetconfOperation::COMMIT:
      return "commit";
    case NetconfOperation::DISCARD_CHANGES:
      return "discard-changes";
    case NetconfOperation::VALIDATE:
      return "validate";
    case NetconfOperation::CANCEL_COMMIT:
      return "cancel-commit";
    default:
      return nullptr;
    }
  }

  // Static map for string to operation conversion
  inline const std::map<std::string, NetconfOperation> operation_map = {
      {"get", NetconfOperation::GET},
      {"get-config", NetconfOperation::GET_CONFIG},
      {"edit-config", NetconfOperation::EDIT_CONFIG},
      {"copy-config", NetconfOperation::COPY_CONFIG},
      {"delete-config", NetconfOperation::DELETE_CONFIG},
      {"lock", NetconfOperation::LOCK},
      {"unlock", NetconfOperation::UNLOCK},
      {"close-session", NetconfOperation::CLOSE_SESSION},
      {"kill-session", NetconfOperation::KILL_SESSION},
      {"commit", NetconfOperation::COMMIT},
      {"discard-changes", NetconfOperation::DISCARD_CHANGES},
      {"validate", NetconfOperation::VALIDATE},
      {"cancel-commit", NetconfOperation::CANCEL_COMMIT}};

  // Convert string to enum for parsing
  inline NetconfOperation stringToOperation(const std::string &op_str) {
    auto it = operation_map.find(op_str);
    if (it != operation_map.end()) {
      return it->second;
    }
    throw netd::shared::RpcError("Invalid operation name: " + op_str);
  }

  class Rpc {
  public:
    // Static methods to be implemented by derived classes
    static void processRpc(RpcRxStream &rpc_stream, NetconfSession *session);
    static void processRequest(RpcRxStream &rpc_stream,
                               NetconfSession *session);
    static void processReply(RpcRxStream &rpc_stream, NetconfSession *session);

    // Shared hello message functionality
    static void sendHelloToServer(const ClientSocket &client_socket,
                                  NetconfSession *session);
    static void sendHelloToClient(const ClientSocket &client_socket,
                                  NetconfSession *session);

    virtual ~Rpc() = default;

  private:
    Rpc() = default;
  };

} // namespace netd::shared::netconf

#endif // NETD_SHARED_NETCONF_RPC_HPP
