#include <server/include/netconf/rpc.hpp>
#include <server/include/netconf/server.hpp>
#include <server/include/netconf/session.hpp>
#include <server/include/signal.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/netconf/rpc/rpc.hpp>
#include <shared/include/netconf/rpc/stream.hpp>
#include <shared/include/request/hello.hpp>
#include <shared/include/socket.hpp>
#include <shared/include/unix.hpp>
#include <shared/include/yang.hpp>
#ifdef HAVE_OPENSSL
#include <shared/include/dtls.hpp>
#include <shared/include/tls.hpp>
#endif
#include <chrono>
#include <cstring>
#include <errno.h>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

namespace netd::server::netconf {

  NetconfServer::NetconfServer(netd::shared::TransportType transport_type,
                               const std::string &bind_address, int port)
      : transport_type_(transport_type), bind_address_(bind_address),
        port_(port) {}

  NetconfServer::~NetconfServer() {
    stop();

    for (auto &thread : session_threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    session_threads_.clear();
  }

  std::unique_ptr<netd::shared::BaseTransport>
  NetconfServer::createTransport() {
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

    std::string formatted_address = netd::shared::BaseTransport::formatAddress(
        transport_type_, bind_address_, port_);

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

    for (auto &thread : session_threads_) {
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
        netd::shared::ClientSocket client_socket_obj(client_socket);
        auto session = this->handleClientSession(client_socket_obj);

        netd::shared::netconf::Rpc::sendHelloToClient(client_socket_obj,
                                                      session);
        netd::shared::RpcRxStream rpc_stream(client_socket_obj);

        // Try to receive messages up to 3 times
        auto &logger = netd::shared::Logger::getInstance();
        for (int attempt = 1; attempt <= 3; attempt++) {
          logger.debug("server: Attempt " + std::to_string(attempt) +
                       " to receive message");

          if (rpc_stream.hasData()) {
            try {
              this->rpcReceive(rpc_stream, session);
              attempt = 1;
            } catch (const std::exception &e) {
              logger.error("server: Exception in rpcReceive: " +
                           std::string(e.what()));
              return;
            }
          } else {
            logger.debug("Error in rpcReceive: " +
                         std::to_string(attempt));
          }

          if (attempt < 3) {
            logger.debug("server: Sleeping 1 second before next attempt");
            std::this_thread::sleep_for(std::chrono::seconds(1));
          }
        }

        client_socket_obj.close();
      });
    }
  }

  netd::shared::netconf::NetconfSession *NetconfServer::handleClientSession(
      const netd::shared::ClientSocket &client_socket) {

    auto &session_manager = SessionManager::getInstance();
    netd::shared::netconf::NetconfSession *session;

    if (transport_type_ == netd::shared::TransportType::UNIX) {
      uid_t user_id = client_socket.getUserId();
      auto *existing_session = session_manager.findSessionByUserId(user_id);

      if (existing_session) {
        existing_session->updateSocket(client_socket.getSocket());
        session = existing_session;
      } else {
        auto &yang = netd::shared::Yang::getInstance();
        ly_ctx *ctx = yang.getContext();

        auto new_session =
            std::make_unique<netd::shared::netconf::NetconfSession>(
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

  void
  NetconfServer::rpcReceive(netd::shared::RpcRxStream &rpc_stream,
                            netd::shared::netconf::NetconfSession *session) {

    auto &logger = netd::shared::Logger::getInstance();

    if (!session) {
      throw netd::shared::SessionError("session not found");
    }

    if (rpc_stream.hasData()) {
      std::string xml = rpc_stream.readNextMessage();
      if (netd::shared::xml::isRpcMessage(xml)) {
        rpc_stream.rewindOne();
        logger.debug("rpcReceive: Processing RPC message");
        netd::server::netconf::ServerRpc::processRpc(rpc_stream, session);
      } else if (netd::shared::xml::isHelloMessage(xml)) {
        logger.debug("rpcReceive: Processing client hello message");
        auto yang_ctx = session->getContext();
        auto client_hello =
            netd::shared::xml::HelloToServer::fromXml(xml, yang_ctx);
        auto hello_request =
            netd::shared::request::HelloRequest::fromHelloToServer(
                *client_hello);
        session->processHelloRequest(*hello_request);
        logger.debug("rpcReceive: Hello request processed");
      } else {
        logger.error("rpcReceive: received unknown message");
      }
    } else {
      logger.debug("rpcReceive: No data received");
    }
  }

} // namespace netd::server::netconf
