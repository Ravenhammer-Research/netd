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

#pragma once

#include <map>
#include <memory>
#include <shared/include/lldp/chassis.hpp>
#include <shared/include/lldp/config.hpp>
#include <shared/include/lldp/connection.hpp>
#include <shared/include/lldp/custom.hpp>
#include <shared/include/lldp/interface.hpp>
#include <shared/include/lldp/neighbor.hpp>
#include <shared/include/lldp/port.hpp>
#include <string>
#include <vector>

namespace netd::shared::lldp {

  class Client {
  public:
    Client();
    ~Client();

    void initialize();
    void cleanup();

    std::vector<std::string> getLLDPInterfaces() const;
    std::map<std::string, std::string> getLinkLocalAddresses() const;

    // Get LLDP objects
    std::vector<std::unique_ptr<Port>> getPorts() const;
    std::unique_ptr<Port> getLocalPort() const;
    std::vector<std::unique_ptr<Port>> getAllLocalPorts() const;
    std::unique_ptr<Chassis> getLocalChassis() const;
    std::unique_ptr<Config> getConfiguration() const;
    std::unique_ptr<CustomTLVManager> getCustomTLVManager() const;

    bool isInitialized() const { return initialized_; }

  private:
    std::unique_ptr<Connection> connection_;
    std::unique_ptr<Interface> interface_;
    bool initialized_;
  };

} // namespace netd::shared::lldp