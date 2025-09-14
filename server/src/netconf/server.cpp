#include <server/include/netconf/server.hpp>
#include <shared/include/exception.hpp>
#include <server/include/netconf/session.hpp>
#include <shared/include/netconf/rpc.hpp>
#include <server/include/netconf/rpc.hpp>
#include <shared/include/stream.hpp>
#include <shared/include/socket.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/yang.hpp>
#include <server/include/signal.hpp>
#include <shared/include/xml/hello.hpp>
#include <shared/include/request/hello.hpp>
#include <shared/include/unix.hpp>
#ifdef HAVE_OPENSSL
#include <shared/include/tls.hpp>
#include <shared/include/dtls.hpp>
#endif
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>

namespace netd::server::netconf {

  NetconfServer::NetconfServer(netd::shared::TransportType transport_type, 
                               const std::string& bind_address, 
                               int port)
      : transport_type_(transport_type), bind_address_(bind_address), port_(port) {
  }

  NetconfServer::~NetconfServer() {
    stop();
    
    for (auto& thread : session_threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    session_threads_.clear();
  }

  std::unique_ptr<netd::shared::BaseTransport> NetconfServer::createTransport() {
    return netd::shared::BaseTransport::create(transport_type_);
  }

  bool NetconfServer::start() {
    if (isListening()) {
      return true;
    }

    transport_ = createTransport();
    if (!transport_) {
      throw netd::shared::TransportError("Failed to create transport");
    }

    std::string formatted_address = netd::shared::BaseTransport::formatAddress(transport_type_, bind_address_, port_);

    if (!transport_->start(formatted_address)) {
      throw netd::shared::BindError("Failed to start transport");
    }

    initializeLLDP();

    return true;
  }

  bool NetconfServer::isListening() const {
    return transport_ && transport_->isListening();
  }

  void NetconfServer::stop() {
    cleanupLLDP();

    if (transport_) {
      transport_->stop();
      transport_.reset();
    }

    for (auto& thread : session_threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    session_threads_.clear();
  }

  void NetconfServer::run() {
    if (!transport_) {
      throw netd::shared::TransportError("Transport not available");
    }

    while (netd::server::isRunning()) {
      int client_socket = transport_->acceptConnection();
      
      if (client_socket < 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        continue;
      }

      session_threads_.emplace_back([this, client_socket]() {
        try {
          netd::shared::ClientSocket client_socket_obj(client_socket);
          auto session = this->handleClientSession(client_socket_obj);
          this->sendHello(client_socket_obj, session);
          netd::shared::RpcRxStream rpc_stream(client_socket_obj);
          this->rpcRequestReceiveWait(rpc_stream, session);
          client_socket_obj.close();
        } catch (const std::exception& e) {
          // Log the error and close the socket
          auto& logger = netd::shared::Logger::getInstance();
          logger.error("netd::server::netconf::NetconfServer::run: " + std::string(e.what()));
          close(client_socket);
        }
      });
    }
  }

  void NetconfServer::sendHello(
    const netd::shared::ClientSocket& client_socket, 
    netd::shared::netconf::NetconfSession* session) {
    
    if (!session) {
      throw netd::shared::ArgumentError("session not found");
    }

    auto yang_ctx = session->getContext();
    
    auto capabilities = netd::shared::Yang::getInstance().getCapabilities();
    auto hello = netd::shared::xml::HelloToClient::toXml(
      session->getSessionId(), 
      capabilities, 
      yang_ctx);
    
    std::string xml_str = hello->toString(yang_ctx);
    
    netd::shared::RpcTxStream tx_stream(const_cast<netd::shared::ClientSocket&>(client_socket));
    
    tx_stream << xml_str;
    tx_stream.flush();
  }

  netd::shared::netconf::NetconfSession* NetconfServer::handleClientSession(
    const netd::shared::ClientSocket& client_socket) {

    auto& session_manager = SessionManager::getInstance();
    netd::shared::netconf::NetconfSession* session;
    
    if (transport_type_ == netd::shared::TransportType::UNIX) {
      uid_t user_id = client_socket.getUserId();
      auto* existing_session = session_manager.findSessionByUserId(user_id);
      
      if (existing_session) {
        existing_session->updateSocket(client_socket.getSocket());
        session = existing_session;
      } else {
        auto& yang = netd::shared::Yang::getInstance();        
        ly_ctx* ctx = yang.getContext();

        auto new_session = std::make_unique<netd::shared::netconf::NetconfSession>(
            ctx, client_socket.getSocket(), transport_type_);

        new_session->setUserId(user_id);
        
        session = new_session.get();
        session_manager.addSession(std::move(new_session));
      }
    } else {
      throw netd::shared::NotImplementedError("Transport type not implemented");
    }
    
    return session;
  }

  void NetconfServer::rpcRequestReceiveWait(
    netd::shared::RpcRxStream& rpc_stream, 
    netd::shared::netconf::NetconfSession* session) {
    
    if (!session) {
      return;
    }

    wait:
    if (rpc_stream.hasData()) {
      std::string xml = rpc_stream.readToEnd();
      
      if (netd::shared::xml::isRpcMessage(xml)) {
        rpc_stream.rewind();
        netd::server::netconf::ServerRpc::processRpc(rpc_stream, session);
      } else if (netd::shared::xml::isHelloMessage(xml)) {
        auto yang_ctx = session->getContext();
        auto client_hello = netd::shared::xml::HelloToServer::fromXml(xml, yang_ctx);
        auto hello_request = netd::shared::request::HelloRequest::fromHelloToServer(*client_hello);
        session->processHelloRequest(*hello_request);
        goto wait;
      }
    }
  }

} // namespace netd::server::netconf
