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
#include <client/include/terminal.hpp>
#include <client/include/table.hpp>
#include <string>
#include <sstream>
#include <vector>

namespace netd {

class CommandProcessor {
public:
    CommandProcessor(Terminal& terminal) : terminal_(terminal) {
        setupCompletions();
    }

    bool processCommand(const std::string& command) {
        std::istringstream iss(command);
        std::vector<std::string> tokens;
        std::string token;
        
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        if (tokens.empty()) {
            return true;
        }

        const std::string& cmd = tokens[0];
        
        if (cmd == "show") {
            return handleShowCommand(tokens);
        } else if (cmd == "set") {
            return handleSetCommand(tokens);
        } else if (cmd == "delete") {
            return handleDeleteCommand(tokens);
        } else if (cmd == "commit") {
            return handleCommitCommand(tokens);
        } else {
            terminal_.writeLine("Unknown command: " + cmd);
            return false;
        }
    }

private:
    Terminal& terminal_;

    void setupCompletions() {
        std::vector<std::string> completions = {
            "show", "set", "delete", "commit", "help", "quit", "exit",
            "vrf", "interface", "protocol", "static", "host", "network",
            "gateway", "iface", "address", "mtu", "group", "member",
            "laggport", "vlandev", "vlanproto", "vxlanid", "tunnel",
            "local", "remote", "type", "name", "id", "inet", "inet6"
        };
        terminal_.setCompletions(completions);
    }

    bool handleShowCommand(const std::vector<std::string>& tokens) {
        if (tokens.size() < 2) {
            terminal_.writeLine("Usage: show <vrf|interface> [options]");
            return false;
        }

        const std::string& subcmd = tokens[1];
        
        if (subcmd == "vrf") {
            return handleShowVrf(tokens);
        } else if (subcmd == "interface") {
            return handleShowInterface(tokens);
        } else {
            terminal_.writeLine("Unknown show command: " + subcmd);
            return false;
        }
    }

    bool handleShowVrf(const std::vector<std::string>& tokens) {
        // TODO: Implement VRF display
        terminal_.writeLine("VRF information:");
        terminal_.writeLine("  FIB 0: default");
        return true;
    }

    bool handleShowInterface(const std::vector<std::string>& tokens) {
        try {
            // Send get-config request with interface filter
            // This should generate: <get-config><source><running/></source><filter type="subtree"><interfaces/></filter></get-config>
            netd::Response response = netd::getConfig("running");
            if (!response.isSuccess()) {
                terminal_.writeLine("Failed to get interface data: " + response.getData());
                return false;
            }

            // TODO: Parse response data to shared Interface types
            // For now, create a sample table
            netd::Table table;
            table.addColumn("Interface");
            table.addColumn("Type");
            table.addColumn("Status");
            table.addColumn("MTU");
            
            table.addRow({"lo0", "loopback", "UP", "16384"});
            table.addRow({"em0", "ethernet", "UP", "1500"});
            table.addRow({"bridge0", "bridge", "DOWN", "1500"});
            
            terminal_.writeLine(table.format());
            return true;
        } catch (const std::exception& e) {
            terminal_.writeLine("Error getting interface data: " + std::string(e.what()));
            return false;
        }
    }

    bool handleSetCommand(const std::vector<std::string>& tokens) {
        if (tokens.size() < 2) {
            terminal_.writeLine("Usage: set <vrf|interface> [options]");
            return false;
        }

        const std::string& subcmd = tokens[1];
        
        if (subcmd == "vrf") {
            return handleSetVrf(tokens);
        } else if (subcmd == "interface") {
            return handleSetInterface(tokens);
        } else {
            terminal_.writeLine("Unknown set command: " + subcmd);
            return false;
        }
    }

    bool handleSetVrf(const std::vector<std::string>& tokens) {
        // TODO: Implement VRF configuration
        terminal_.writeLine("VRF configuration not yet implemented");
        return true;
    }

    bool handleSetInterface(const std::vector<std::string>& tokens) {
        // TODO: Implement interface configuration
        terminal_.writeLine("Interface configuration not yet implemented");
        return true;
    }

    bool handleDeleteCommand(const std::vector<std::string>& tokens) {
        // TODO: Implement delete operations
        terminal_.writeLine("Delete operations not yet implemented");
        return true;
    }

    bool handleCommitCommand(const std::vector<std::string>& tokens) {
        try {
            netd::Response response = netd::commit();
            if (response.isSuccess()) {
                terminal_.writeLine("Configuration committed successfully");
            } else {
                terminal_.writeLine("Commit failed: " + response.getData());
            }
            return response.isSuccess();
        } catch (const std::exception& e) {
            terminal_.writeLine("Error during commit: " + std::string(e.what()));
            return false;
        }
    }
};

} // namespace netd

int main() {
    auto& logger = netd::Logger::getInstance();
    logger.info("NETD Client starting...");

    // Initialize terminal
    netd::Terminal terminal;
    if (!terminal.initialize()) {
        std::cerr << "Failed to initialize terminal" << std::endl;
        return 1;
    }

    // Connect to server
    terminal.writeLine("Connecting to NETD server...");
    std::string socketPath = "/tmp/netd.sock";
    if (!netd::connectToServer(socketPath)) {
        terminal.writeLine("Failed to connect to NETD server");
        terminal.writeLine("Make sure the server is running with: ./netd");
        terminal.cleanup();
        return 1;
    }

    terminal.writeLine("Connected to NETD server successfully!");

    // Set up command processor
    netd::CommandProcessor processor(terminal);
    terminal.setCommandHandler([&processor](const std::string& command) {
        return processor.processCommand(command);
    });

    // Run interactive mode
    terminal.runInteractive();

    // Cleanup
    netd::disconnectFromServer();
    terminal.cleanup();
    
    logger.info("NETD Client finished");
    return 0;
}
