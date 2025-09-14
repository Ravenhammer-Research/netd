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

#ifndef NETD_CLIENT_PROCESSOR_COMPLETION_HPP
#define NETD_CLIENT_PROCESSOR_COMPLETION_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include <shared/include/expect/base.hpp>

// Forward declaration
namespace netd::client::netconf {
    class NetconfClient;
}

namespace netd::client::processor {

class CommandCompletion {
public:
    /**
     * @brief Get all available command keywords for completion
     * @return Vector of command keywords
     */
    static const std::vector<std::string>& getCommandKeywords();
    
    /**
     * @brief Get all available interface keywords for completion
     * @return Vector of interface keywords
     */
    static const std::vector<std::string>& getInterfaceKeywords();
    
    /**
     * @brief Get all available routing keywords for completion
     * @return Vector of routing keywords
     */
    static const std::vector<std::string>& getRoutingKeywords();
    
    /**
     * @brief Get all available display keywords for completion
     * @return Vector of display keywords
     */
    static const std::vector<std::string>& getDisplayKeywords();
    
    /**
     * @brief Get all available protocol keywords for completion
     * @return Vector of protocol keywords
     */
    static const std::vector<std::string>& getProtocolKeywords();
    
    /**
     * @brief Get all available keywords for completion
     * @return Vector of all keywords
     */
    static const std::vector<std::string>& getAllKeywords();
    
    /**
     * @brief Find completions for a partial command
     * @param partial The partial command string
     * @return Vector of matching completions
     */
    static std::vector<std::string> findCompletions(const std::string& partial);
    
    /**
     * @brief Find contextual completions based on command context
     * @param command_line The full command line being typed
     * @return Vector of contextual completions
     */
    static std::vector<std::string> findContextualCompletions(const std::string& command_line);
    
    /**
     * @brief Get the longest common prefix of multiple strings
     * @param strings Vector of strings
     * @return Common prefix string
     */
    static std::string getCommonPrefix(const std::vector<std::string>& strings);
    
    /**
     * @brief Check if a string is a valid command keyword
     * @param keyword The keyword to check
     * @return true if valid, false otherwise
     */
    static bool isValidKeyword(const std::string& keyword);
    
    /**
     * @brief Debug method to print completion candidates
     * @param command_line The command line being completed
     */
    static void debugCompletions(const std::string& command_line);
    
    /**
     * @brief Set the NETCONF client for fetching real interface data
     * @param client Reference to the NETCONF client
     */
    static void setNetconfClient(netd::client::netconf::NetconfClient* client);
    
    /**
     * @brief Get actual interface names from the system via NETCONF
     * @return Vector of interface names
     */
    static std::vector<std::string> getNetconfInterfaces();

private:
    static void initializeKeywords();
    static bool initialized_;
    static std::vector<std::string> command_keywords_;
    static std::vector<std::string> interface_keywords_;
    static std::vector<std::string> routing_keywords_;
    static std::vector<std::string> display_keywords_;
    static std::vector<std::string> protocol_keywords_;
    static std::vector<std::string> all_keywords_;
    static std::unordered_set<std::string> keyword_set_;
    static netd::client::netconf::NetconfClient* netconf_client_;
};

} // namespace netd::client::processor

#endif // NETD_CLIENT_PROCESSOR_COMPLETION_HPP
