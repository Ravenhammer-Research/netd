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

#include <freebsd/include/interface/epair.hpp>
#include <shared/include/logger.hpp>

#include <cstdlib>
#include <cstring>
#include <net/if.h>
#include <net/if_clone.h>
#include <net/if_var.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace netd::freebsd::interface {

  EpairInterface::EpairInterface()
      : netd::shared::interface::EpairInterface(), name_(""), peerEnd_(""),
        epairUnit_(-1), socket_(-1) {}

  EpairInterface::EpairInterface(const std::string &name)
      : netd::shared::interface::EpairInterface(), name_(name), peerEnd_(""),
        epairUnit_(-1), socket_(-1) {}

  EpairInterface::~EpairInterface() { closeSocket(); }

  bool EpairInterface::createInterface() {
    auto &logger = shared::Logger::getInstance();

    if (!openSocket()) {
      logger.error("Failed to open socket for creating epair interface");
      return false;
    }

    // Use SIOCIFCREATE to create epair interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);

    if (ioctl(socket_, SIOCIFCREATE, &ifr) < 0) {
      logger.error("Failed to create epair interface " + name_ + ": " +
                   std::strerror(errno));
      closeSocket();
      return false;
    }

    logger.info("Created epair interface " + name_);
    return true;
  }

  bool EpairInterface::destroyInterface() {
    auto &logger = shared::Logger::getInstance();

    if (!openSocket()) {
      logger.error("Failed to open socket for destroying epair interface");
      return false;
    }

    // Use SIOCIFDESTROY to destroy epair interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);

    if (ioctl(socket_, SIOCIFDESTROY, &ifr) < 0) {
      logger.error("Failed to destroy epair interface " + name_ + ": " +
                   std::strerror(errno));
      closeSocket();
      return false;
    }

    logger.info("Destroyed epair interface " + name_);
    return true;
  }

  bool EpairInterface::loadFromSystem() {
    auto &logger = shared::Logger::getInstance();

    if (!openSocket()) {
      return false;
    }

    if (!getEpairInfo()) {
      closeSocket();
      return false;
    }

    closeSocket();
    logger.info("Loaded epair interface information from system: " + name_);
    return true;
  }

  bool EpairInterface::applyToSystem() {
    auto &logger = shared::Logger::getInstance();

    if (!openSocket()) {
      return false;
    }

    if (!setEpairInfo()) {
      closeSocket();
      return false;
    }

    closeSocket();
    logger.info("Applied epair interface configuration to system: " + name_);
    return true;
  }

  bool EpairInterface::setPeerEnd(const std::string &peerEnd) {
    peerEnd_ = peerEnd;
    return true;
  }

  std::string EpairInterface::getPeerEnd() const { return peerEnd_; }

  bool EpairInterface::setEpairUnit(int unit) {
    epairUnit_ = unit;
    return true;
  }

  int EpairInterface::getEpairUnit() const { return epairUnit_; }

  bool EpairInterface::openSocket() {
    if (socket_ >= 0) {
      return true; // Already open
    }

    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) {
      return false;
    }

    return true;
  }

  void EpairInterface::closeSocket() {
    if (socket_ >= 0) {
      close(socket_);
      socket_ = -1;
    }
  }

  bool EpairInterface::getEpairInfo() {
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

    // Get epair-specific information
    // TODO: Use epair-specific ioctls to get epair details

    return true;
  }

  bool EpairInterface::setEpairInfo() const {
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

    // Set epair-specific information
    // TODO: Use epair-specific ioctls to set epair details

    return true;
  }

} // namespace netd::freebsd::interface
