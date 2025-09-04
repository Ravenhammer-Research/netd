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

#include <iostream>
#include <shared/include/logger.hpp>
#include <client/include/netconf.hpp>
#include <string>

int main() {
    auto& logger = netd::Logger::getInstance();
    logger.info("NETD Client starting...");

    std::cout << "NETD Client - Network Configuration Tool" << std::endl;
    std::cout << "Connecting to server..." << std::endl;

    // Try to connect to the NETD server
    if (netd::connectToServer()) {
        std::cout << "Connected to NETD server successfully!" << std::endl;
        
        // Test basic NETCONF operations
        try {
            std::cout << "Testing get-config..." << std::endl;
            netd::Response response = netd::getConfig();
            std::cout << "Response: " << (response.isSuccess() ? "SUCCESS" : "ERROR") << " - " << response.getData() << std::endl;
            
            std::cout << "Testing commit..." << std::endl;
            response = netd::commit();
            std::cout << "Response: " << (response.isSuccess() ? "SUCCESS" : "ERROR") << " - " << response.getData() << std::endl;
            
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        
        // Disconnect from server
        netd::disconnectFromServer();
        std::cout << "Disconnected from server" << std::endl;
    } else {
        std::cout << "Failed to connect to NETD server" << std::endl;
        std::cout << "Make sure the server is running with: ./netd" << std::endl;
    }

    logger.info("NETD Client finished");
    return 0;
}
