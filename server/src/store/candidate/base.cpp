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

#include <freebsd/include/interface/bridge.hpp>
#include <freebsd/include/interface/ethernet.hpp>
#include <freebsd/include/interface/vlan.hpp>
#include <libyang/libyang.h>
#include <libyang/tree_data.h>
#include <server/include/store/candidate.hpp>
#include <server/include/store/running.hpp>
#include <server/include/store/startup.hpp>
#include <shared/include/interface/base/ether.hpp>
#include <shared/include/logger.hpp>

namespace netd::server::store::candidate {

  // Singleton instance
  CandidateStore &CandidateStore::getInstance() {
    static CandidateStore instance;
    return instance;
  }

  // Note: load() method not declared in header, removing implementation

  void CandidateStore::clear() {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Clearing candidate store");

    lyd_node *currentTree = getDataTree();
    if (currentTree) {
      lyd_free_tree(currentTree);
      setDataTree(nullptr);
    }
  }

  bool CandidateStore::add(lyd_node *node) {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Adding node to candidate store");

    lyd_node *currentTree = getDataTree();
    if (!currentTree) {
      setDataTree(node);
      return true;
    }

    LY_ERR err = lyd_merge_tree(&currentTree, node, LYD_MERGE_DESTRUCT);
    if (err == LY_SUCCESS) {
      setDataTree(currentTree);
      return true;
    }

    logger.error("Failed to add node to candidate store");
    return false;
  }

  bool CandidateStore::remove(lyd_node *node) {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Removing node from candidate store");

    lyd_node *currentTree = getDataTree();
    if (!currentTree) {
      return true; // Nothing to remove
    }

    lyd_node *found = nullptr;
    LY_ERR err = lyd_find_path(
        currentTree, lyd_path(node, LYD_PATH_STD, nullptr, 0), 0, &found);
    if (err == LY_SUCCESS && found) {
      lyd_free_tree(found);
      return true;
    }

    return false;
  }

  bool CandidateStore::commit() {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Committing candidate configuration to system");

    try {
      lyd_node *candidateTree = getDataTree();
      if (!candidateTree) {
        logger.warning("No candidate configuration found");
        return true;
      }

      // Phase 1: Try to apply candidate configuration to the system
      logger.info("Phase 1: Applying candidate configuration to system");
      if (!applyConfigurationToSystem(candidateTree)) {
        logger.error("Failed to apply candidate configuration to system - "
                     "aborting commit");
        return false;
      }

      // Phase 2: If system application succeeded, merge candidate with running
      logger.info(
          "Phase 2: Merging candidate configuration with running store");
      auto &runningStore =
          netd::server::store::running::RunningStore::getInstance();

      // Clone the candidate data tree for running store
      lyd_node *runningTree = nullptr;
      LY_ERR err = lyd_dup_single(candidateTree, nullptr, LYD_DUP_RECURSIVE,
                                  &runningTree);
      if (err == LY_SUCCESS && runningTree) {
        runningStore.setDataTree(runningTree);
      } else {
        logger.error(
            "Failed to duplicate candidate configuration for running store");
        return false;
      }

      logger.info(
          "Successfully committed candidate configuration to running store");
      return true;

    } catch (const std::exception &e) {
      logger.error("Exception committing candidate configuration: " +
                   std::string(e.what()));
      return false;
    }
  }

  bool CandidateStore::applyConfigurationToSystem(lyd_node *configTree) {
    // Suppress unused parameter warning
    (void)configTree;
    auto &logger = netd::shared::Logger::getInstance();

    try {
      // Apply interface configuration changes
      auto interfaceNodes = search("/ietf-interfaces:interfaces/interface");
      for (const auto &interfaceNode : interfaceNodes) {
        if (interfaceNode) {
          // Extract interface name
          lyd_node *nameNode = lyd_child(interfaceNode);
          while (nameNode &&
                 strcmp(lyd_node_schema(nameNode)->name, "name") != 0) {
            nameNode = nameNode->next;
          }

          if (nameNode && lyd_get_value(nameNode)) {
            std::string ifName = lyd_get_value(nameNode);

            // Extract interface type
            std::string ifType = "ethernetCsmacd"; // default
            lyd_node *typeNode = lyd_child(interfaceNode);
            while (typeNode &&
                   strcmp(lyd_node_schema(typeNode)->name, "type") != 0) {
              typeNode = typeNode->next;
            }
            if (typeNode && lyd_get_value(typeNode)) {
              ifType = lyd_get_value(typeNode);
            }

            // Extract enabled status
            bool enabled = true; // default
            lyd_node *enabledNode = lyd_child(interfaceNode);
            while (enabledNode &&
                   strcmp(lyd_node_schema(enabledNode)->name, "enabled") != 0) {
              enabledNode = enabledNode->next;
            }
            if (enabledNode && lyd_get_value(enabledNode)) {
              enabled = (strcmp(lyd_get_value(enabledNode), "true") == 0);
            }

            // Extract MTU
            uint32_t mtu = 1500; // default
            lyd_node *mtuNode = lyd_child(interfaceNode);
            while (mtuNode &&
                   strcmp(lyd_node_schema(mtuNode)->name, "mtu") != 0) {
              mtuNode = mtuNode->next;
            }
            if (mtuNode && lyd_get_value(mtuNode)) {
              mtu = std::stoul(lyd_get_value(mtuNode));
            }

            logger.info("Applying interface " + ifName + " type=" + ifType +
                        " enabled=" + (enabled ? "true" : "false") +
                        " mtu=" + std::to_string(mtu));

            // Create appropriate FreeBSD interface object based on type
            std::unique_ptr<netd::freebsd::interface::EthernetInterface>
                freebsdInterface;

            if (ifType == "ethernetCsmacd") {
              freebsdInterface =
                  std::make_unique<netd::freebsd::interface::EthernetInterface>(
                      ifName);
            } else {
              logger.warning("Unsupported interface type: " + ifType +
                             " for interface " + ifName);
              continue; // Skip unsupported interface types
            }

            // Configure the interface
            if (freebsdInterface) {
              // Set MTU
              if (!freebsdInterface->setMTU(mtu)) {
                logger.error("Failed to set MTU for interface " + ifName);
                return false;
              }

              // Set enabled/disabled status
              if (enabled) {
                if (!freebsdInterface->up()) {
                  logger.error("Failed to bring up interface " + ifName);
                  return false;
                }
              } else {
                if (!freebsdInterface->down()) {
                  logger.error("Failed to bring down interface " + ifName);
                  return false;
                }
              }

              // Apply configuration to the system
              if (!freebsdInterface->applyToSystem()) {
                logger.error(
                    "Failed to apply configuration to system for interface " +
                    ifName);
                return false;
              }

              logger.info("Successfully applied configuration for interface " +
                          ifName);
            }
          }
        }
      }

      // TODO: Apply route configuration changes
      // TODO: Apply VRF configuration changes

      logger.info("Successfully applied candidate configuration to system");
      return true;

    } catch (const std::exception &e) {
      logger.error("Exception applying configuration to system: " +
                   std::string(e.what()));
      return false;
    }
  }

} // namespace netd::server::store::candidate
