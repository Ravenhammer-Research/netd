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

#include <libyang/libyang.h>
#include <string>
#include <memory>

namespace netd::shared::netconf {

  class RpcData {
  public:
    RpcData(lyd_node* node);
    ~RpcData();

    // Convert to XML string
    std::string toString() const;

    // Get the underlying YANG node
    lyd_node* getNode() const { return node_; }

  private:
    lyd_node* node_;
  };

  class Rpc {
  public:
    // Create RPC request envelope with child data
    static std::unique_ptr<RpcData> createRpcRequest(ly_ctx* ctx, 
                                                    const std::string& message_id,
                                                    const std::string& operation_name,
                                                    lyd_node* child_data = nullptr);

    // Create RPC reply envelope with child data
    static std::unique_ptr<RpcData> createRpcReply(ly_ctx* ctx,
                                                  const std::string& message_id,
                                                  lyd_node* child_data = nullptr);

    // Extract data from RPC envelope (handles both <rpc> and <rpc-reply>)
    static lyd_node* extractFromEnvelope(const std::string& rpc_xml);

  private:
    Rpc() = delete;
    ~Rpc() = delete;
  };

} // namespace netd::shared::netconf

#endif // NETD_SHARED_NETCONF_RPC_HPP
