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

#ifndef NETD_INTERFACE_BASE_MASTER_HPP
#define NETD_INTERFACE_BASE_MASTER_HPP

#include <vector>
#include <string>
#include <memory>
#include <cstdint>

namespace netd::shared::interface::base {

    class Master {
    public:
        virtual ~Master() = default;

        // Slave interface management
        virtual bool addSlave(const std::string& slaveName, uint32_t priority = 0);
        virtual bool removeSlave(const std::string& slaveName);
        virtual std::vector<std::string> getSlaves() const;
        virtual bool hasSlave(const std::string& slaveName) const;

        // Slave configuration
        virtual bool setSlavePriority(const std::string& slaveName, uint32_t priority);
        virtual uint32_t getSlavePriority(const std::string& slaveName) const;
        virtual bool setSlaveEnabled(const std::string& slaveName, bool enabled);
        virtual bool isSlaveEnabled(const std::string& slaveName) const;

        // Master interface state
        virtual bool isMaster() const;
        virtual uint32_t getSlaveCount() const;
        virtual bool validateSlaveConfiguration() const;

    protected:
        struct SlaveInfo {
            std::string name;
            uint32_t priority{0};
            bool enabled{true};
            
            SlaveInfo(const std::string& n, uint32_t p = 0, bool e = true)
                : name(n), priority(p), enabled(e) {}
        };

        std::vector<SlaveInfo> slaves_;
        bool isMaster_{true};
    };

} // namespace netd::shared::interface::base

#endif // NETD_INTERFACE_BASE_MASTER_HPP
