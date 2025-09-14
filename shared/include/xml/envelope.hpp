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

#ifndef NETD_SHARED_XML_ENVELOPE_HPP
#define NETD_SHARED_XML_ENVELOPE_HPP

#include <bsdxml.h>
#include <libyang/libyang.h>
#include <memory>
#include <sstream>
#include <string>
#include <shared/include/marshalling/filter.hpp>
#include <shared/include/netconf/rpc.hpp>
#include <shared/include/xml/base.hpp>

namespace netd::shared::xml {

  // RPC type enumeration
  enum class RpcType {
    RPC,        // <rpc> request
    RPC_REPLY,  // <rpc-reply> response
    RPC_ERROR   // <rpc-error> error response
  };
  
  // RPC envelope class for XML serialization/deserialization
  class RpcEnvelope : public XMLTree {
  public:
    RpcEnvelope();
    virtual ~RpcEnvelope() = default;

    // Parse XML string into RpcEnvelope
    static std::unique_ptr<RpcEnvelope> fromXml(const std::string& xml, const struct ly_ctx* ctx);

    // Generate RpcEnvelope from parameters
    static std::unique_ptr<RpcEnvelope> toXml(RpcType rpc_type, 
                                             int message_id, 
                                             netconf::NetconfOperation operation,
                                             marshalling::Filter* filter,
                                             lyd_node* lyd_data,
                                             const struct ly_ctx* ctx);

    // Override XMLTree methods
    std::stringstream toXmlStream(const struct ly_ctx* ctx) const override;

    // Getters and setters
    RpcType getRpcType() const { return rpc_type_; }
    void setRpcType(RpcType type) { rpc_type_ = type; }

    int getMessageId() const { return message_id_; }
    void setMessageId(int id) { message_id_ = id; }

    netconf::NetconfOperation getOperation() const { return operation_; }
    void setOperation(netconf::NetconfOperation op) { operation_ = op; }

    const std::unique_ptr<marshalling::Filter>& getFilter() const { return filter_; }
    void setFilter(std::unique_ptr<marshalling::Filter> filter) { filter_ = std::move(filter); }

    lyd_node* getLydData() const { return lyd_data_; }
    void setLydData(lyd_node* data) { lyd_data_ = data; }

  protected:
    // Override XMLTree protected methods
    void startElementHandler(void* userData, const XML_Char* name, const XML_Char** atts) override;
    void endElementHandler(void* userData, const XML_Char* name) override;
    void characterDataHandler(void* userData, const XML_Char* s, int len) override;

  private:
    RpcType rpc_type_ = RpcType::RPC;
    int message_id_ = 0;
    netconf::NetconfOperation operation_ = netconf::NetconfOperation::GET;
    std::unique_ptr<marshalling::Filter> filter_;
    lyd_node* lyd_data_ = nullptr;
  };

} // namespace netd::shared::xml

#endif // NETD_SHARED_XML_ENVELOPE_HPP
