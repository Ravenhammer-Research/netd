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

#include <algorithm>
#include <shared/include/exception.hpp>
#include <shared/include/interface/base/master.hpp>

namespace netd::shared::interface::base {

  bool Master::addSlave(const std::string &slaveName, uint32_t priority) {
    // Check if slave already exists
    if (hasSlave(slaveName)) {
      return false;
    }
    slaves_.emplace_back(slaveName, priority);
    return true;
  }

  bool Master::removeSlave(const std::string &slaveName) {
    auto it = std::find_if(
        slaves_.begin(), slaves_.end(),
        [&slaveName](const SlaveInfo &info) { return info.name == slaveName; });
    if (it != slaves_.end()) {
      slaves_.erase(it);
      return true;
    }
    return false;
  }

  std::vector<std::string> Master::getSlaves() const {
    std::vector<std::string> slaveNames;
    for (const auto &slave : slaves_) {
      slaveNames.push_back(slave.name);
    }
    return slaveNames;
  }

  bool Master::hasSlave(const std::string &slaveName) const {
    return std::any_of(
        slaves_.begin(), slaves_.end(),
        [&slaveName](const SlaveInfo &info) { return info.name == slaveName; });
  }

  bool Master::setSlavePriority(const std::string &slaveName,
                                uint32_t priority) {
    auto it = std::find_if(
        slaves_.begin(), slaves_.end(),
        [&slaveName](const SlaveInfo &info) { return info.name == slaveName; });
    if (it != slaves_.end()) {
      it->priority = priority;
      return true;
    }
    return false;
  }

  uint32_t Master::getSlavePriority(const std::string &slaveName) const {
    auto it = std::find_if(
        slaves_.begin(), slaves_.end(),
        [&slaveName](const SlaveInfo &info) { return info.name == slaveName; });
    if (it != slaves_.end()) {
      return it->priority;
    }
    return 0;
  }

  bool Master::setSlaveEnabled(const std::string &slaveName, bool enabled) {
    auto it = std::find_if(
        slaves_.begin(), slaves_.end(),
        [&slaveName](const SlaveInfo &info) { return info.name == slaveName; });
    if (it != slaves_.end()) {
      it->enabled = enabled;
      return true;
    }
    return false;
  }

  bool Master::isSlaveEnabled(const std::string &slaveName) const {
    auto it = std::find_if(
        slaves_.begin(), slaves_.end(),
        [&slaveName](const SlaveInfo &info) { return info.name == slaveName; });
    if (it != slaves_.end()) {
      return it->enabled;
    }
    return false;
  }

  bool Master::isMaster() const { return isMaster_; }

  uint32_t Master::getSlaveCount() const {
    return static_cast<uint32_t>(slaves_.size());
  }

  bool Master::validateSlaveConfiguration() const {
    // Basic validation - can be expanded
    return true;
  }

} // namespace netd::shared::interface::base
