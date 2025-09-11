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

#include <shared/include/lldp/client.hpp>
#include <shared/include/logger.hpp>

namespace netd::shared::lldp {

Client::Client()
    : initialized_(false)
{
}

Client::~Client() {
    cleanup();
}

bool Client::initialize() {
    auto& logger = Logger::getInstance();
    
    try {
        // Create connection
        connection_ = std::make_unique<Connection>();
        if (!connection_->initialize()) {
            logger.error("Failed to initialize LLDP connection");
            return false;
        }

        // Create other components
        discovery_ = std::make_unique<Discovery>(connection_->getConnection());
        service_ = std::make_unique<Service>(connection_->getConnection());
        interface_ = std::make_unique<Interface>(connection_->getConnection());

        initialized_ = true;
        logger.info("LLDP client initialized successfully");
        return true;
    } catch (const std::exception& e) {
        logger.error("Exception during LLDP client initialization: " + std::string(e.what()));
        return false;
    }
}

void Client::cleanup() {
    if (discovery_) {
        discovery_->stop();
    }
    
    if (service_) {
        service_->unregisterService();
    }
    
    if (connection_) {
        connection_->cleanup();
    }
    
    initialized_ = false;
}

bool Client::registerService(const std::string& service_name,
                            ServiceType service_type,
                            const std::string& hostname,
                            uint16_t port,
                            const std::string& interface_name,
                            const std::map<std::string, std::string>& additional_info) {
    if (!initialized_ || !service_) {
        auto& logger = Logger::getInstance();
        logger.error("LLDP client not initialized");
        return false;
    }

    return service_->registerService(service_name, service_type, hostname, port, interface_name, additional_info);
}

bool Client::unregisterService() {
    if (!service_) {
        return true;
    }
    return service_->unregisterService();
}

bool Client::startDiscovery() {
    if (!initialized_ || !discovery_) {
        auto& logger = Logger::getInstance();
        logger.error("LLDP client not initialized");
        return false;
    }

    return discovery_->start();
}

void Client::stopDiscovery() {
    if (discovery_) {
        discovery_->stop();
    }
}

bool Client::discoverOnce() {
    if (!discovery_) {
        return false;
    }
    return discovery_->discoverOnce();
}

std::vector<ServiceInfo> Client::getDiscoveredServices() const {
    if (!discovery_) {
        return {};
    }
    return discovery_->getDiscoveredServices();
}

std::vector<ServiceInfo> Client::getDiscoveredServices(ServiceType service_type) const {
    if (!discovery_) {
        return {};
    }
    return discovery_->getDiscoveredServices(service_type);
}

std::vector<ServiceInfo> Client::getDiscoveredServices(const std::string& service_name) const {
    if (!discovery_) {
        return {};
    }
    return discovery_->getDiscoveredServices(service_name);
}

std::vector<std::string> Client::getLLDPInterfaces() const {
    if (!interface_) {
        return {};
    }
    return interface_->getLLDPInterfaces();
}

std::map<std::string, std::string> Client::getLinkLocalAddresses() const {
    if (!interface_) {
        return {};
    }
    
    auto address_map = interface_->getLinkLocalAddresses();
    std::map<std::string, std::string> string_map;
    
    for (const auto& pair : address_map) {
        if (pair.second) {
            string_map[pair.first] = pair.second->getString();
        }
    }
    
    return string_map;
}

bool Client::isRegistered() const {
    return service_ && service_->isRegistered();
}

bool Client::isDiscoveryRunning() const {
    return discovery_ && discovery_->isRunning();
}

} // namespace netd::shared::lldp