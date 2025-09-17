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

#include <iomanip>
#include <iostream>
#include <shared/include/exception.hpp>
#include <shared/include/lldp/client.hpp>
#include <string>

int listLLDPNeighbors() {
  try {
    // Initialize LLDP client
    netd::shared::lldp::Client lldp_client;
    try {
      lldp_client.initialize();
    } catch (const netd::shared::LLDPError &e) {
      std::cerr << "Failed to initialize LLDP client: " << e.what()
                << std::endl;
      return 1;
    }

    // Get LLDP information
    auto ports = lldp_client.getPorts();
    auto link_local_addrs = lldp_client.getLinkLocalAddresses();

    std::cout << "LLDP Discovery Results:" << std::endl;
    std::cout << "======================" << std::endl;

    if (ports.empty()) {
      std::cout << "No LLDP ports found." << std::endl;
    } else {
      // Print table header
      std::cout << std::left;
      std::cout << std::setw(20) << "PORT" << std::setw(20) << "DESCRIPTION"
                << std::setw(15) << "NEIGHBORS" << std::setw(15) << "TTL"
                << std::endl;
      std::cout << std::string(70, '-') << std::endl;

      // Print table rows
      for (const auto &port : ports) {
        if (port && port->isValid()) {
          auto neighbors = port->getNeighbors();
          std::string neighbor_count = std::to_string(neighbors.size());
          std::string ttl = std::to_string(port->getPortTTL());

          std::cout << std::setw(20) << port->getPortName() << std::setw(20)
                    << port->getPortDescription() << std::setw(15)
                    << neighbor_count << std::setw(15) << ttl << std::endl;
        }
      }
    }

    // Show link-local addresses
    if (!link_local_addrs.empty()) {
      std::cout << "\nLink-local addresses:" << std::endl;
      std::cout << "===================" << std::endl;
      for (const auto &[interface_name, address] : link_local_addrs) {
        std::cout << interface_name << ": " << address << std::endl;
      }
    }

    // Cleanup
    lldp_client.cleanup();

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error listing LLDP neighbors: " << e.what() << std::endl;
    return 1;
  }
}
