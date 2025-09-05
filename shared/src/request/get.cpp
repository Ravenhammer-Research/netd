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

#include <shared/include/request/get.hpp>
#include <shared/include/yang.hpp>
#include <shared/include/exception.hpp>
#include <libyang/libyang.h>
#include <libyang/tree_data.h>

namespace netd::shared::request {

    GetRequest::GetRequest() {
    }

    GetRequest::~GetRequest() {
    }

    lyd_node* GetRequest::toYang(ly_ctx* ctx) const {
        if (!ctx) {
            return nullptr;
        }
        
        // Get the ietf-netconf module
        const struct lys_module* mod = ly_ctx_get_module_implemented(ctx, "ietf-netconf");
        if (!mod) {
            return nullptr;
        }
        
        // Create the complete RPC structure with envelope
        lyd_node* rpcNode = nullptr;
        if (lyd_new_inner(nullptr, mod, "rpc", 0, &rpcNode) != LY_SUCCESS) {
            return nullptr;
        }
        
        // Add message-id attribute to the RPC envelope
        if (lyd_new_meta(nullptr, rpcNode, nullptr, "message-id", "1", 0, nullptr) != LY_SUCCESS) {
            lyd_free_tree(rpcNode);
            return nullptr;
        }
        
        // Create the get operation inside the RPC
        lyd_node* getNode = nullptr;
        if (lyd_new_inner(rpcNode, mod, "get", 0, &getNode) != LY_SUCCESS) {
            lyd_free_tree(rpcNode);
            return nullptr;
        }
        
        return rpcNode;
    }

    std::unique_ptr<Request> GetRequest::fromYang(const ly_ctx* ctx, const lyd_node* node) {
        if (!node) {
            throw NotImplementedError("Invalid YANG node provided to GetRequest::fromYang");
        }
        
        return std::make_unique<GetRequest>();
    }

} // namespace netd::shared::request
