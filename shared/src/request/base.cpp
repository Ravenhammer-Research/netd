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
#include <libyang/tree_data.h>
#include <shared/include/request/base.hpp>
#include <shared/include/request/get/config.hpp>
#include <shared/include/yang.hpp>
#include <shared/include/exception.hpp>

namespace netd::shared::request {

  // Base Request class implementation
  // Template class - implementations are in the header
  
  // toXml implementation for template class
  template<typename T>
  std::string Request<T>::toXml() const {
    // Get YANG context from singleton
    auto &yang = netd::shared::Yang::getInstance();
    ly_ctx *ctx = yang.getContext();
    
    lyd_node *rpcNode = toYang(ctx);
    if (!rpcNode) {
      throw netd::shared::ArgumentError("Failed to convert request to YANG");
    }
    
    char *xmlStr = nullptr;
    if (lyd_print_mem(&xmlStr, rpcNode, LYD_XML, LYD_PRINT_WITHSIBLINGS) != LY_SUCCESS) {
      lyd_free_tree(rpcNode);
      throw netd::shared::ArgumentError("Failed to convert request to XML");
    }
    
    std::string result(xmlStr);
    free(xmlStr);
    lyd_free_tree(rpcNode);
    
    return result;
  }

  // toRpc implementation for template class
  template<typename T>
  struct nc_rpc *Request<T>::toRpc() const {
    // Convert to XML first
    std::string xmlRequest = toXml();
    
    // Create nc_rpc using act_generic with XML string
    struct nc_rpc *rpc = nc_rpc_act_generic_xml(xmlRequest.c_str(), NC_PARAMTYPE_FREE);
    
    if (!rpc) {
      throw netd::shared::NetworkError("Failed to create RPC from XML: " + xmlRequest);
    }
    
    return rpc;
  }

  // Explicit template instantiation for GetConfigRequest
  template class Request<get::GetConfigRequest>;

} // namespace netd::shared::request
