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

#ifndef NETD_MARSHALLING_FILTER_HPP
#define NETD_MARSHALLING_FILTER_HPP

#include <memory>
#include <shared/include/base/serialization.hpp>
#include <string>

namespace netd::shared::marshalling {

  // Filter type enumeration
  enum class FilterType { SUBTREE, XPATH };

  class Filter {
  public:
    Filter();
    virtual ~Filter() = default;

    // Pure virtual methods that must be implemented by subclasses
    virtual lyd_node *toYang(ly_ctx *ctx) const = 0;
    virtual std::unique_ptr<Filter> fromYang(const ly_ctx *ctx,
                                             const lyd_node *node) = 0;

    // Getter and setter for filter type
    FilterType getType() const { return type; }
    void setType(FilterType t) { type = t; }

  protected:
    FilterType type = FilterType::SUBTREE;
  };

  // Subtree filter implementation
  class SubtreeFilter : public Filter {
  public:
    SubtreeFilter();
    virtual ~SubtreeFilter() = default;

    // Override base methods
    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<Filter> fromYang(const ly_ctx *ctx,
                                     const lyd_node *node) override;

    // Subtree-specific methods
    void setSubtree(const std::string &subtree) { subtreeData = subtree; }
    const std::string &getSubtree() const { return subtreeData; }

  private:
    std::string subtreeData;
  };

  // XPath filter implementation
  class XPathFilter : public Filter {
  public:
    XPathFilter();
    virtual ~XPathFilter() = default;

    // Override base methods
    lyd_node *toYang(ly_ctx *ctx) const override;
    std::unique_ptr<Filter> fromYang(const ly_ctx *ctx,
                                     const lyd_node *node) override;

    // XPath-specific methods
    void setXPath(const std::string &xpath) { xpathData = xpath; }
    const std::string &getXPath() const { return xpathData; }

  private:
    std::string xpathData;
  };

} // namespace netd::shared::marshalling

#endif // NETD_MARSHALLING_FILTER_HPP
