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

#ifndef NETD_SERVER_STORE_HPP
#define NETD_SERVER_STORE_HPP

#include <string>
#include <memory>
#include <vector>

namespace netd {

// Forward declarations
class Interface;
class VRF;
class Route;

// Configuration store types
enum class ConfigType {
    CANDIDATE,
    RUNNING,
    STARTUP
};

// Configuration management functions
bool loadStartupConfiguration();
bool saveStartupConfiguration();
bool commitCandidateConfiguration();

// Configuration retrieval
std::string getConfiguration(ConfigType type);

// Configuration modification
bool setCandidateConfiguration(const std::string& config);
bool resetCandidateConfiguration();
bool discardCandidateConfiguration();

// Configuration enumeration
std::vector<std::shared_ptr<Interface>> enumerateInterfaces();
std::vector<std::shared_ptr<VRF>> enumerateVRFs();
std::vector<std::shared_ptr<Route>> enumerateRoutes();

} // namespace netd

#endif // NETD_SERVER_STORE_HPP
