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

#include <libnetconf2/messages_server.h>
#include <libnetconf2/netconf.h>
#include <libyang/libyang.h>
#include <libyang/tree_data.h>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/response/get/library.hpp>
#include <shared/include/yang.hpp>

namespace netd::shared::response::get {

  // Copy of the private struct from libnetconf2/src/messages_p.h
  struct nc_server_reply_data {
    NC_RPL type;
    struct lyd_node
        *data; /**< always points to the operation, for both RPCs and actions */
    int free;
    NC_WD_MODE wd;
  };

  GetLibraryResponse::GetLibraryResponse() {}

  GetLibraryResponse::GetLibraryResponse(GetLibraryResponse &&other) noexcept
      : GetResponse(), modules_(std::move(other.modules_)) {}

  GetLibraryResponse &
  GetLibraryResponse::operator=(GetLibraryResponse &&other) noexcept {
    if (this != &other) {
      modules_ = std::move(other.modules_);
    }
    return *this;
  }

  GetLibraryResponse::~GetLibraryResponse() {}

  void
  GetLibraryResponse::addModule([[maybe_unused]] const YangModule &module) {
    throw netd::shared::NotImplementedError(
        "GetLibraryResponse::addModule not implemented");
  }

  void GetLibraryResponse::addModule(
      [[maybe_unused]] const std::string &name,
      [[maybe_unused]] const std::string &revision,
      [[maybe_unused]] const std::string &namespace_) {
    throw netd::shared::NotImplementedError(
        "GetLibraryResponse::addModule not implemented");
  }

  void
  GetLibraryResponse::setLibraryData([[maybe_unused]] struct lyd_node *data) {
    throw netd::shared::NotImplementedError(
        "GetLibraryResponse::setLibraryData not implemented");
  }

  lyd_node *GetLibraryResponse::toYang(ly_ctx *ctx) const {
    if (!ctx) {
      throw netd::shared::ArgumentError("toYang: ctx is null");
    }

    // If we have pre-built YANG library data, return it directly
    if (libraryData_) {
      // Clone the data to avoid ownership issues
      lyd_node *cloned_data = nullptr;
      if (lyd_dup_single(libraryData_, nullptr, LYD_DUP_RECURSIVE,
                         &cloned_data) != LY_SUCCESS) {
        throw netd::shared::ArgumentError(
            "toYang: failed to clone YANG library data");
      }
      return cloned_data;
    }

    // If no pre-built data, create empty modules-state container
    const struct lys_module *yanglib_mod =
        ly_ctx_get_module(ctx, "ietf-yang-library", "2019-01-04");
    if (!yanglib_mod) {
      throw netd::shared::ArgumentError(
          "toYang: ietf-yang-library module not found");
    }

    // Create the modules-state container
    lyd_node *modulesState = nullptr;
    if (lyd_new_inner(nullptr, yanglib_mod, "modules-state", 0,
                      &modulesState) != LY_SUCCESS) {
      throw netd::shared::ArgumentError(
          "toYang: failed to create modules-state container");
    }

    return modulesState;
  }

  std::unique_ptr<Response>
  GetLibraryResponse::fromYang([[maybe_unused]] const ly_ctx *ctx,
                               [[maybe_unused]] const lyd_node *node) {
    throw netd::shared::NotImplementedError(
        "GetLibraryResponse::fromYang not implemented");
  }

} // namespace netd::shared::response::get
