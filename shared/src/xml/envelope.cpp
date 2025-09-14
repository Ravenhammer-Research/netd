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

#include <shared/include/xml/envelope.hpp>
#include <shared/include/xml/base.hpp>
#include <shared/include/exception.hpp>
#include <shared/include/yang.hpp>
#include <sstream>
#include <stdexcept>
#include <bsdxml.h>

namespace netd::shared::xml {

  struct XmlParseState {
    RpcEnvelope* envelope;
    const struct ly_ctx* ctx;
    std::string currentElement;
    std::string currentOperation;
    std::string currentData;
    bool inData = false;
    bool inError = false;
    bool inFilter = false;
    std::string filterType;
    std::string xpathSelect;
  };
  
  RpcEnvelope::RpcEnvelope() = default;

std::unique_ptr<RpcEnvelope> RpcEnvelope::fromXml(const std::string& xml, const struct ly_ctx* ctx) {
    auto envelope = std::make_unique<RpcEnvelope>();

    if (xml.empty()) {
      throw XmlParseError("Empty XML string provided");
    }

    XML_Parser parser = XML_ParserCreate(nullptr);
    if (!parser) {
      throw XmlParseError("Failed to create XML parser");
    }

    XmlParseState state;
    state.envelope = envelope.get();
    state.ctx = ctx;
    XML_SetUserData(parser, &state);

    XML_SetElementHandler(parser, 
                         [](void* userData, const XML_Char* name, const XML_Char** atts) {
                           XmlParseState* state = static_cast<XmlParseState*>(userData);
                           state->envelope->startElementHandler(userData, name, atts);
                         },
                         [](void* userData, const XML_Char* name) {
                           XmlParseState* state = static_cast<XmlParseState*>(userData);
                           state->envelope->endElementHandler(userData, name);
                         });
    XML_SetCharacterDataHandler(parser, 
                               [](void* userData, const XML_Char* s, int len) {
                                 XmlParseState* state = static_cast<XmlParseState*>(userData);
                                 state->envelope->characterDataHandler(userData, s, len);
                               });

    enum XML_Status status = XML_Parse(parser, xml.c_str(), static_cast<int>(xml.length()), 1);
    
    if (status == XML_STATUS_ERROR) {
      enum XML_Error error = XML_GetErrorCode(parser);
      const XML_LChar* errorString = XML_ErrorString(error);
      XML_ParserFree(parser);
      throw XmlParseError("XML parsing failed: " + std::string(errorString));
    }

    XML_ParserFree(parser);
    return envelope;
  }

std::unique_ptr<RpcEnvelope> RpcEnvelope::toXml(RpcType rpc_type, 
                                               int message_id, 
                                               netconf::NetconfOperation operation,
                                               marshalling::Filter* filter,
                                               lyd_node* lyd_data,
                                               const struct ly_ctx* ctx [[maybe_unused]]) {
    if (rpc_type == RpcType::RPC && message_id <= 0) {
      throw XmlValidationError("Invalid message-id for RPC request");
    }

    if (rpc_type == RpcType::RPC) {
      const char* operationName = netconf::operationToString(operation);
      if (!operationName) {
        throw XmlValidationError("Invalid NETCONF operation");
      }
    }

    auto envelope = std::make_unique<RpcEnvelope>();
    
    envelope->rpc_type_ = rpc_type;
    envelope->message_id_ = message_id;
    envelope->operation_ = operation;
    
    if (filter) {
      envelope->filter_ = std::unique_ptr<marshalling::Filter>(filter);
    }
    
    envelope->lyd_data_ = lyd_data;

    return envelope;
  }

std::stringstream RpcEnvelope::toXmlStream(const struct ly_ctx* ctx) const {
    std::stringstream xml;
    xml << XmlHeader::toString();
    
    std::string rootElementName;
    switch (rpc_type_) {
      case RpcType::RPC:
        rootElementName = RPC_ELEMENT;
        break;
      case RpcType::RPC_REPLY:
        rootElementName = RPC_REPLY_ELEMENT;
        break;
      case RpcType::RPC_ERROR:
        rootElementName = RPC_ERROR_ELEMENT;
        break;
      default:
        rootElementName = RPC_ELEMENT;
        break;
    }
    
    XmlElement rootElement = RpcElement::create(rootElementName, message_id_);

    if (rpc_type_ == RpcType::RPC) {
      const char* operationName = netconf::operationToString(operation_);
      if (operationName) {
        XmlElement operation = OperationElement::create(operationName);
        
        if (filter_) {
          if (filter_->getType() == marshalling::FilterType::SUBTREE) {
            XmlElement filter = FilterElement::createSubtree();
            if (lyd_data_) {
              char* dataXml = nullptr;
              if (lyd_print_mem(&dataXml, lyd_data_, LYD_XML, LYD_PRINT_WITHSIBLINGS) == LY_SUCCESS) {
                filter.setContent(std::string(dataXml));
                free(dataXml);
              } else {
                throw YangDataError(const_cast<void*>(static_cast<const void*>(ctx)));
              }
            }
            operation.addChild(filter);
          } else if (filter_->getType() == marshalling::FilterType::XPATH) {
            auto* xpathFilter = dynamic_cast<marshalling::XPathFilter*>(filter_.get());
            if (xpathFilter) {
              XmlElement filter = FilterElement::createXPath(xpathFilter->getXPath());
              if (lyd_data_) {
                char* dataXml = nullptr;
                if (lyd_print_mem(&dataXml, lyd_data_, LYD_XML, LYD_PRINT_WITHSIBLINGS) == LY_SUCCESS) {
                  filter.setContent(std::string(dataXml));
                  free(dataXml);
                } else {
                  throw YangDataError(const_cast<void*>(static_cast<const void*>(ctx)));
                }
              }
              operation.addChild(filter);
            } else {
              throw XmlSerializationError("Failed to cast filter to XPathFilter");
            }
          }
        }
        
        if (lyd_data_ && !filter_) {
          char* dataXml = nullptr;
          if (lyd_print_mem(&dataXml, lyd_data_, LYD_XML, LYD_PRINT_WITHSIBLINGS) == LY_SUCCESS) {
            operation.setContent(std::string(dataXml));
            free(dataXml);
          } else {
            throw YangDataError(const_cast<void*>(static_cast<const void*>(ctx)));
          }
        }
        
        rootElement.addChild(operation);
      }
    } else if (rpc_type_ == RpcType::RPC_REPLY) {
      if (lyd_data_) {
        XmlElement data = DataElement::create();
        char* dataXml = nullptr;
        if (lyd_print_mem(&dataXml, lyd_data_, LYD_XML, LYD_PRINT_WITHSIBLINGS) == LY_SUCCESS) {
          data.setContent(std::string(dataXml));
          free(dataXml);
        } else {
          throw YangDataError(const_cast<void*>(static_cast<const void*>(ctx)));
        }
        rootElement.addChild(data);
      }
    } else if (rpc_type_ == RpcType::RPC_ERROR) {
      if (lyd_data_) {
        XmlElement error = ErrorElement::create();
        char* errorXml = nullptr;
        if (lyd_print_mem(&errorXml, lyd_data_, LYD_XML, LYD_PRINT_WITHSIBLINGS) == LY_SUCCESS) {
          error.setContent(std::string(errorXml));
          free(errorXml);
        } else {
          throw YangDataError(const_cast<void*>(static_cast<const void*>(ctx)));
        }
        rootElement.addChild(error);
      }
    }

    xml << rootElement.toString();
    return xml;
  }


  void RpcEnvelope::startElementHandler(void* userData, const XML_Char* name, const XML_Char** atts) {
    if (!userData || !name) return;
    
    XmlParseState* state = static_cast<XmlParseState*>(userData);
    RpcEnvelope* envelope = state->envelope;
    state->currentElement = name;

    if (strcmp(name, RPC_ELEMENT) == 0) {
      envelope->rpc_type_ = RpcType::RPC;
    } else if (strcmp(name, RPC_REPLY_ELEMENT) == 0) {
      envelope->rpc_type_ = RpcType::RPC_REPLY;
    } else if (strcmp(name, RPC_ERROR_ELEMENT) == 0) {
      envelope->rpc_type_ = RpcType::RPC_ERROR;
    } else if (strcmp(name, DATA_NAME) == 0) {
      state->inData = true;
    } else if (strcmp(name, ERROR_NAME) == 0) {
      state->inError = true;
    } else if (strcmp(name, FILTER_NAME) == 0) {
      state->inFilter = true;
      for (int i = 0; atts[i] != nullptr; i += 2) {
        if (strcmp(atts[i], TYPE_ATTR) == 0) {
          state->filterType = atts[i + 1];
        } else if (strcmp(atts[i], SELECT_ATTR) == 0) {
          state->xpathSelect = atts[i + 1];
        }
      }
    } else if (envelope->rpc_type_ == RpcType::RPC && !state->inFilter && !state->inData) {
      state->currentOperation = name;
    }

    for (int i = 0; atts[i] != nullptr; i += 2) {
      if (strcmp(atts[i], MESSAGE_ID_ATTR) == 0) {
        try {
          envelope->message_id_ = std::stoi(atts[i + 1]);
        } catch (const std::exception&) {
        }
        break;
      }
    }
  }

  void RpcEnvelope::endElementHandler(void* userData, const XML_Char* name) {
    if (!userData || !name) return;
    
    XmlParseState* state = static_cast<XmlParseState*>(userData);
    RpcEnvelope* envelope = state->envelope;
    const struct ly_ctx* yang_ctx = state->ctx;

    if (strcmp(name, DATA_NAME) == 0) {
      state->inData = false;
      if (!state->currentData.empty()) {
        lyd_node* dataNode = nullptr;
        if (lyd_parse_data_mem(yang_ctx, state->currentData.c_str(), LYD_XML, 0, 0, &dataNode) == LY_SUCCESS) {
          envelope->lyd_data_ = dataNode;
        } else {
          throw YangDataError(const_cast<void*>(static_cast<const void*>(yang_ctx)));
        }
        state->currentData.clear();
      }
    } else if (strcmp(name, ERROR_NAME) == 0) {
      state->inError = false;
      if (!state->currentData.empty()) {
        lyd_node* errorNode = nullptr;
        if (lyd_parse_data_mem(yang_ctx, state->currentData.c_str(), LYD_XML, 0, 0, &errorNode) == LY_SUCCESS) {
          envelope->lyd_data_ = errorNode;
        } else {
          throw YangDataError(const_cast<void*>(static_cast<const void*>(yang_ctx)));
        }
        state->currentData.clear();
      }
    } else if (strcmp(name, FILTER_NAME) == 0) {
      state->inFilter = false;
      if (state->filterType == SUBTREE_TYPE && !state->currentData.empty()) {
        auto filter = std::make_unique<marshalling::SubtreeFilter>();
        filter->setSubtree(state->currentData);
        envelope->filter_ = std::move(filter);
      } else if (state->filterType == XPATH_TYPE && !state->xpathSelect.empty()) {
        auto filter = std::make_unique<marshalling::XPathFilter>();
        filter->setXPath(state->xpathSelect);
        envelope->filter_ = std::move(filter);
      }
      state->currentData.clear();
      state->filterType.clear();
      state->xpathSelect.clear();
    } else if (envelope->rpc_type_ == RpcType::RPC && !state->currentOperation.empty() && strcmp(name, state->currentOperation.c_str()) == 0) {
      envelope->operation_ = netconf::stringToOperation(state->currentOperation);
      state->currentOperation.clear();
    }
  }

  void RpcEnvelope::characterDataHandler(void* userData, const XML_Char* s, int len) {
    if (!userData || !s || len <= 0) return;
    
    XmlParseState* state = static_cast<XmlParseState*>(userData);
    
    if (state->inData || state->inError || state->inFilter) {
      state->currentData.append(s, len);
    }
  }

} // namespace netd::shared::xml
