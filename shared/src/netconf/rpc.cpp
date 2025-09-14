
#include <shared/include/netconf/rpc.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/yang.hpp>
#include <shared/include/xml/base.hpp>
#include <shared/include/xml/envelope.hpp>
#include <libyang/libyang.h>
#include <sstream>

namespace netd::shared::netconf {


  void Rpc::processRpc(RpcRxStream& rpc_stream, NetconfSession* session) {
    std::string xml_data = rpc_stream.readToEnd();
    
    ly_ctx *ctx = session->getContext();
    auto envelope = netd::shared::xml::RpcEnvelope::fromXml(xml_data, ctx);
    
    if (envelope->getRpcType() == netd::shared::xml::RpcType::RPC) {
      rpc_stream.rewind();
      processRequest(rpc_stream, session);
    } else if (envelope->getRpcType() == netd::shared::xml::RpcType::RPC_REPLY) {
      rpc_stream.rewind();
      processReply(rpc_stream, session);
    } else {
      throw netd::shared::RpcError("Invalid RPC type");
    }
  }

  void Rpc::processRequest(RpcRxStream& rpc_stream, NetconfSession* session) {
    (void)rpc_stream;
    (void)session;
    throw netd::shared::NotImplementedError("processRequest not implemented");
  }

  void Rpc::processReply(RpcRxStream& rpc_stream, NetconfSession* session) {
    (void)rpc_stream;
    (void)session;
    throw netd::shared::NotImplementedError("processReply not implemented");
  }
  
} // namespace netd::shared::netconf