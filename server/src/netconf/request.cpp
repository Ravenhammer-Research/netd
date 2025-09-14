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

#include <server/include/netconf/rpc.hpp>
#include <server/include/netconf/handlers.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/stream.hpp>
#include <shared/include/socket.hpp>
#include <shared/include/xml/base.hpp>
#include <shared/include/xml/envelope.hpp>
#include <istream>
#include <string>

namespace netd::server::netconf {

  void ServerRpc::processRequest(netd::shared::RpcRxStream& rpc_stream, netd::shared::netconf::NetconfSession* session) {
    std::string xml_data = rpc_stream.readToEnd();
    
    ly_ctx *ctx = session->getContext();
    auto envelope = netd::shared::xml::RpcEnvelope::fromXml(xml_data, ctx);
    
    int message_id = envelope->getMessageId();
    lyd_node* inner_data = envelope->getLydData();
    netd::shared::netconf::NetconfOperation operation = envelope->getOperation();
    
    // Convert unique_ptr to shared_ptr for sharing between request and response
    auto envelope_shared = std::shared_ptr<netd::shared::xml::RpcEnvelope>(envelope.release());
    
    switch (operation) {
      case netd::shared::netconf::NetconfOperation::GET: {
        auto request = netd::shared::request::get::GetRequest::fromRpcEnvelope(ctx, envelope_shared);
        auto response = netd::server::netconf::handlers::RpcHandler::handleGetRequest(request.get());
        if (response) {
          auto reply_envelope = response->toRpcEnvelope(envelope_shared, ctx);
          std::string reply_xml = reply_envelope->toString(ctx);
          
          netd::shared::RpcTxStream tx_stream(rpc_stream.getSocket());
          tx_stream << reply_xml;
          tx_stream.flush();
        }
        break;
      }
      case netd::shared::netconf::NetconfOperation::GET_CONFIG: {
        auto request = netd::shared::request::get::GetConfigRequest::fromRpcEnvelope(ctx, envelope_shared);
        auto response = netd::server::netconf::handlers::RpcHandler::handleGetConfigRequest(request.get());
        if (response) {
          auto reply_envelope = response->toRpcEnvelope(envelope_shared, ctx);
          std::string reply_xml = reply_envelope->toString(ctx);
          
          netd::shared::RpcTxStream tx_stream(rpc_stream.getSocket());
          tx_stream << reply_xml;
          tx_stream.flush();
        }
        break;
      }
      case netd::shared::netconf::NetconfOperation::EDIT_CONFIG: {
        auto request = netd::shared::request::EditConfigRequest::fromRpcEnvelope(ctx, envelope_shared);
        auto response = netd::server::netconf::handlers::RpcHandler::handleEditConfigRequest(request.get());
        if (response) {
          auto reply_envelope = response->toRpcEnvelope(envelope_shared, ctx);
          std::string reply_xml = reply_envelope->toString(ctx);
          
          netd::shared::RpcTxStream tx_stream(rpc_stream.getSocket());
          tx_stream << reply_xml;
          tx_stream.flush();
        }
        break;
      }
      case netd::shared::netconf::NetconfOperation::COPY_CONFIG: {
        auto request = netd::shared::request::CopyConfigRequest::fromRpcEnvelope(ctx, envelope_shared);
        auto response = netd::server::netconf::handlers::RpcHandler::handleCopyConfigRequest(request.get());
        if (response) {
          auto reply_envelope = response->toRpcEnvelope(envelope_shared, ctx);
          std::string reply_xml = reply_envelope->toString(ctx);
          
          netd::shared::RpcTxStream tx_stream(rpc_stream.getSocket());
          tx_stream << reply_xml;
          tx_stream.flush();
        }
        break;
      }
      case netd::shared::netconf::NetconfOperation::DELETE_CONFIG: {
        auto request = netd::shared::request::DeleteConfigRequest::fromRpcEnvelope(ctx, envelope_shared);
        auto response = netd::server::netconf::handlers::RpcHandler::handleDeleteConfigRequest(request.get());
        if (response) {
          auto reply_envelope = response->toRpcEnvelope(envelope_shared, ctx);
          std::string reply_xml = reply_envelope->toString(ctx);
          
          netd::shared::RpcTxStream tx_stream(rpc_stream.getSocket());
          tx_stream << reply_xml;
          tx_stream.flush();
        }
        break;
      }
      case netd::shared::netconf::NetconfOperation::LOCK: {
        auto request = netd::shared::request::LockRequest::fromRpcEnvelope(ctx, envelope_shared);
        auto response = netd::server::netconf::handlers::RpcHandler::handleLockRequest(request.get());
        if (response) {
          auto reply_envelope = response->toRpcEnvelope(envelope_shared, ctx);
          std::string reply_xml = reply_envelope->toString(ctx);
          
          netd::shared::RpcTxStream tx_stream(rpc_stream.getSocket());
          tx_stream << reply_xml;
          tx_stream.flush();
        }
        break;
      }
      case netd::shared::netconf::NetconfOperation::UNLOCK: {
        auto request = netd::shared::request::UnlockRequest::fromRpcEnvelope(ctx, envelope_shared);
        auto response = netd::server::netconf::handlers::RpcHandler::handleUnlockRequest(request.get());
        if (response) {
          auto reply_envelope = response->toRpcEnvelope(envelope_shared, ctx);
          std::string reply_xml = reply_envelope->toString(ctx);
          
          netd::shared::RpcTxStream tx_stream(rpc_stream.getSocket());
          tx_stream << reply_xml;
          tx_stream.flush();
        }
        break;
      }
      case netd::shared::netconf::NetconfOperation::COMMIT: {
        auto request = netd::shared::request::CommitRequest::fromRpcEnvelope(ctx, envelope_shared);
        auto response = netd::server::netconf::handlers::RpcHandler::handleCommitRequest(request.get());
        if (response) {
          auto reply_envelope = response->toRpcEnvelope(envelope_shared, ctx);
          std::string reply_xml = reply_envelope->toString(ctx);
          
          netd::shared::RpcTxStream tx_stream(rpc_stream.getSocket());
          tx_stream << reply_xml;
          tx_stream.flush();
        }
        break;
      }
      case netd::shared::netconf::NetconfOperation::DISCARD_CHANGES: {
        auto request = netd::shared::request::DiscardRequest::fromRpcEnvelope(ctx, envelope_shared);
        auto response = netd::server::netconf::handlers::RpcHandler::handleDiscardRequest(request.get());
        if (response) {
          auto reply_envelope = response->toRpcEnvelope(envelope_shared, ctx);
          std::string reply_xml = reply_envelope->toString(ctx);
          
          netd::shared::RpcTxStream tx_stream(rpc_stream.getSocket());
          tx_stream << reply_xml;
          tx_stream.flush();
        }
        break;
      }
      case netd::shared::netconf::NetconfOperation::VALIDATE: {
        auto request = netd::shared::request::ValidateRequest::fromRpcEnvelope(ctx, envelope_shared);
        auto response = netd::server::netconf::handlers::RpcHandler::handleValidateRequest(request.get());
        if (response) {
          auto reply_envelope = response->toRpcEnvelope(envelope_shared, ctx);
          std::string reply_xml = reply_envelope->toString(ctx);
          
          netd::shared::RpcTxStream tx_stream(rpc_stream.getSocket());
          tx_stream << reply_xml;
          tx_stream.flush();
        }
        break;
      }
      case netd::shared::netconf::NetconfOperation::CLOSE_SESSION: {
        auto request = netd::shared::request::session::CloseRequest::fromRpcEnvelope(ctx, envelope_shared);
        auto response = netd::server::netconf::handlers::RpcHandler::handleCloseSessionRequest(request.get());
        if (response) {
          auto reply_envelope = response->toRpcEnvelope(envelope_shared, ctx);
          std::string reply_xml = reply_envelope->toString(ctx);
          
          netd::shared::RpcTxStream tx_stream(rpc_stream.getSocket());
          tx_stream << reply_xml;
          tx_stream.flush();
        }
        break;
      }
      case netd::shared::netconf::NetconfOperation::KILL_SESSION: {
        auto request = netd::shared::request::session::DestroyRequest::fromRpcEnvelope(ctx, envelope_shared);
        auto response = netd::server::netconf::handlers::RpcHandler::handleDestroySessionRequest(request.get());
        if (response) {
          auto reply_envelope = response->toRpcEnvelope(envelope_shared, ctx);
          std::string reply_xml = reply_envelope->toString(ctx);
          
          netd::shared::RpcTxStream tx_stream(rpc_stream.getSocket());
          tx_stream << reply_xml;
          tx_stream.flush();
        }
        break;
      }
      default:
        throw netd::shared::NotImplementedError("Operation not implemented");
    }
    
    (void)message_id;
    (void)inner_data;
  }


} // namespace netd::server::netconf
