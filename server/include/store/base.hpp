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

#ifndef NETD_SERVER_STORE_BASE_HPP
#define NETD_SERVER_STORE_BASE_HPP

#include <libyang/tree_data.h>
#include <mutex>
#include <string>
#include <vector>

namespace netd::server::store {

  class Store {
  public:
    Store() = default;
    virtual ~Store() = default;

    // Core store operations
    virtual bool load();
    virtual bool commit();
    virtual void clear() = 0;
    virtual bool add(lyd_node *node) = 0;
    virtual bool remove(lyd_node *node) = 0;

    // Search methods
    std::vector<lyd_node *> search(const std::string &xpath);

    // Generic search method that accepts request objects
    template <typename RequestType>
    std::vector<lyd_node *> search(const RequestType &request
                                   [[maybe_unused]]) {
      // Default implementation - can be overridden by derived classes
      // For now, just return empty results
      return std::vector<lyd_node *>();
    }

    // Data tree access
    lyd_node *getDataTree() const { return dataTree_; }
    void setDataTree(lyd_node *tree) { dataTree_ = tree; }

    // Lock/unlock methods
    void lock();
    void unlock();

  protected:
    lyd_node *dataTree_ = nullptr;

  private:
    std::mutex storeMutex_;
  };

} // namespace netd::server::store

#endif // NETD_SERVER_STORE_BASE_HPP
