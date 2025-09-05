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

#ifndef NETD_YANG_HPP
#define NETD_YANG_HPP

#include <libyang/libyang.h>
#include <memory>
#include <string>

namespace netd::shared {

  class Yang {
  public:
    // Singleton access
    static Yang &getInstance();

    // Delete copy constructor and assignment operator
    Yang(const Yang &) = delete;
    Yang &operator=(const Yang &) = delete;

    // YANG context management
    ly_ctx *getContext() const;
    bool loadSchema(const std::string &schemaPath);
    bool loadSchemaByName(const std::string &name,
                          const std::string &revision = "");

    // Utility functions for YANG data conversion
    static std::string yangToXml(const lyd_node *node);
    static std::string yangToJson(const lyd_node *node);
    static lyd_node *xmlToYang(ly_ctx *ctx, const std::string &xml);
    static lyd_node *jsonToYang(ly_ctx *ctx, const std::string &json);

  protected:
    // Protected constructor for singleton
    Yang();
    virtual ~Yang();

  private:
    ly_ctx *ctx_;
    void loadStandardSchemas();
  };

} // namespace netd::shared

#endif // NETD_YANG_HPP
