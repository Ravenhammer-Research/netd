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

#ifndef NETD_FREEBSD_ROUTE_HPP
#define NETD_FREEBSD_ROUTE_HPP

#include <memory>
#include <shared/include/backend.hpp>
#include <shared/include/base/serialization.hpp>
#include <shared/include/route.hpp>
#include <string>

namespace netd::freebsd {

  class Route : public netd::shared::Route,
                public netd::shared::BaseNativeBackend {
  public:
    Route();
    explicit Route(const std::string &destination,
                   const std::string &gateway = "",
                   const std::string &interface = "");
    virtual ~Route();

    // Route management
    bool add();
    bool remove();
    bool modify();

    // FreeBSD-specific operations
    bool loadFromSystem();
    bool applyToSystem() const;

    // Route properties
    bool setMetric(uint32_t metric);
    uint32_t getMetric() const;
    bool setFlags(uint32_t flags);
    uint32_t getFlags() const;
    bool setFibTable(uint32_t fibTable);
    uint32_t getFibTable() const;

  private:
    std::string destination_;
    std::string gateway_;
    std::string interface_;
    uint32_t metric_;
    uint32_t flags_;
    uint32_t fibTable_;

    // Helper methods
    bool parseRouteString(const std::string &routeStr);
    std::string formatRouteString() const;
  };

} // namespace netd::freebsd

#endif // NETD_FREEBSD_ROUTE_HPP
