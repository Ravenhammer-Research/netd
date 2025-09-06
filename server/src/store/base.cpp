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

#include <libyang/tree_data.h>
#include <server/include/store/base.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>

namespace netd::server::store {

  // Core store operations
  bool Store::load() {
    throw netd::shared::NotImplementedError("Store::load not implemented");
  }

  bool Store::commit() {
    throw netd::shared::NotImplementedError("Store::commit not implemented");
  }

  // Search methods
  std::vector<lyd_node *> Store::search(const std::string &xpath) {
    std::vector<lyd_node *> results;

    if (!dataTree_) {
      return results;
    }

    // Use libyang XPath search - new API requires context
    lyd_node *match = nullptr;
    LY_ERR err = lyd_find_path(dataTree_, xpath.c_str(), 0, &match);
    if (err == LY_SUCCESS && match) {
      // For now, just add the first match
      // TODO: Handle multiple matches properly
      results.push_back(match);
    }

    return results;
  }

  std::vector<lyd_node *> Store::searchInterface(const std::string &filter) {
    std::string xpath =
        filter.empty() ? "/ietf-interfaces:interfaces/interface" : filter;
    return search(xpath);
  }

  std::vector<lyd_node *> Store::searchVRF(const std::string &filter) {
    std::string xpath = filter.empty()
                            ? "/ietf-routing:routing/control-plane-protocols/"
                              "control-plane-protocol"
                            : filter;
    return search(xpath);
  }

  std::vector<lyd_node *> Store::searchRoute(const std::string &filter) {
    std::string xpath =
        filter.empty() ? "/ietf-routing:routing/ribs/rib/route" : filter;
    return search(xpath);
  }

  // Lock/unlock methods
  void Store::lock() { storeMutex_.lock(); }

  void Store::unlock() { storeMutex_.unlock(); }

} // namespace netd::server::store
