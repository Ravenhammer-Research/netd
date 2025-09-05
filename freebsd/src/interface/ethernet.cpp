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

#include <freebsd/include/interface/ethernet.hpp>
#include <shared/include/logger.hpp>

#include <cstdlib>
#include <cstring>
#include <net/if.h>
#include <net/if_var.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace netd::freebsd::interface {

  EthernetInterface::EthernetInterface()
      : netd::shared::interface::EthernetInterface(), name_(""),
        duplex_("auto"), speed_(0), autoNegotiation_(true), flowControl_(false),
        socket_(-1) {}

  EthernetInterface::EthernetInterface(const std::string &name)
      : netd::shared::interface::EthernetInterface(), name_(name),
        duplex_("auto"), speed_(0), autoNegotiation_(true), flowControl_(false),
        socket_(-1) {}

  EthernetInterface::~EthernetInterface() { closeSocket(); }

  bool EthernetInterface::createInterface() {
    auto &logger = shared::Logger::getInstance();

    if (!openSocket()) {
      logger.error("Failed to open socket for creating ethernet interface");
      return false;
    }

    // Use SIOCIFCREATE to create ethernet interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);

    if (ioctl(socket_, SIOCIFCREATE, &ifr) < 0) {
      logger.error("Failed to create ethernet interface " + name_ + ": " +
                   std::strerror(errno));
      closeSocket();
      return false;
    }

    logger.info("Created ethernet interface " + name_);
    return true;
  }

  bool EthernetInterface::destroyInterface() {
    auto &logger = shared::Logger::getInstance();

    if (!openSocket()) {
      logger.error("Failed to open socket for destroying ethernet interface");
      return false;
    }

    // Use SIOCIFDESTROY to destroy ethernet interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);

    if (ioctl(socket_, SIOCIFDESTROY, &ifr) < 0) {
      logger.error("Failed to destroy ethernet interface " + name_ + ": " +
                   std::strerror(errno));
      closeSocket();
      return false;
    }

    logger.info("Destroyed ethernet interface " + name_);
    return true;
  }

  bool EthernetInterface::loadFromSystem() {
    auto &logger = shared::Logger::getInstance();

    if (!openSocket()) {
      return false;
    }

    if (!getInterfaceInfo()) {
      closeSocket();
      return false;
    }

    closeSocket();
    logger.info("Loaded ethernet interface information from system: " + name_);
    return true;
  }

  bool EthernetInterface::applyToSystem() {
    auto &logger = shared::Logger::getInstance();

    if (!openSocket()) {
      return false;
    }

    if (!setInterfaceInfo()) {
      closeSocket();
      return false;
    }

    closeSocket();
    logger.info("Applied ethernet interface configuration to system: " + name_);
    return true;
  }

  bool EthernetInterface::setDuplex(const std::string &duplex) {
    duplex_ = duplex;
    return true;
  }

  std::string EthernetInterface::getDuplex() const { return duplex_; }

  bool EthernetInterface::setSpeed(uint32_t speed) {
    speed_ = speed;
    return true;
  }

  uint32_t EthernetInterface::getSpeed() const { return speed_; }

  bool EthernetInterface::setAutoNegotiation(bool enabled) {
    autoNegotiation_ = enabled;
    return true;
  }

  bool EthernetInterface::isAutoNegotiationEnabled() const {
    return autoNegotiation_;
  }

  bool EthernetInterface::setFlowControl(bool enabled) {
    flowControl_ = enabled;
    return true;
  }

  bool EthernetInterface::isFlowControlEnabled() const { return flowControl_; }

  bool EthernetInterface::openSocket() {
    if (socket_ >= 0) {
      return true; // Already open
    }

    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) {
      return false;
    }

    return true;
  }

  void EthernetInterface::closeSocket() {
    if (socket_ >= 0) {
      close(socket_);
      socket_ = -1;
    }
  }

  bool EthernetInterface::getInterfaceInfo() {
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);

    // Get interface flags
    if (ioctl(socket_, SIOCGIFFLAGS, &ifr) < 0) {
      return false;
    }

    // Get interface MTU
    if (ioctl(socket_, SIOCGIFMTU, &ifr) < 0) {
      return false;
    }

    // Get interface address
    if (ioctl(socket_, SIOCGIFADDR, &ifr) < 0) {
      return false;
    }

    return true;
  }

  bool EthernetInterface::setInterfaceInfo() const {
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);

    // Set interface flags
    if (ioctl(socket_, SIOCSIFFLAGS, &ifr) < 0) {
      return false;
    }

    // Set interface MTU
    if (ioctl(socket_, SIOCSIFMTU, &ifr) < 0) {
      return false;
    }

    return true;
  }

} // namespace netd::freebsd::interface
