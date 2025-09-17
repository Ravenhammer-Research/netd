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

#ifndef NETD_BASE_SERIALIZATION_HPP
#define NETD_BASE_SERIALIZATION_HPP

// Suppress GNU extension warnings from libyang headers
// https://github.com/CESNET/libyang/issues/2421
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#include <libyang/libyang.h>
#pragma clang diagnostic pop
#include <string>

namespace netd::shared::base {

  template <typename T> class Serialization {
  public:
    virtual ~Serialization() = default;

    // YANG serialization
    virtual lyd_node *toYang(ly_ctx *ctx) const = 0;
    static T fromYang(const ly_ctx *ctx, const lyd_node *node);

    // XML string conversion
    std::string toXmlString(ly_ctx *ctx) const {
      auto yang_node = toYang(ctx);
      if (!yang_node) {
        throw std::runtime_error("Failed to create YANG node");
      }

      char *xml_str = nullptr;
      if (lyd_print_mem(&xml_str, yang_node, LYD_XML, LYD_PRINT_WITHSIBLINGS) !=
          LY_SUCCESS) {
        lyd_free_tree(yang_node);
        throw std::runtime_error("Failed to convert YANG to XML");
      }

      std::string result(xml_str);
      free(xml_str);
      lyd_free_tree(yang_node);

      return result;
    }
  };

} // namespace netd::shared::base

#endif // NETD_BASE_SERIALIZATION_HPP