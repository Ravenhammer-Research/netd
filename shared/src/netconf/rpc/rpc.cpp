
#include <libyang/libyang.h>
#include <shared/include/exception.hpp>
#include <shared/include/netconf/rpc/rpc.hpp>
#include <shared/include/socket.hpp>
#include <shared/include/xml/base.hpp>
#include <shared/include/xml/envelope.hpp>
#include <shared/include/xml/hello.hpp>
#include <shared/include/yang.hpp>
#include <sstream>

namespace netd::shared::netconf {

  void Rpc::processRpc(RpcRxStream &rpc_stream, NetconfSession *session) {
    std::string xml_data = rpc_stream.readNextMessage();

    ly_ctx *ctx = session->getContext();
    auto envelope = netd::shared::xml::RpcEnvelope::fromXml(xml_data, ctx);
    rpc_stream.rewindOne();

    if (envelope->getRpcType() == netd::shared::xml::RpcType::RPC) {
      processRequest(rpc_stream, session);
    } else if (envelope->getRpcType() ==
               netd::shared::xml::RpcType::RPC_REPLY) {
      processReply(rpc_stream, session);
    } else {
      throw netd::shared::RpcError("Invalid RPC type");
    }
  }

  void Rpc::processRequest(RpcRxStream &rpc_stream, NetconfSession *session) {
    (void)rpc_stream;
    (void)session;
    throw netd::shared::NotImplementedError("processRequest not implemented");
  }

  void Rpc::processReply(RpcRxStream &rpc_stream, NetconfSession *session) {
    (void)rpc_stream;
    (void)session;
    throw netd::shared::NotImplementedError("processReply not implemented");
  }

  void Rpc::sendHelloToServer(const ClientSocket &client_socket,
                              NetconfSession *session) {
    if (!session) {
      throw netd::shared::ArgumentError("session not found");
    }

    auto yang_ctx = session->getContext();

    auto capabilities = netd::shared::Yang::getInstance().getCapabilities();
    auto hello = netd::shared::xml::HelloToServer::toXml(
        session->getSessionId(), capabilities, yang_ctx);

    std::string xml_str = hello->toString(yang_ctx);

    RpcTxStream tx_stream(const_cast<ClientSocket &>(client_socket));

    tx_stream << xml_str;
    tx_stream.flush();
  }

  void Rpc::sendHelloToClient(const ClientSocket &client_socket,
                              NetconfSession *session) {
    if (!session) {
      throw netd::shared::ArgumentError("session not found");
    }

    auto yang_ctx = session->getContext();

    auto capabilities = netd::shared::Yang::getInstance().getCapabilities();
    auto hello = netd::shared::xml::HelloToClient::toXml(
        session->getSessionId(), capabilities, yang_ctx);

    std::string xml_str = hello->toString(yang_ctx);

    RpcTxStream tx_stream(const_cast<ClientSocket &>(client_socket));

    tx_stream << xml_str;
    tx_stream.flush();
  }

} // namespace netd::shared::netconf