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

#include <freebsd/include/interface/base.hpp>
#include <freebsd/include/route.hpp>
#include <freebsd/include/vrf.hpp>
#include <libyang/libyang.h>
#include <server/include/store/startup.hpp>
#include <shared/include/logger.hpp>

namespace netd::server::store::startup {

  // Singleton instance
  StartupStore &StartupStore::getInstance() {
    static StartupStore instance;
    return instance;
  }

  bool StartupStore::load() {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Loading startup configuration from FreeBSD system");

    try {
      // Create a new YANG context for startup data
      ly_ctx *ctx = nullptr;
      LY_ERR err = ly_ctx_new(nullptr, 0, &ctx);
      if (err != LY_SUCCESS || !ctx) {
        logger.error("Failed to create YANG context for startup store");
        return false;
      }

      // Load required YANG modules
      if (!ly_ctx_load_module(ctx, "ietf-interfaces", nullptr, nullptr) ||
          !ly_ctx_load_module(ctx, "ietf-routing", nullptr, nullptr)) {
        logger.error("Failed to load required YANG modules");
        ly_ctx_destroy(ctx);
        return false;
      }

      // Create root data tree
      lyd_node *root = nullptr;
      if (lyd_new_inner(nullptr,
                        ly_ctx_get_module(ctx, "ietf-interfaces", nullptr),
                        "interfaces", 0, &root) != LY_SUCCESS) {
        logger.error("Failed to create interfaces container");
        ly_ctx_destroy(ctx);
        return false;
      }

      // Enumerate interfaces from FreeBSD system
      auto interfaces = netd::freebsd::interface::Interface::getAllInterfaces();
      for (const auto &iface : interfaces) {
        if (iface) {
          // Convert interface to YANG and add to tree
          lyd_node *interfaceYang = iface->toYang(ctx);
          if (interfaceYang) {
            lyd_insert_child(root, interfaceYang);
          }
        }
      }

      // TODO: Enumerate routes from FreeBSD system
      // TODO: Enumerate VRFs from FreeBSD system

      // Set the data tree
      setDataTree(root);

      logger.info("Successfully loaded startup configuration");
      return true;

    } catch (const std::exception &e) {
      logger.error("Exception loading startup configuration: " +
                   std::string(e.what()));
      return false;
    }
  }

  bool StartupStore::commit() {
    // Startup store is read-only, commit is not applicable
    return true;
  }

  void StartupStore::clear() {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Clearing startup store");

    lyd_node *currentTree = getDataTree();
    if (currentTree) {
      lyd_free_tree(currentTree);
      setDataTree(nullptr);
    }
  }

  bool StartupStore::add(lyd_node *node) {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Adding node to startup store");

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

    logger.error("Failed to add node to startup store");
    return false;
  }

  bool StartupStore::remove(lyd_node *node) {
    auto &logger = netd::shared::Logger::getInstance();
    logger.info("Removing node from startup store");

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

} // namespace netd::server::store::startup
