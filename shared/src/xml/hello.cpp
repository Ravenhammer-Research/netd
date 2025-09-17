#include <bsdxml.h>
#include <shared/include/exception.hpp>
#include <shared/include/xml/base.hpp>
#include <shared/include/xml/hello.hpp>
#include <sstream>

namespace netd::shared::xml {

  // Hello base class implementation
  std::unique_ptr<Hello>
  Hello::fromXml([[maybe_unused]] const std::string &xml,
                 [[maybe_unused]] const struct ly_ctx *ctx) {
    throw NotImplementedError("Hello::fromXml not implemented");
  }

  std::stringstream
  Hello::toXmlStream([[maybe_unused]] const struct ly_ctx *ctx) const {
    throw NotImplementedError("Hello::toXmlStream not implemented");
  }

  void Hello::startElementHandler([[maybe_unused]] void *userData,
                                  [[maybe_unused]] const XML_Char *name,
                                  [[maybe_unused]] const XML_Char **atts) {
    throw NotImplementedError("Hello::startElementHandler not implemented");
  }

  void Hello::endElementHandler([[maybe_unused]] void *userData,
                                [[maybe_unused]] const XML_Char *name) {
    throw NotImplementedError("Hello::endElementHandler not implemented");
  }

  void Hello::characterDataHandler([[maybe_unused]] void *userData,
                                   [[maybe_unused]] const XML_Char *s,
                                   [[maybe_unused]] int len) {
    throw NotImplementedError("Hello::characterDataHandler not implemented");
  }

  // HelloToClient implementation
  std::unique_ptr<HelloToClient>
  HelloToClient::toXml(uint32_t sessionId,
                       const std::vector<std::string> &capabilities,
                       [[maybe_unused]] const struct ly_ctx *ctx) {
    auto hello = std::make_unique<HelloToClient>();
    hello->setSessionId(sessionId);
    hello->capabilities_ = capabilities;

    return hello;
  }

  std::stringstream
  HelloToClient::toXmlStream([[maybe_unused]] const struct ly_ctx *ctx) const {
    std::stringstream xml;
    xml << XmlHeader::toString();

    XmlElement helloElement(HELLO_ELEMENT);
    helloElement.addAttribute(XMLNS_ATTR, NETCONF_NAMESPACE);

    XmlElement capabilitiesElement(CAPABILITIES_ELEMENT);
    for (const auto &capability : capabilities_) {
      XmlElement capabilityElement(CAPABILITY_ELEMENT);
      capabilityElement.setContent(capability);
      capabilitiesElement.addChild(capabilityElement);
    }
    helloElement.addChild(capabilitiesElement);

    XmlElement sessionIdElement(SESSION_ID_ELEMENT);
    sessionIdElement.setContent(std::to_string(sessionId_));
    helloElement.addChild(sessionIdElement);

    xml << helloElement.toString();
    return xml;
  }

  // HelloToServer implementation
  std::unique_ptr<HelloToServer>
  HelloToServer::fromXml([[maybe_unused]] const std::string &xml,
                         [[maybe_unused]] const struct ly_ctx *ctx) {
    throw NotImplementedError("HelloToServer::fromXml not implemented");
  }

  std::unique_ptr<HelloToServer>
  HelloToServer::toXml(lyd_node *lyd_data,
                       [[maybe_unused]] const struct ly_ctx *ctx) {
    auto hello = std::make_unique<HelloToServer>();

    // Generate hello XML using XmlElement
    XmlElement helloElement(HELLO_ELEMENT);
    helloElement.addAttribute(XMLNS_ATTR, NETCONF_NAMESPACE);

    // Add lyd_data as XML content
    if (lyd_data) {
      char *xml_str = nullptr;
      if (lyd_print_mem(&xml_str, lyd_data, LYD_XML, LYD_PRINT_WITHSIBLINGS) ==
          LY_SUCCESS) {
        helloElement.setContent(std::string(xml_str));
        free(xml_str);
      }
    }

    return hello;
  }

  std::unique_ptr<HelloToServer>
  HelloToServer::toXml([[maybe_unused]] uint32_t sessionId,
                       const std::vector<std::string> &capabilities,
                       [[maybe_unused]] const struct ly_ctx *ctx) {
    auto hello = std::make_unique<HelloToServer>();
    hello->capabilities_ = capabilities;

    return hello;
  }

  std::stringstream
  HelloToServer::toXmlStream([[maybe_unused]] const struct ly_ctx *ctx) const {
    std::stringstream xml;
    xml << XmlHeader::toString();

    XmlElement helloElement(HELLO_ELEMENT);
    helloElement.addAttribute(XMLNS_ATTR, NETCONF_NAMESPACE);

    XmlElement capabilitiesElement(CAPABILITIES_ELEMENT);
    for (const auto &capability : capabilities_) {
      XmlElement capabilityElement(CAPABILITY_ELEMENT);
      capabilityElement.setContent(capability);
      capabilitiesElement.addChild(capabilityElement);
    }
    helloElement.addChild(capabilitiesElement);

    xml << helloElement.toString();
    return xml;
  }

} // namespace netd::shared::xml
