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

#ifndef NETD_REQUEST_BASE_HPP
#define NETD_REQUEST_BASE_HPP

#include <libyang/libyang.h>
#include <memory>
#include <shared/include/base/serialization.hpp>
#include <shared/include/marshalling/filter.hpp>
#include <shared/include/netconf/session.hpp>
#include <shared/include/xml/base.hpp>
#include <shared/include/xml/envelope.hpp>
#include <string>

namespace netd::shared::request {

  // Source datastore enumeration
  enum class Source { RUNNING, CANDIDATE, STARTUP };

  template <typename T> class Request : public netd::shared::base::Serialization<Request<T>> {
  public:
    Request() = default;
    Request(netd::shared::netconf::NetconfSession *session, struct lyd_node *rpc)
        : session_(session), rpc_(rpc) {}
    virtual ~Request() = default;

    // Pure virtual methods that must be implemented by subclasses
    virtual lyd_node *toYang(ly_ctx *ctx) const = 0;
    static std::unique_ptr<T> fromYang(const ly_ctx *ctx,
                                       const lyd_node *node);
    static std::unique_ptr<T> fromRpcEnvelope(const ly_ctx *ctx,
                                              std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope);
    
    // Convert to XML string
    std::string toXml() const;
    std::string toRpcXml() const;

    // Session and RPC accessors
    netd::shared::netconf::NetconfSession *getSession() const { return session_; }
    struct lyd_node *getRpc() const { return rpc_; }
    void setSession(netd::shared::netconf::NetconfSession *session) { session_ = session; }
    void setRpc(struct lyd_node *rpc) { rpc_ = rpc; }

  protected:
    // NETCONF session and RPC node
    netd::shared::netconf::NetconfSession *session_ = nullptr;
    struct lyd_node *rpc_ = nullptr;
    std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope_ = nullptr;

    // RPC request properties
    std::string messageId = "1";
    std::string xmlns = "urn:ietf:params:xml:ns:netconf:base:1.0";
    Source source = Source::RUNNING;
    std::unique_ptr<netd::shared::marshalling::Filter> filter = nullptr;

    // Helper methods for property access
    void setMessageId(const std::string &id) { messageId = id; }
    const std::string &getMessageId() const { return messageId; }

    void setXmlns(const std::string &ns) { xmlns = ns; }
    const std::string &getXmlns() const { return xmlns; }

    void setSource(Source src) { source = src; }
    Source getSource() const { return source; }

    void setFilter(std::unique_ptr<netd::shared::marshalling::Filter> f) {
      filter = std::move(f);
    }
    netd::shared::marshalling::Filter *getFilter() const {
      return filter.get();
    }

    // Helper method to convert Source enum to string
    std::string sourceToString() const {
      switch (source) {
      case Source::RUNNING:
        return "running";
      case Source::CANDIDATE:
        return "candidate";
      case Source::STARTUP:
        return "startup";
      default:
        return "running";
      }
    }
  };

} // namespace netd::shared::request

#endif // NETD_REQUEST_BASE_HPP
