#ifndef NETD_XML_HELLO_HPP
#define NETD_XML_HELLO_HPP

#include <string>
#include <vector>
#include <memory>
#include <libyang/libyang.h>
#include <bsdxml.h>
#include <shared/include/xml/base.hpp>

namespace netd::shared::xml {

  // Hello-specific XML constants
  static constexpr const char* HELLO_ELEMENT = "hello";
  static constexpr const char* CAPABILITIES_ELEMENT = "capabilities";
  static constexpr const char* CAPABILITY_ELEMENT = "capability";
  static constexpr const char* SESSION_ID_ELEMENT = "session-id";

  // Base Hello class implementing XMLTree
  class Hello : public XMLTree {
  public:
    Hello() = default;
    virtual ~Hello() = default;

    // Parse XML string into Hello object
    static std::unique_ptr<Hello> fromXml(const std::string& xml, const struct ly_ctx* ctx);

    // Override XMLTree methods
    std::stringstream toXmlStream(const struct ly_ctx* ctx) const override;

  protected:
    // Override XMLTree protected methods
    void startElementHandler(void* userData, const XML_Char* name, const XML_Char** atts) override;
    void endElementHandler(void* userData, const XML_Char* name) override;
    void characterDataHandler(void* userData, const XML_Char* s, int len) override;

    std::vector<std::string> capabilities_;
  };

  // Server-to-client Hello message (includes session-id)
  class HelloToClient : public Hello {
  public:
    HelloToClient() = default;
    virtual ~HelloToClient() = default;

    // Generate HelloToClient from parameters
    static std::unique_ptr<HelloToClient> toXml(uint32_t sessionId, const std::vector<std::string>& capabilities, const struct ly_ctx* ctx);

    // Override XMLTree methods
    std::stringstream toXmlStream(const struct ly_ctx* ctx) const override;

    // Getters and setters
    uint32_t getSessionId() const { return sessionId_; }
    void setSessionId(uint32_t id) { sessionId_ = id; }
    const std::vector<std::string>& getCapabilities() const { return capabilities_; }
    void setCapabilities(const std::vector<std::string>& caps) { capabilities_ = caps; }

  private:
    uint32_t sessionId_ = 0;
    std::vector<std::string> capabilities_;
  };

  // Client-to-server Hello message (no session-id)
  class HelloToServer : public Hello {
  public:
    HelloToServer() = default;
    virtual ~HelloToServer() = default;

    // Parse XML string into HelloToServer object
    static std::unique_ptr<HelloToServer> fromXml(const std::string& xml, const struct ly_ctx* ctx);

    // Generate HelloToServer from parameters
    static std::unique_ptr<HelloToServer> toXml(lyd_node* lyd_data, const struct ly_ctx* ctx);
    static std::unique_ptr<HelloToServer> toXml(uint32_t sessionId, const std::vector<std::string>& capabilities, const struct ly_ctx* ctx);

    // Override XMLTree methods
    std::stringstream toXmlStream(const struct ly_ctx* ctx) const override;
    
    // Getters and setters
    const std::vector<std::string>& getCapabilities() const { return capabilities_; }
    void setCapabilities(const std::vector<std::string>& caps) { capabilities_ = caps; }
  };

} // namespace netd::shared::xml

#endif // NETD_XML_HELLO_HPP
