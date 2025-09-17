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

#ifndef NETD_CLIENT_NETCONF_HANDLERS_HPP
#define NETD_CLIENT_NETCONF_HANDLERS_HPP

#include <shared/include/netconf/session.hpp>
#include <shared/include/request/base.hpp>
#include <shared/include/request/commit.hpp>
#include <shared/include/request/copy.hpp>
#include <shared/include/request/delete.hpp>
#include <shared/include/request/discard.hpp>
#include <shared/include/request/edit.hpp>
#include <shared/include/request/get/base.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/request/hello.hpp>
#include <shared/include/request/lock.hpp>
#include <shared/include/request/unlock.hpp>
#include <shared/include/request/validate.hpp>
#include <shared/include/request/session/destroy.hpp>
#include <shared/include/request/session/close.hpp>
#include <shared/include/response/base.hpp>
#include <shared/include/response/close.hpp>
#include <shared/include/response/commit.hpp>
#include <shared/include/response/copy.hpp>
#include <shared/include/response/delete.hpp>
#include <shared/include/response/discard.hpp>
#include <shared/include/response/edit.hpp>
#include <shared/include/response/get/base.hpp>
#include <shared/include/response/get/config.hpp>
#include <shared/include/response/hello.hpp>
#include <shared/include/response/lock.hpp>
#include <shared/include/response/unlock.hpp>
#include <shared/include/response/validate.hpp>
#include <string>
#include <memory>

namespace netd::client::netconf {

  class RpcHandler {
  public:
    static std::string handleRpc(const std::string& xml_request, 
                                netd::shared::netconf::NetconfSession* session);
    static std::unique_ptr<netd::shared::response::get::GetResponse> handleGetRequest(
        const netd::shared::request::get::GetRequest& request, 
        netd::shared::netconf::NetconfSession* session);
    
    static std::unique_ptr<netd::shared::response::get::GetConfigResponse> handleGetConfigRequest(
        const netd::shared::request::get::GetConfigRequest& request, 
        netd::shared::netconf::NetconfSession* session);
    
    static std::unique_ptr<netd::shared::response::EditConfigResponse> handleEditConfigRequest(
        const netd::shared::request::EditConfigRequest& request, 
        netd::shared::netconf::NetconfSession* session);
    
    static std::unique_ptr<netd::shared::response::CopyConfigResponse> handleCopyConfigRequest(
        const netd::shared::request::CopyConfigRequest& request, 
        netd::shared::netconf::NetconfSession* session);
    
    static std::unique_ptr<netd::shared::response::DeleteConfigResponse> handleDeleteConfigRequest(
        const netd::shared::request::DeleteConfigRequest& request, 
        netd::shared::netconf::NetconfSession* session);
    
    static std::unique_ptr<netd::shared::response::LockResponse> handleLockRequest(
        const netd::shared::request::LockRequest& request, 
        netd::shared::netconf::NetconfSession* session);
    
    static std::unique_ptr<netd::shared::response::UnlockResponse> handleUnlockRequest(
        const netd::shared::request::UnlockRequest& request, 
        netd::shared::netconf::NetconfSession* session);
    
    static std::unique_ptr<netd::shared::response::DiscardResponse> handleDiscardRequest(
        const netd::shared::request::DiscardRequest& request, 
        netd::shared::netconf::NetconfSession* session);
    
    static std::unique_ptr<netd::shared::response::ValidateResponse> handleValidateRequest(
        const netd::shared::request::ValidateRequest& request, 
        netd::shared::netconf::NetconfSession* session);
    
    static std::unique_ptr<netd::shared::response::HelloResponse> handleHelloRequest(
        const netd::shared::request::HelloRequest& request, 
        netd::shared::netconf::NetconfSession* session);
    
    static std::unique_ptr<netd::shared::response::CommitResponse> handleCommitRequest(
        const netd::shared::request::CommitRequest& request, 
        netd::shared::netconf::NetconfSession* session);
    
    static std::unique_ptr<netd::shared::response::CloseResponse> handleCloseSessionRequest(
        const netd::shared::request::session::CloseRequest& request, 
        netd::shared::netconf::NetconfSession* session);
    
    static std::unique_ptr<netd::shared::response::CloseResponse> handleDestroySessionRequest(
        const netd::shared::request::session::DestroyRequest& request, 
        netd::shared::netconf::NetconfSession* session);

  };

} // namespace netd::client::netconf

#endif // NETD_CLIENT_NETCONF_HANDLERS_HPP
