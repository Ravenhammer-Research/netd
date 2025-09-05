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

#include <cstdlib>
#include <cstring>
#include <freebsd/include/interface/bridge.hpp>
#include <net/if.h>
#include <net/if_bridgevar.h>
#include <net/if_var.h>
#include <shared/include/logger.hpp>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace netd::freebsd::interface {

  BridgeInterface::BridgeInterface()
      : netd::shared::interface::BridgeInterface(), stpEnabled_(false),
        maxAge_(20), helloTime_(2), forwardDelay_(15), bridgeSocket_(-1) {}

  BridgeInterface::BridgeInterface(const std::string &name)
      : netd::shared::interface::BridgeInterface(), stpEnabled_(false),
        maxAge_(20), helloTime_(2), forwardDelay_(15), bridgeSocket_(-1) {
    // Store the name
    name_ = name;
  }

  BridgeInterface::~BridgeInterface() { closeBridgeSocket(); }

  bool BridgeInterface::addMember(const std::string &memberName) {
    auto &logger = shared::Logger::getInstance();

    if (!openBridgeSocket()) {
      logger.error("Failed to open bridge socket for adding member: " +
                   memberName);
      return false;
    }

    struct ifdrv ifd;
    struct ifbreq req;

    std::memset(&ifd, 0, sizeof(ifd));
    std::memset(&req, 0, sizeof(req));

    std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
    ifd.ifd_cmd = BRDGADD;
    ifd.ifd_len = sizeof(req);
    ifd.ifd_data = &req;

    std::strncpy(req.ifbr_ifsname, memberName.c_str(), IFNAMSIZ - 1);

    if (ioctl(bridgeSocket_, SIOCSDRVSPEC, &ifd) < 0) {
      logger.error("Failed to add bridge member " + memberName + ": " +
                   std::strerror(errno));
      return false;
    }

    // Add to our local list
    members_.push_back(memberName);
    logger.info("Added member " + memberName + " to bridge " + getName());
    return true;
  }

  bool BridgeInterface::removeMember(const std::string &memberName) {
    auto &logger = shared::Logger::getInstance();

    if (!openBridgeSocket()) {
      logger.error("Failed to open bridge socket for removing member: " +
                   memberName);
      return false;
    }

    struct ifdrv ifd;
    struct ifbreq req;

    std::memset(&ifd, 0, sizeof(ifd));
    std::memset(&req, 0, sizeof(req));

    std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
    ifd.ifd_cmd = BRDGDEL;
    ifd.ifd_len = sizeof(req);
    ifd.ifd_data = &req;

    std::strncpy(req.ifbr_ifsname, memberName.c_str(), IFNAMSIZ - 1);

    if (ioctl(bridgeSocket_, SIOCSDRVSPEC, &ifd) < 0) {
      logger.error("Failed to remove bridge member " + memberName + ": " +
                   std::strerror(errno));
      return false;
    }

    // Remove from our local list
    auto it = std::find(members_.begin(), members_.end(), memberName);
    if (it != members_.end()) {
      members_.erase(it);
    }

    logger.info("Removed member " + memberName + " from bridge " + getName());
    return true;
  }

  std::vector<std::string> BridgeInterface::getMembers() const {
    return members_;
  }

  bool BridgeInterface::setStpEnabled(bool enabled) {
    stpEnabled_ = enabled;
    return true;
  }

  bool BridgeInterface::isStpEnabled() const { return stpEnabled_; }

  bool BridgeInterface::setMaxAge(uint16_t maxAge) {
    if (maxAge < 6 || maxAge > 40) {
      return false;
    }
    maxAge_ = maxAge;
    return true;
  }

  uint16_t BridgeInterface::getMaxAge() const { return maxAge_; }

  bool BridgeInterface::setHelloTime(uint16_t helloTime) {
    if (helloTime < 1 || helloTime > 10) {
      return false;
    }
    helloTime_ = helloTime;
    return true;
  }

  uint16_t BridgeInterface::getHelloTime() const { return helloTime_; }

  bool BridgeInterface::setForwardDelay(uint16_t forwardDelay) {
    if (forwardDelay < 4 || forwardDelay > 30) {
      return false;
    }
    forwardDelay_ = forwardDelay;
    return true;
  }

  uint16_t BridgeInterface::getForwardDelay() const { return forwardDelay_; }

  bool BridgeInterface::createInterface() {
    auto &logger = shared::Logger::getInstance();

    if (!openBridgeSocket()) {
      logger.error("Failed to open bridge socket for creating interface");
      return false;
    }

    // Use SIOCIFCREATE to create bridge interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(bridgeSocket_, SIOCIFCREATE, &ifr) < 0) {
      logger.error("Failed to create bridge interface " + getName() + ": " +
                   std::strerror(errno));
      closeBridgeSocket();
      return false;
    }

    logger.info("Created bridge interface: " + getName());
    return true;
  }

  bool BridgeInterface::destroyInterface() {
    auto &logger = shared::Logger::getInstance();

    if (!openBridgeSocket()) {
      logger.error("Failed to open bridge socket for destroying interface");
      return false;
    }

    // Use SIOCIFDESTROY to destroy bridge interface
    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, getName().c_str(), IFNAMSIZ - 1);

    if (ioctl(bridgeSocket_, SIOCIFDESTROY, &ifr) < 0) {
      logger.error("Failed to destroy bridge interface " + getName() + ": " +
                   std::strerror(errno));
      closeBridgeSocket();
      return false;
    }

    logger.info("Destroyed bridge interface: " + getName());
    return true;
  }

  bool BridgeInterface::loadFromSystem() {
    auto &logger = shared::Logger::getInstance();

    if (!openBridgeSocket()) {
      logger.error("Failed to open bridge socket for loading interface info");
      return false;
    }

    // Get bridge information
    if (!getBridgeInfo()) {
      logger.error("Failed to get bridge information from system");
      return false;
    }

    logger.info("Loaded bridge interface information from system: " +
                getName());
    return true;
  }

  bool BridgeInterface::applyToSystem() const {
    auto &logger = shared::Logger::getInstance();

    if (!openBridgeSocket()) {
      logger.error("Failed to open bridge socket for applying configuration");
      return false;
    }

    // Apply bridge configuration
    if (!setBridgeInfo()) {
      logger.error("Failed to apply bridge configuration to system");
      return false;
    }

    logger.info("Applied bridge configuration to system: " + getName());
    return true;
  }

  // YANG serialization is inherited from shared::BridgeInterface
  // No need to override - the shared implementation handles it

  bool BridgeInterface::openBridgeSocket() {
    if (bridgeSocket_ >= 0) {
      return true; // Already open
    }

    bridgeSocket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (bridgeSocket_ < 0) {
      return false;
    }

    return true;
  }

  bool BridgeInterface::openBridgeSocket() const {
    // For const methods, we need to create a temporary socket
    int tempSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (tempSocket < 0) {
      return false;
    }

    // Close it immediately since we can't store it
    close(tempSocket);
    return true;
  }

  void BridgeInterface::closeBridgeSocket() {
    if (bridgeSocket_ >= 0) {
      close(bridgeSocket_);
      bridgeSocket_ = -1;
    }
  }

  bool BridgeInterface::getBridgeInfo() {
    if (bridgeSocket_ < 0) {
      return false;
    }

    // Get bridge members
    struct ifdrv ifd;
    struct ifbreq req;

    std::memset(&ifd, 0, sizeof(ifd));
    std::memset(&req, 0, sizeof(req));

    std::strncpy(ifd.ifd_name, getName().c_str(), IFNAMSIZ - 1);
    ifd.ifd_cmd = BRDGGIFFLGS;
    ifd.ifd_len = sizeof(req);
    ifd.ifd_data = &req;

    // Get member count first
    if (ioctl(bridgeSocket_, SIOCSDRVSPEC, &ifd) < 0) {
      return false;
    }

    // TODO: Implement full bridge information retrieval
    // This would involve multiple ioctl calls to get STP parameters,
    // member interfaces, and other bridge-specific information

    return true;
  }

  bool BridgeInterface::setBridgeInfo() const {
    if (bridgeSocket_ < 0) {
      return false;
    }

    // TODO: Implement full bridge configuration
    // This would involve multiple ioctl calls to set STP parameters,
    // member interfaces, and other bridge-specific configuration

    return true;
  }

  BridgeInterface::operator netd::shared::interface::BridgeInterface() const {
    // Cast to shared interface - we inherit from it so this is safe
    return static_cast<const netd::shared::interface::BridgeInterface &>(*this);
  }

} // namespace netd::freebsd::interface
