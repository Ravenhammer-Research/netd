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

#include <libyang/libyang.h>
#include <server/include/store/running.hpp>
#include <server/include/store/startup.hpp>
#include <shared/include/logger.hpp>

namespace netd::server::store::running {

  // Singleton instance
  RunningStore &RunningStore::getInstance() {
    static RunningStore instance;
    return instance;
  }

  bool RunningStore::load() {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Loading running configuration from startup store");

    try {
      // Copy data from startup store to running store
      auto &startupStore =
          netd::server::store::startup::StartupStore::getInstance();

      // Load startup configuration first
      if (!startupStore.load()) {
        logger.error("Failed to load startup configuration");
        return false;
      }

      // Clone the startup data tree for running store
      lyd_node *startupTree = startupStore.getDataTree();
      if (startupTree) {
        lyd_node *runningTree = nullptr;
        LY_ERR err = lyd_dup_single(startupTree, nullptr, LYD_DUP_RECURSIVE,
                                    &runningTree);
        if (err == LY_SUCCESS && runningTree) {
          setDataTree(runningTree);
          logger.info(
              "Successfully copied startup configuration to running store");
          return true;
        } else {
          logger.error("Failed to duplicate startup configuration");
          return false;
        }
      } else {
        logger.warning(
            "No startup configuration found, creating empty running store");
        return true;
      }

    } catch (const std::exception &e) {
      logger.error("Exception loading running configuration: " +
                   std::string(e.what()));
      return false;
    }
  }

  bool RunningStore::commit() {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info(
        "Running store commit - configuration already applied to system");

    // Running store commit is now just a confirmation
    // The actual system application happens in candidate store commit
    // This method is called after successful system application
    return true;
  }

  void RunningStore::clear() {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Clearing running store");

    lyd_node *currentTree = getDataTree();
    if (currentTree) {
      lyd_free_tree(currentTree);
      setDataTree(nullptr);
    }
  }

  bool RunningStore::add(lyd_node *node) {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Adding node to running store");

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

    logger.error("Failed to add node to running store");
    return false;
  }

  bool RunningStore::remove(lyd_node *node) {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Removing node from running store");

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

} // namespace netd::server::store::running
