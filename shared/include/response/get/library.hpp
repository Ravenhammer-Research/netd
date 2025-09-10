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

#ifndef NETD_RESPONSE_GET_LIBRARY_HPP
#define NETD_RESPONSE_GET_LIBRARY_HPP

#include <shared/include/response/get/base.hpp>
#include <string>
#include <vector>

namespace netd::shared::response::get {

  struct YangModule {
    std::string name;
    std::string revision;
    std::string namespace_;
    std::vector<std::string> features;
    std::vector<std::string> deviations;
  };

  class GetLibraryResponse : public netd::shared::response::get::GetResponse {
  public:
    GetLibraryResponse();
    GetLibraryResponse(GetLibraryResponse &&other) noexcept;
    GetLibraryResponse &operator=(GetLibraryResponse &&other) noexcept;
    virtual ~GetLibraryResponse();

    // Override base methods
    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<Response> fromYang(const ly_ctx *ctx,
                                       const lyd_node *node) override;

    // Methods to add YANG modules
    void addModule(const YangModule &module);
    void addModule(const std::string &name, const std::string &revision,
                   const std::string &namespace_);

    // Method to set pre-built YANG library data
    void setLibraryData(struct lyd_node *data);

    // Access methods
    const std::vector<YangModule> &getModules() const { return modules_; }

  private:
    std::vector<YangModule> modules_;
    struct lyd_node *libraryData_ = nullptr;  // Pre-built YANG library data
  };

} // namespace netd::shared::response::get

#endif // NETD_RESPONSE_GET_LIBRARY_HPP
