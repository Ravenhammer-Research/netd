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

#ifndef NETD_RESPONSE_BASE_HPP
#define NETD_RESPONSE_BASE_HPP

#include <libyang/libyang.h>
#include <memory>
#include <shared/include/base/serialization.hpp>
#include <shared/include/marshalling/data.hpp>
#include <shared/include/marshalling/error.hpp>
#include <shared/include/netconf/session.hpp>
#include <string>

namespace netd::shared::response {

  class Response : public netd::shared::base::Serialization<Response> {
  public:
    Response();
    Response(Response &&other) noexcept;
    Response &operator=(Response &&other) noexcept;
    virtual ~Response() = default;

    // Pure virtual methods that must be implemented by subclasses
    virtual lyd_node *toYang(ly_ctx *ctx) const = 0;
    virtual std::unique_ptr<Response> fromYang(const ly_ctx *ctx,
                                               const lyd_node *node) = 0;


    // Error and data properties for flexible initialization
    std::unique_ptr<netd::shared::marshalling::Error> error = nullptr;
    std::unique_ptr<netd::shared::marshalling::Data> data = nullptr;

    // Helper methods for initialization
    void setError(std::unique_ptr<netd::shared::marshalling::Error> err) {
      error = std::move(err);
    }
    void setData(std::unique_ptr<netd::shared::marshalling::Data> d) {
      data = std::move(d);
    }

    // Set YANG data tree
    void setData(struct lyd_node *yang_data);

    netd::shared::marshalling::Error *getError() const { return error.get(); }
    netd::shared::marshalling::Data *getData() const { return data.get(); }
    
    // Check if response has an error
    bool isError() const { return error != nullptr; }

    // Convenience methods for common error types
    void setProtocolError(netd::shared::marshalling::ErrorTag tag,
                          const std::string &message = "");
    void setApplicationError(netd::shared::marshalling::ErrorTag tag,
                             const std::string &message = "");
    void setRpcError(netd::shared::marshalling::ErrorTag tag,
                     const std::string &message = "");
    void setTransportError(netd::shared::marshalling::ErrorTag tag,
                           const std::string &message = "");

  };

} // namespace netd::shared::response

#endif // NETD_RESPONSE_BASE_HPP
