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

#include <shared/include/xml/base.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/xml/envelope.hpp>
#include <shared/include/xml/hello.hpp>
#include <sstream>
#include <stdexcept>
#include <map>
#include <vector>
#include <algorithm>
#include <bsdxml.h>

namespace netd::shared::xml {


  XmlElement& XmlElement::addAttribute(const std::string& key, const std::string& value) {
    attributes[key] = value;
    return *this;
  }

  XmlElement& XmlElement::setContent(const std::string& c) {
    content = c;
    return *this;
  }

  XmlElement& XmlElement::addChild(const XmlElement& child) {
    children.push_back(child);
    return *this;
  }

  std::string XmlElement::toString() const {
    std::stringstream xml;
    xml << XmlElement::OPEN_TAG 
      << name;
    
    for (const auto& attr : attributes) {
      xml << XmlElement::SPACE 
      << attr.first 
      << XmlElement::EQUALS
      << XmlElement::QUOTE 
      << attr.second 
      << XmlElement::QUOTE;
    }
    
    if (content.empty() && children.empty()) {
      xml << XmlElement::SELF_CLOSE_TAG;
    } else {
      xml << XmlElement::CLOSE_TAG;
      xml << content;
      for (const auto& child : children) {
        xml << child.toString();
      }
      xml << XmlElement::CLOSE_OPEN_TAG 
          << name 
          << XmlElement::CLOSE_TAG;
    }
    
    return xml.str();
  }

  std::string XmlHeader::toString() {
    return XML_DECL_START;
  }

  XmlElement RpcElement::create(const std::string& type, int messageId) {
    XmlElement element(type);
    if (messageId > 0) {
      element.addAttribute(MESSAGE_ID_ATTR, std::to_string(messageId));
    }
    element.addAttribute(XMLNS_ATTR, NETCONF_NAMESPACE);
    return element;
  }

  XmlElement FilterElement::createSubtree() {
    XmlElement element(FILTER_NAME);
    element.addAttribute(TYPE_ATTR, SUBTREE_TYPE);
    return element;
  }

  XmlElement FilterElement::createXPath(const std::string& xpath) {
    XmlElement element(FILTER_NAME);
    element.addAttribute(TYPE_ATTR, XPATH_TYPE);
    element.addAttribute(SELECT_ATTR, xpath);
    return element;
  }

  XmlElement DataElement::create() {
    return XmlElement(DATA_NAME);
  }

  XmlElement ErrorElement::create() {
    return XmlElement(ERROR_NAME);
  }

  XmlElement OperationElement::create(const std::string& operationName) {
    return XmlElement(operationName);
  }

  struct MessageTypeParseState {
    bool foundHello = false;
    bool foundRpc = false;
    bool foundRpcReply = false;
    bool foundRpcError = false;
    bool parsingError = false;
  };

  void messageTypeStartElementHandler(void* userData, const XML_Char* name, const XML_Char** atts [[maybe_unused]]) {
    if (!userData || !name) return;
    
    MessageTypeParseState* state = static_cast<MessageTypeParseState*>(userData);
    
    if (strcmp(name, HELLO_ELEMENT) == 0) {
      state->foundHello = true;
    } else if (strcmp(name, RPC_ELEMENT) == 0) {
      state->foundRpc = true;
    } else if (strcmp(name, RPC_REPLY_ELEMENT) == 0) {
      state->foundRpcReply = true;
    } else if (strcmp(name, RPC_ERROR_ELEMENT) == 0) {
      state->foundRpcError = true;
    }
  }

  bool isHelloMessage(const std::string& xml) {
    if (xml.empty()) {
      return false;
    }

    XML_Parser parser = XML_ParserCreate(nullptr);
    if (!parser) {
      return false;
    }

    MessageTypeParseState state;
    XML_SetUserData(parser, &state);
    XML_SetElementHandler(parser, messageTypeStartElementHandler, nullptr);

    enum XML_Status status = XML_Parse(parser, xml.c_str(), static_cast<int>(xml.length()), 1);
    
    XML_ParserFree(parser);
    
    return status != XML_STATUS_ERROR && state.foundHello && 
           !state.foundRpc && !state.foundRpcReply && !state.foundRpcError;
  }

  bool isRpcMessage(const std::string& xml) {
    if (xml.empty()) {
      return false;
    }

    XML_Parser parser = XML_ParserCreate(nullptr);
    if (!parser) {
      return false;
    }

    MessageTypeParseState state;
    XML_SetUserData(parser, &state);
    XML_SetElementHandler(parser, messageTypeStartElementHandler, nullptr);

    enum XML_Status status = XML_Parse(parser, xml.c_str(), static_cast<int>(xml.length()), 1);
    
    XML_ParserFree(parser);
    
    return status != XML_STATUS_ERROR && 
           (state.foundRpc || state.foundRpcReply || state.foundRpcError);
  }

  // XMLTree implementation
  std::unique_ptr<XMLTree> XMLTree::fromXml([[maybe_unused]] const std::string& xml, [[maybe_unused]] const struct ly_ctx* ctx) {
    // This is a factory method that should be overridden by derived classes
    // For now, we'll throw an exception as this should not be called directly
    throw std::runtime_error("XMLTree::fromXml should be overridden by derived classes");
  }

  std::string XMLTree::toString(const struct ly_ctx* ctx) const {
    std::stringstream stream = toXmlStream(ctx);
    return stream.str();
  }

  bool XMLTree::validate(const struct ly_ctx* ctx) const {
    std::string xml = toString(ctx);
    if (xml.empty()) {
      return false;
    }
    
    lyd_node* dataNode = nullptr;
    LY_ERR result = lyd_parse_data_mem(ctx, xml.c_str(), LYD_XML, 0, 0, &dataNode);
    
    if (dataNode) {
      lyd_free_tree(dataNode);
    }
    
    return result == LY_SUCCESS;
  }

} // namespace netd::shared::xml
