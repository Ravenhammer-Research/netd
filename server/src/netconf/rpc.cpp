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

#include <libyang/libyang.h>
#include <libyang/parser_data.h>
#include <server/include/netconf/rpc.hpp>
#include <server/include/netconf/handlers.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/yang.hpp>
#include <shared/include/netconf/rpc.hpp>
#include <shared/include/request/hello.hpp>
#include <shared/include/request/commit.hpp>
#include <shared/include/request/get/base.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/request/edit.hpp>
#include <shared/include/request/copy.hpp>
#include <shared/include/request/delete.hpp>
#include <shared/include/request/discard.hpp>
#include <shared/include/request/session/close.hpp>
#include <shared/include/request/session/destroy.hpp>
#include <shared/include/request/lock.hpp>
#include <shared/include/request/unlock.hpp>
#include <shared/include/request/validate.hpp>
#include <shared/include/marshalling/error.hpp>
#include <shared/include/exception.hpp>
#include <memory>
#include <string>

namespace netd::server::netconf {

NetconfRpc::NetconfRpc() = default;

std::string NetconfRpc::handleRpc(const std::string& xml, netd::shared::netconf::NetconfSession& session) {
    auto &logger = netd::shared::Logger::getInstance();
    
    try {
        // Get YANG context from session
        ly_ctx* ctx = session.getContext();
        if (!ctx) {
            logger.error("Failed to get YANG context from session");
            // Can't create proper RPC reply without context, so throw exception
            throw netd::shared::RpcException("Internal server error: Failed to get YANG context from session");
        }

        // Create input stream from XML string
        struct ly_in* in = nullptr;
        if (ly_in_new_memory(xml.c_str(), &in) != LY_SUCCESS) {
            logger.error("Failed to create input stream from XML");
            auto error = std::make_unique<netd::shared::marshalling::Error>(
                netd::shared::marshalling::ErrorType::PROTOCOL,
                netd::shared::marshalling::ErrorTag::MALFORMED_MESSAGE,
                netd::shared::marshalling::ErrorSeverity::ERROR);
            error->setMessage("Invalid XML format: Failed to create input stream from XML data");
            lyd_node* error_tree = error->toYang(ctx);
            auto rpc_reply = netd::shared::netconf::Rpc::createRpcReply(ctx, "1", error_tree);
            return rpc_reply->toString();
        }

        // Parse the RPC using lyd_parse_op
        struct lyd_node* tree = nullptr;
        struct lyd_node* op = nullptr;
        
        LY_ERR result = lyd_parse_op(ctx, nullptr, in, LYD_XML, LYD_TYPE_RPC_NETCONF, &tree, &op);
        ly_in_free(in, 0);
        
        if (result != LY_SUCCESS) {
            logger.error("Failed to parse RPC XML");
            if (tree) {
                lyd_free_tree(tree);
            }
            auto error = std::make_unique<netd::shared::marshalling::Error>(
                netd::shared::marshalling::ErrorType::PROTOCOL,
                netd::shared::marshalling::ErrorTag::MALFORMED_MESSAGE,
                netd::shared::marshalling::ErrorSeverity::ERROR);
            error->setMessage("Failed to parse RPC: Invalid XML structure or syntax");
            lyd_node* error_tree = error->toYang(ctx);
            auto rpc_reply = netd::shared::netconf::Rpc::createRpcReply(ctx, "1", error_tree);
            return rpc_reply->toString();
        }

        if (!op) {
            logger.error("No operation found in RPC");
            if (tree) {
                lyd_free_tree(tree);
            }
            auto error = std::make_unique<netd::shared::marshalling::Error>(
                netd::shared::marshalling::ErrorType::PROTOCOL,
                netd::shared::marshalling::ErrorTag::MALFORMED_MESSAGE,
                netd::shared::marshalling::ErrorSeverity::ERROR);
            error->setMessage("No operation found: RPC message missing operation element");
            lyd_node* error_tree = error->toYang(ctx);
            auto rpc_reply = netd::shared::netconf::Rpc::createRpcReply(ctx, "1", error_tree);
            return rpc_reply->toString();
        }

        // Extract message ID from tree (NETCONF envelope)
        std::string request_message_id = "1"; // default
        if (tree) {
            struct lyd_node* msg_id_node = nullptr;
            if (lyd_find_path(tree, "message-id", 0, &msg_id_node) == LY_SUCCESS && msg_id_node) {
                request_message_id = lyd_get_value(msg_id_node);
            }
        }
        
        // Response message ID is request message ID + 1
        std::string response_message_id = std::to_string(std::stoi(request_message_id) + 1);

        // Get operation name
        const char* op_name = lyd_node_schema(op)->name;
        if (!op_name) {
            logger.error("Failed to get operation name");
            lyd_free_tree(tree);
            lyd_free_tree(op);
            auto error = std::make_unique<netd::shared::marshalling::Error>(
                netd::shared::marshalling::ErrorType::PROTOCOL,
                netd::shared::marshalling::ErrorTag::MALFORMED_MESSAGE,
                netd::shared::marshalling::ErrorSeverity::ERROR);
            error->setMessage("Invalid operation: Failed to extract operation name from RPC");
            lyd_node* error_tree = error->toYang(ctx);
            auto rpc_reply = netd::shared::netconf::Rpc::createRpcReply(ctx, response_message_id, error_tree);
            return rpc_reply->toString();
        }

        logger.info("Processing RPC: " + std::string(op_name) + " (message-id: " + request_message_id + ")");

        // Route to appropriate handler based on operation name
        lyd_node* response_tree = nullptr;
        
        if (strcmp(op_name, "hello") == 0) {
            auto request = std::make_unique<netd::shared::request::HelloRequest>(&session, op);
            auto response = netd::server::netconf::handlers::RpcHandler::handleHelloRequest(std::move(request));
            response_tree = response->toYang(ctx);
        }
        else if (strcmp(op_name, "close-session") == 0) {
            auto request = std::make_unique<netd::shared::request::session::CloseRequest>(&session, op);
            auto response = netd::server::netconf::handlers::RpcHandler::handleCloseSessionRequest(std::move(request));
            response_tree = response->toYang(ctx);
        }
        else if (strcmp(op_name, "commit") == 0) {
            auto request = std::make_unique<netd::shared::request::CommitRequest>(&session, op);
            auto response = netd::server::netconf::handlers::RpcHandler::handleCommitRequest(std::move(request));
            response_tree = response->toYang(ctx);
        }
        else if (strcmp(op_name, "get") == 0) {
            auto request = std::make_unique<netd::shared::request::get::GetRequest>(&session, op);
            auto response = netd::server::netconf::handlers::RpcHandler::handleGetRequest(std::move(request));
            response_tree = response->toYang(ctx);
        }
        else if (strcmp(op_name, "get-config") == 0) {
            auto request = std::make_unique<netd::shared::request::get::GetConfigRequest>(&session, op);
            auto response = netd::server::netconf::handlers::RpcHandler::handleGetConfigRequest(std::move(request));
            response_tree = response->toYang(ctx);
        }
        else if (strcmp(op_name, "edit-config") == 0) {
            auto request = std::make_unique<netd::shared::request::EditConfigRequest>(&session, op);
            auto response = netd::server::netconf::handlers::RpcHandler::handleEditConfigRequest(std::move(request));
            response_tree = response->toYang(ctx);
        }
        else if (strcmp(op_name, "copy-config") == 0) {
            auto request = std::make_unique<netd::shared::request::CopyConfigRequest>(&session, op);
            auto response = netd::server::netconf::handlers::RpcHandler::handleCopyConfigRequest(std::move(request));
            response_tree = response->toYang(ctx);
        }
        else if (strcmp(op_name, "delete-config") == 0) {
            auto request = std::make_unique<netd::shared::request::DeleteConfigRequest>(&session, op);
            auto response = netd::server::netconf::handlers::RpcHandler::handleDeleteConfigRequest(std::move(request));
            response_tree = response->toYang(ctx);
        }
        else if (strcmp(op_name, "lock") == 0) {
            auto request = std::make_unique<netd::shared::request::LockRequest>(&session, op);
            auto response = netd::server::netconf::handlers::RpcHandler::handleLockRequest(std::move(request));
            response_tree = response->toYang(ctx);
        }
        else if (strcmp(op_name, "unlock") == 0) {
            auto request = std::make_unique<netd::shared::request::UnlockRequest>(&session, op);
            auto response = netd::server::netconf::handlers::RpcHandler::handleUnlockRequest(std::move(request));
            response_tree = response->toYang(ctx);
        }
        else if (strcmp(op_name, "discard-changes") == 0) {
            auto request = std::make_unique<netd::shared::request::DiscardRequest>(&session, op);
            auto response = netd::server::netconf::handlers::RpcHandler::handleDiscardRequest(std::move(request));
            response_tree = response->toYang(ctx);
        }
        else if (strcmp(op_name, "kill-session") == 0) {
            auto request = std::make_unique<netd::shared::request::session::DestroyRequest>(&session, op);
            auto response = netd::server::netconf::handlers::RpcHandler::handleDestroySessionRequest(std::move(request));
            response_tree = response->toYang(ctx);
        }
        else if (strcmp(op_name, "validate") == 0) {
            auto request = std::make_unique<netd::shared::request::ValidateRequest>(&session, op);
            auto response = netd::server::netconf::handlers::RpcHandler::handleValidateRequest(std::move(request));
            response_tree = response->toYang(ctx);
        }
        else {
            logger.warning("Unknown RPC operation: " + std::string(op_name));
            auto error = std::make_unique<netd::shared::marshalling::Error>(
                netd::shared::marshalling::ErrorType::PROTOCOL,
                netd::shared::marshalling::ErrorTag::OPERATION_NOT_SUPPORTED,
                netd::shared::marshalling::ErrorSeverity::ERROR);
            error->setMessage("Operation '" + std::string(op_name) + "' not supported");
            lyd_node* error_tree = error->toYang(ctx);
            auto rpc_reply = netd::shared::netconf::Rpc::createRpcReply(ctx, response_message_id, error_tree);
            return rpc_reply->toString();
        }

        // Convert response tree to XML string using createRpcReply
        std::string response_xml;
        if (response_tree) {
            // For successful responses, use the response object's message ID if available
            // For now, we'll use request ID + 1 as the response ID
            auto rpc_reply = netd::shared::netconf::Rpc::createRpcReply(ctx, response_message_id, response_tree);
            response_xml = rpc_reply->toString();
        } else {
            logger.error("No response tree generated");
            auto error = std::make_unique<netd::shared::marshalling::Error>(
                netd::shared::marshalling::ErrorType::PROTOCOL,
                netd::shared::marshalling::ErrorTag::OPERATION_FAILED,
                netd::shared::marshalling::ErrorSeverity::ERROR);
            error->setMessage("No response generated: Response object toYang() returned null tree");
            lyd_node* error_tree = error->toYang(ctx);
            auto rpc_reply = netd::shared::netconf::Rpc::createRpcReply(ctx, response_message_id, error_tree);
            response_xml = rpc_reply->toString();
        }

        // Clean up
        lyd_free_tree(tree);
        lyd_free_tree(op);

        return response_xml;

    } catch (const std::exception& e) {
        // Rethrow RpcException to let it propagate
        if (dynamic_cast<const netd::shared::RpcException*>(&e)) {
            throw;
        }
        
        logger.error("Exception in handleRpc: " + std::string(e.what()));
        // Get context from session for error handling
        ly_ctx* ctx = session.getContext();
        if (ctx) {
            auto error = std::make_unique<netd::shared::marshalling::Error>(
                netd::shared::marshalling::ErrorType::PROTOCOL,
                netd::shared::marshalling::ErrorTag::OPERATION_FAILED,
                netd::shared::marshalling::ErrorSeverity::ERROR);
            error->setMessage("Internal server error: " + std::string(e.what()));
            lyd_node* error_tree = error->toYang(ctx);
            auto rpc_reply = netd::shared::netconf::Rpc::createRpcReply(ctx, "1", error_tree);
            return rpc_reply->toString();
        }
        // Fallback if no context available
        throw netd::shared::RpcException("No YANG context available for error handling: Session context is null");
    }
}

} // namespace netd::server::netconf