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

#ifndef NETD_SHARED_XML_BASE_HPP
#define NETD_SHARED_XML_BASE_HPP

#include <bsdxml.h>
#include <libyang/libyang.h>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace netd::shared::xml {

  // XML element constants
  static constexpr const char *XML_DECL_START =
      R"(<?xml version="1.0" encoding="UTF-8"?>)";
  static constexpr const char *MESSAGE_ID_ATTR = R"(message-id)";
  static constexpr const char *XMLNS_ATTR = R"(xmlns)";
  static constexpr const char *NETCONF_NAMESPACE =
      R"(urn:ietf:params:xml:ns:netconf:base:1.0)";
  static constexpr const char *FILTER_NAME = R"(filter)";
  static constexpr const char *TYPE_ATTR = R"(type)";
  static constexpr const char *SELECT_ATTR = R"(select)";
  static constexpr const char *SUBTREE_TYPE = R"(subtree)";
  static constexpr const char *XPATH_TYPE = R"(xpath)";
  static constexpr const char *DATA_NAME = R"(data)";
  static constexpr const char *ERROR_NAME = R"(error)";
  static constexpr const char *RPC_ELEMENT = R"(rpc)";
  static constexpr const char *RPC_REPLY_ELEMENT = R"(rpc-reply)";
  static constexpr const char *RPC_ERROR_ELEMENT = R"(rpc-error)";

  // XML element helper struct
  struct XmlElement {
    // XML syntax constants
    static constexpr const char OPEN_TAG[] = {0x3C, 0x00};             // '<'
    static constexpr const char CLOSE_TAG[] = {0x3E, 0x00};            // '>'
    static constexpr const char SELF_CLOSE_TAG[] = {0x2F, 0x3E, 0x00}; // '/>'
    static constexpr const char QUOTE[] = {0x22, 0x00};                // '"'
    static constexpr const char SPACE[] = {0x20, 0x00};                // ' '
    static constexpr const char EQUALS[] = {0x3D, 0x00};               // '='
    static constexpr const char CLOSE_OPEN_TAG[] = {0x3C, 0x2F, 0x00}; // '</'

    std::string name;
    std::map<std::string, std::string> attributes;
    std::string content;
    std::vector<XmlElement> children;

    XmlElement(const std::string &n) : name(n) {}

    XmlElement &addAttribute(const std::string &key, const std::string &value);
    XmlElement &setContent(const std::string &c);
    XmlElement &addChild(const XmlElement &child);
    std::string toString() const;
  };

  // Helper structs for XML element creation
  struct XmlHeader {
    static std::string toString();
  };

  struct RpcElement {
    static XmlElement create(const std::string &type, int messageId = 0);
  };

  struct FilterElement {
    static XmlElement createSubtree();
    static XmlElement createXPath(const std::string &xpath);
  };

  struct DataElement {
    static XmlElement create();
  };

  struct ErrorElement {
    static XmlElement create();
  };

  struct OperationElement {
    static XmlElement create(const std::string &operationName);
  };

  // Abstract base class for XML tree structures
  class XMLTree {
  public:
    XMLTree() = default;
    virtual ~XMLTree() = default;

    // Parse XML string into object
    static std::unique_ptr<XMLTree> fromXml(const std::string &xml,
                                            const struct ly_ctx *ctx);

    // Serialize to XML stream
    virtual std::stringstream toXmlStream(const struct ly_ctx *ctx) const = 0;

    // Convert to string representation
    virtual std::string toString(const struct ly_ctx *ctx) const;

    // Validate by converting to XML and parsing it
    virtual bool validate(const struct ly_ctx *ctx) const;

  protected:
    // Helper methods for XML parsing (to be implemented by derived classes)
    virtual void startElementHandler(void *userData, const XML_Char *name,
                                     const XML_Char **atts) = 0;
    virtual void endElementHandler(void *userData, const XML_Char *name) = 0;
    virtual void characterDataHandler(void *userData, const XML_Char *s,
                                      int len) = 0;
  };

  // Utility functions for message type detection
  bool isHelloMessage(const std::string &xml);
  bool isRpcMessage(const std::string &xml);

} // namespace netd::shared::xml

#endif // NETD_SHARED_XML_BASE_HPP
