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

#ifndef NETD_FREEBSD_INTERFACE_EPAIR_HPP
#define NETD_FREEBSD_INTERFACE_EPAIR_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <shared/include/base/serialization.hpp>
#include <shared/include/interface/epair.hpp>
#include <shared/include/backend.hpp>

namespace netd::freebsd::interface {

  class EpairInterface : public netd::shared::interface::EpairInterface, public netd::shared::BaseNativeBackend {
  public:
    EpairInterface();
    explicit EpairInterface(const std::string &name);
    virtual ~EpairInterface();

    // Interface name
    std::string getName() const { return name_; }

    // FreeBSD-specific operations
    bool createInterface();
    bool destroyInterface();
    bool loadFromSystem();
    bool applyToSystem();

    // Epair-specific configuration
    bool setPeerEnd(const std::string &peerEnd);
    std::string getPeerEnd() const;
    bool setEpairUnit(int unit);
    int getEpairUnit() const;

    // Statistics and information
    std::string getType() const { return "epair"; }

  private:
    // Interface name
    std::string name_;

    // Epair-specific members
    std::string peerEnd_;
    int epairUnit_;

    // FreeBSD system interface
    int socket_;

    // Helper methods
    bool openSocket();
    void closeSocket();
    bool getEpairInfo();
    bool setEpairInfo() const;
  };

} // namespace netd::freebsd::interface

#endif // NETD_FREEBSD_INTERFACE_EPAIR_HPP
