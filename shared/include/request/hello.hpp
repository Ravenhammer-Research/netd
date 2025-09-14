#ifndef NETD_REQUEST_HELLO_HPP
#define NETD_REQUEST_HELLO_HPP

#include <shared/include/request/base.hpp>
#include <shared/include/xml/base.hpp>
#include <shared/include/xml/envelope.hpp>
#include <shared/include/xml/hello.hpp>

namespace netd::shared::request {

  class HelloRequest : public Request<HelloRequest> {
  public:
    HelloRequest() : Request<HelloRequest>() {}
    HelloRequest(netd::shared::netconf::NetconfSession *session, struct lyd_node *rpc)
        : Request<HelloRequest>(session, rpc) {}
    virtual ~HelloRequest() = default;

    lyd_node *toYang(ly_ctx *ctx) const ;
    static std::unique_ptr<HelloRequest> fromYang(const ly_ctx *ctx,
                                                 const lyd_node *node) ;
    static std::unique_ptr<HelloRequest> fromRpcEnvelope(const ly_ctx *ctx,
                                                        std::shared_ptr<netd::shared::xml::RpcEnvelope> envelope) ;
    static std::unique_ptr<HelloRequest> fromHelloToServer(const netd::shared::xml::HelloToServer& hello_to_server) ;
    
    // Get session ID from hello request (if present)
    int getSessionId() const { return session_id_; }
    
    // Get capabilities from hello request
    const std::vector<std::string>& getCapabilities() const { return capabilities_; }

  private:
    std::vector<std::string> capabilities_;
    int session_id_ = -1; // -1 means no session ID provided
  };

} // namespace netd::shared::request

#endif // NETD_REQUEST_HELLO_HPP
