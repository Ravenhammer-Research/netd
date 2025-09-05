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

#include <freebsd/include/interface/ppp.hpp>
#include <shared/include/logger.hpp>

#include <cstdlib>
#include <cstring>
#include <net/if.h>
#include <net/if_var.h>
#include <net/ppp_defs.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <unistd.h>

namespace netd::freebsd::interface {

  PppInterface::PppInterface()
      : netd::shared::interface::PppInterface(), name_(""), pppUnit_(-1),
        pppMode_("ppp"), pppProtocol_("ppp"), socket_(-1) {}

  PppInterface::PppInterface(const std::string &name)
      : netd::shared::interface::PppInterface(), name_(name), pppUnit_(-1),
        pppMode_("ppp"), pppProtocol_("ppp"), socket_(-1) {}

  PppInterface::~PppInterface() { closeSocket(); }

  bool PppInterface::createInterface() {
    auto &logger = shared::Logger::getInstance();

    if (!openSocket()) {
      logger.error("Failed to open socket for creating PPP interface");
      return false;
    }

    // Use SIOCIFCREATE to create PPP interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);

    if (ioctl(socket_, SIOCIFCREATE, &ifr) < 0) {
      logger.error("Failed to create PPP interface " + name_ + ": " +
                   std::strerror(errno));
      closeSocket();
      return false;
    }

    logger.info("Created PPP interface " + name_);
    return true;
  }

  bool PppInterface::destroyInterface() {
    auto &logger = shared::Logger::getInstance();

    if (!openSocket()) {
      logger.error("Failed to open socket for destroying PPP interface");
      return false;
    }

    // Use SIOCIFDESTROY to destroy PPP interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, name_.c_str(), IFNAMSIZ - 1);

    if (ioctl(socket_, SIOCIFDESTROY, &ifr) < 0) {
      logger.error("Failed to destroy PPP interface " + name_ + ": " +
                   std::strerror(errno));
      closeSocket();
      return false;
    }

    logger.info("Destroyed PPP interface " + name_);
    return true;
  }

  bool PppInterface::loadFromSystem() {
    auto &logger = shared::Logger::getInstance();

    if (!openSocket()) {
      return false;
    }

    if (!getPppInfo()) {
      closeSocket();
      return false;
    }

    closeSocket();
    logger.info("Loaded PPP interface information from system: " + name_);
    return true;
  }

  bool PppInterface::applyToSystem() {
    auto &logger = shared::Logger::getInstance();

    if (!openSocket()) {
      return false;
    }

    if (!setPppInfo()) {
      closeSocket();
      return false;
    }

    closeSocket();
    logger.info("Applied PPP interface configuration to system: " + name_);
    return true;
  }

  bool PppInterface::setPppUnit(int unit) {
    pppUnit_ = unit;
    return true;
  }

  int PppInterface::getPppUnit() const { return pppUnit_; }

  bool PppInterface::setPppMode(const std::string &mode) {
    pppMode_ = mode;
    return true;
  }

  std::string PppInterface::getPppMode() const { return pppMode_; }

  bool PppInterface::setPppProtocol(const std::string &protocol) {
    pppProtocol_ = protocol;
    return true;
  }

  std::string PppInterface::getPppProtocol() const { return pppProtocol_; }

  PppInterface::operator netd::shared::interface::PppInterface() const {
    // Cast to shared interface - we inherit from it so this is safe
    return static_cast<const netd::shared::interface::PppInterface &>(*this);
  }

  bool PppInterface::openSocket() {
    if (socket_ >= 0) {
      return true; // Already open
    }

    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) {
      return false;
    }

    return true;
  }

  void PppInterface::closeSocket() {
    if (socket_ >= 0) {
      close(socket_);
      socket_ = -1;
    }
  }

  bool PppInterface::getPppInfo() {
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

    // Get PPP-specific information
    // TODO: Use PPP-specific ioctls to get PPP details

    return true;
  }

  bool PppInterface::setPppInfo() const {
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

    // Set PPP-specific information
    // TODO: Use PPP-specific ioctls to set PPP details

    return true;
  }

} // namespace netd::freebsd::interface
