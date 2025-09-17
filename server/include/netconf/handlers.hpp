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

#ifndef NETD_SERVER_NETCONF_HANDLERS_HPP
#define NETD_SERVER_NETCONF_HANDLERS_HPP

#include <libyang/libyang.h>
#include <memory>
#include <shared/include/interface/base/ether.hpp>
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
#include <shared/include/request/session/close.hpp>
#include <shared/include/request/session/destroy.hpp>
#include <shared/include/request/unlock.hpp>
#include <shared/include/request/validate.hpp>
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
#include <vector>

namespace netd::server::netconf::handlers {

  class RpcHandler {
  public:
    // Get request handler
    static std::unique_ptr<netd::shared::response::get::GetResponse>
    handleGetRequest(netd::shared::request::get::GetRequest *request);

    // Get-config request handler
    static std::unique_ptr<netd::shared::response::get::GetConfigResponse>
    handleGetConfigRequest(
        netd::shared::request::get::GetConfigRequest *request);

    // Edit-config request handler
    static std::unique_ptr<netd::shared::response::EditConfigResponse>
    handleEditConfigRequest(netd::shared::request::EditConfigRequest *request);

    // Copy-config request handler
    static std::unique_ptr<netd::shared::response::CopyConfigResponse>
    handleCopyConfigRequest(netd::shared::request::CopyConfigRequest *request);

    // Delete-config request handler
    static std::unique_ptr<netd::shared::response::DeleteConfigResponse>
    handleDeleteConfigRequest(
        netd::shared::request::DeleteConfigRequest *request);

    // Lock request handler
    static std::unique_ptr<netd::shared::response::LockResponse>
    handleLockRequest(netd::shared::request::LockRequest *request);

    // Unlock request handler
    static std::unique_ptr<netd::shared::response::UnlockResponse>
    handleUnlockRequest(netd::shared::request::UnlockRequest *request);

    // Discard-changes request handler
    static std::unique_ptr<netd::shared::response::DiscardResponse>
    handleDiscardRequest(netd::shared::request::DiscardRequest *request);

    // Validate request handler
    static std::unique_ptr<netd::shared::response::ValidateResponse>
    handleValidateRequest(netd::shared::request::ValidateRequest *request);

    // Hello request handler
    static std::unique_ptr<netd::shared::response::HelloResponse>
    handleHelloRequest(netd::shared::request::HelloRequest *request);

    // Send server hello message
    static void sendServerHello(netd::shared::netconf::NetconfSession &session);

    // Commit request handler
    static std::unique_ptr<netd::shared::response::CommitResponse>
    handleCommitRequest(netd::shared::request::CommitRequest *request);

    // Session management handlers
    static std::unique_ptr<netd::shared::response::CloseResponse>
    handleDestroySessionRequest(
        netd::shared::request::session::DestroyRequest *request);

    static std::unique_ptr<netd::shared::response::CloseResponse>
    handleCloseSessionRequest(
        netd::shared::request::session::CloseRequest *request);

  private:
    // Interface-specific handler function
    static std::unique_ptr<netd::shared::response::get::GetConfigResponse>
    handleGetInterfaceRequest(
        netd::shared::request::get::GetConfigRequest *request);
  };

} // namespace netd::server::netconf::handlers

#endif // NETD_SERVER_NETCONF_HANDLERS_HPP
