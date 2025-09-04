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

#ifndef NETD_TUNNEL_HPP
#define NETD_TUNNEL_HPP

#include <shared/include/interface/base/tunnel.hpp>

namespace netd {

/**
 * Wrapper class for interface::base::Tunnel
 * 
 * This wrapper exists to provide a cleaner namespace structure:
 * - interface::base::Tunnel is the actual implementation with all the methods
 * - netd::Tunnel is a simple wrapper that inherits from interface::base::Tunnel
 * 
 * This allows interface classes to inherit from "Tunnel" instead of the 
 * longer "interface::base::Tunnel" name, making the code more readable.
 * 
 * The wrapper adds no functionality - it just provides a shorter, cleaner name.
 */
class Tunnel : public interface::base::Tunnel {
public:
    Tunnel() = default;
    virtual ~Tunnel() = default;

    // Tunnel class now inherits all functionality from interface::base::Tunnel
    // No additional implementation needed unless Tunnel-specific behavior is required
};

} // namespace netd

#endif // NETD_TUNNEL_HPP
