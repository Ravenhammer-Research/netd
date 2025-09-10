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

#include <fstream>
#include <libyang/libyang.h>
#include <libyang/parser_data.h>
#include <libyang/parser_schema.h>
#include <libyang/printer_data.h>
#include <libyang/tree_data.h>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>
#include <shared/include/yang.hpp>
#include <sstream>

namespace netd::shared {

  // Singleton implementation
  Yang &Yang::getInstance() {
    static Yang instance;
    return instance;
  }

  // Yang class implementation methods
  Yang::Yang() : ctx_(nullptr) {
    // Initialize libyang context
    ly_ctx_new(nullptr, 0, &ctx_);
    if (!ctx_) {
      auto &logger = Logger::getInstance();
      logger.error("Failed to create libyang context");
      throw netd::shared::ConfigurationError("Failed to create libyang context");
    }

    // Set up search paths for YANG modules
    ly_ctx_set_searchdir(ctx_, YANG_DIR "/standard/ietf/RFC");
    ly_ctx_set_searchdir(ctx_, YANG_DIR "/standard/iana/");
    // ly_ctx_set_searchdir(ctx_, "/usr/local/share/yang/modules/libyang");
    // ly_ctx_set_searchdir(ctx_, "/usr/local/share/yang/modules/libnetconf2");
    // ly_ctx_set_searchdir(ctx_, "/usr/share/yang/modules/libyang");
    // ly_ctx_set_searchdir(ctx_, "/usr/share/yang/modules/libnetconf2");

    // NETCONF modules will be loaded in loadStandardSchemas()

    // Load standard schemas
    loadStandardSchemas();
  }

  Yang::~Yang() {
    if (ctx_) {
      ly_ctx_destroy(ctx_);
    }
  }

  ly_ctx *Yang::getContext() const { return ctx_; }

  bool Yang::loadSchema(const std::string &schemaPath) {
    auto &logger = Logger::getInstance();

    if (!ctx_) {
      logger.error("YANG context not initialized");
      return false;
    }

    // Load schema from file
    struct lys_module *module = nullptr;
    if (lys_parse_path(ctx_, schemaPath.c_str(), LYS_IN_YANG, &module) !=
        LY_SUCCESS) {
      logger.error("Failed to load YANG schema: " + schemaPath);
      return false;
    }

    logger.debug("Loaded YANG schema: " + schemaPath);
    return true;
  }

  bool Yang::loadSchemaByName(const std::string &name,
                              const std::string &revision) {
    auto &logger = Logger::getInstance();

    if (!ctx_) {
      logger.error("YANG context not initialized");
      return false;
    }

    // Search for the schema file in search paths
    char *localfile = nullptr;
    LYS_INFORMAT format;
    const char *const *searchpaths = ly_ctx_get_searchdirs(ctx_);
    if (lys_search_localfile(searchpaths, 0, name.c_str(),
                             revision.empty() ? nullptr : revision.c_str(),
                             &localfile, &format) != LY_SUCCESS ||
        !localfile) {
      logger.error("Failed to find YANG schema: " + name);
      return false;
    }

    // Load the found schema
    struct lys_module *module = nullptr;
    if (lys_parse_path(ctx_, localfile, format, &module) != LY_SUCCESS) {
      logger.error("Failed to load YANG schema: " + std::string(localfile));
      free(localfile);
      return false;
    }

    logger.debug("Loaded YANG schema: " + std::string(localfile));
    free(localfile);
    return true;
  }

  void Yang::loadStandardSchemas() {
    auto &logger = Logger::getInstance();

    // Load standard IETF schemas by name in dependency order
    std::vector<std::pair<std::string, std::string>> standardSchemas = {
        // Basic types first
        {"ietf-inet-types", "2013-07-15"},
        {"ietf-yang-types", "2013-07-15"},
        
        // IETF interfaces (required by iana-if-type)
        {"ietf-interfaces", "2018-02-20"},
        
        // IANA interface types (required for interface type identities)
        {"iana-if-type", "2023-01-26"},

        // IANA modules for TLS and SSH (using actual installed versions)
        {"iana-tls-cipher-suite-algs", "2024-10-16"},
        {"iana-ssh-public-key-algs", "2024-10-16"},
        {"iana-ssh-encryption-algs", "2024-10-16"},
        {"iana-ssh-key-exchange-algs", "2024-10-16"},
        {"iana-ssh-mac-algs", "2024-10-16"},

        // Crypto and keystore modules
        {"ietf-crypto-types", "2024-10-10"},
        {"ietf-keystore", "2024-10-10"},

        // TLS and SSH common modules
        {"ietf-tls-common", "2024-10-10"},
        {"ietf-ssh-common", "2024-10-10"},

        // TCP and transport modules
        {"ietf-tcp-common", "2024-10-10"},
        {"ietf-tcp-server", "2024-10-10"},
        {"ietf-ssh-server", "2024-10-10"},
        {"ietf-tls-server", "2024-10-10"},
        {"ietf-truststore", "2024-10-10"},

        // YANG library and datastores (dependencies for NETCONF)
        {"ietf-yang-library", "2019-01-04"},
        {"ietf-datastores", "2018-02-14"},

        // NETCONF modules (dependencies first)
        {"ietf-netconf-acm", "2018-02-14"},
        {"ietf-netconf", "2011-06-01"},
        {"ietf-netconf-monitoring", "2010-10-04"},
        {"ietf-netconf-partial-lock", "2009-10-19"},
        {"ietf-netconf-time", "2016-01-26"},
        {"ietf-netconf-with-defaults", "2011-06-01"},
        {"ietf-netconf-nmda", "2019-01-07"},

        // RESTCONF modules
        {"ietf-restconf", "2017-01-26"},
        {"ietf-restconf-monitoring", "2017-01-26"},
        {"ietf-restconf-subscribed-notifications", "2019-11-17"},

        {"ietf-ip", "2018-02-22"},
        {"ietf-network-instance", "2019-01-21"},
        {"ietf-network-state", "2018-02-26"},
        {"ietf-network-topology-state", "2018-02-26"},
        {"ietf-network-topology", "2018-02-26"},
        {"ietf-network", "2018-02-26"},
        {"ietf-routing-types", "2017-12-04"},
        {"ietf-routing", "2018-03-13"}};

    for (const auto &schema : standardSchemas) {
      logger.debug("Loading schema: " + schema.first + "@" + schema.second);
      if (!loadSchemaByName(schema.first, schema.second)) {
        logger.warning("Failed to load standard schema: " + schema.first + "@" +
                       schema.second);
      } else {
        logger.debug("Successfully loaded schema: " + schema.first + "@" + schema.second);
      }
    }
  }

  // Static utility functions
  std::string Yang::yangToXml(const lyd_node *node) {
    if (!node) {
      return "";
    }

    char *xml_str = nullptr;
    if (lyd_print_mem(&xml_str, node, LYD_XML, LYD_PRINT_WITHSIBLINGS) !=
        LY_SUCCESS) {
      return "";
    }

    std::string result(xml_str);
    free(xml_str);
    return result;
  }

  std::string Yang::yangToJson(const lyd_node *node) {
    if (!node) {
      return "";
    }

    char *json_str = nullptr;
    if (lyd_print_mem(&json_str, node, LYD_JSON, LYD_PRINT_WITHSIBLINGS) !=
        LY_SUCCESS) {
      return "";
    }

    std::string result(json_str);
    free(json_str);
    return result;
  }

  lyd_node *Yang::xmlToYang(ly_ctx *ctx, const std::string &xml) {
    if (!ctx || xml.empty()) {
      return nullptr;
    }

    lyd_node *node = nullptr;
    if (lyd_parse_data_mem(ctx, xml.c_str(), LYD_XML, LYD_PARSE_STRICT, 0,
                           &node) != LY_SUCCESS) {
      return nullptr;
    }

    return node;
  }

  lyd_node *Yang::jsonToYang(ly_ctx *ctx, const std::string &json) {
    if (!ctx || json.empty()) {
      return nullptr;
    }

    lyd_node *node = nullptr;
    if (lyd_parse_data_mem(ctx, json.c_str(), LYD_JSON, LYD_PARSE_STRICT, 0,
                           &node) != LY_SUCCESS) {
      return nullptr;
    }

    return node;
  }

  const struct lys_module *Yang::getModule(const std::string &name, 
                                           const std::string &revision) const {
    auto &logger = Logger::getInstance();
    
    if (!ctx_) {
      logger.error("YANG context not initialized");
      return nullptr;
    }
    
    // Try to load the module if it's not already loaded
    if (name == "ietf-netconf" && revision == "2011-06-01") {
      logger.debug("Attempting to load ietf-netconf module on demand");
      const_cast<Yang*>(this)->loadSchemaByName(name, revision);
    }

    // Get the module
    const struct lys_module *mod = ly_ctx_get_module(ctx_, name.c_str(), 
                                                     revision.empty() ? nullptr : revision.c_str());
    
    if (!mod) {
      logger.error("Failed to get module: " + name + (revision.empty() ? "" : "@" + revision));
      
      // Debug: List all loaded modules
      logger.debug("Available modules:");
      uint32_t index = 0;
      const struct lys_module *iter = nullptr;
      while ((iter = ly_ctx_get_module_iter(ctx_, &index)) != nullptr) {
        logger.debug("  Module: " + std::string(iter->name) + "@" + (iter->revision ? iter->revision : "no-revision"));
      }
    } else {
      logger.debug("Successfully retrieved module: " + name + "@" + (mod->revision ? mod->revision : "no-revision"));
    }
    
    return mod;
  }

  std::vector<std::string> Yang::getCapabilities() const {
    std::vector<std::string> capabilities;
    auto &logger = Logger::getInstance();
    
    if (!ctx_) {
      logger.error("YANG context not initialized");
      return capabilities;
    }

    // Base capabilities (always present)
    capabilities.push_back("urn:ietf:params:netconf:base:1.0");
    capabilities.push_back("urn:ietf:params:netconf:base:1.1");

    // Check ietf-netconf module features
    const struct lys_module *netconf_mod = ly_ctx_get_module_implemented(ctx_, "ietf-netconf");
    if (netconf_mod) {
      if (lys_feature_value(netconf_mod, "writable-running") == LY_SUCCESS) {
        capabilities.push_back("urn:ietf:params:netconf:capability:writable-running:1.0");
      }
      if (lys_feature_value(netconf_mod, "candidate") == LY_SUCCESS) {
        capabilities.push_back("urn:ietf:params:netconf:capability:candidate:1.0");
        if (lys_feature_value(netconf_mod, "confirmed-commit") == LY_SUCCESS) {
          capabilities.push_back("urn:ietf:params:netconf:capability:confirmed-commit:1.1");
        }
      }
      if (lys_feature_value(netconf_mod, "rollback-on-error") == LY_SUCCESS) {
        capabilities.push_back("urn:ietf:params:netconf:capability:rollback-on-error:1.0");
      }
      if (lys_feature_value(netconf_mod, "validate") == LY_SUCCESS) {
        capabilities.push_back("urn:ietf:params:netconf:capability:validate:1.1");
      }
      if (lys_feature_value(netconf_mod, "startup") == LY_SUCCESS) {
        capabilities.push_back("urn:ietf:params:netconf:capability:startup:1.0");
      }
      if (lys_feature_value(netconf_mod, "xpath") == LY_SUCCESS) {
        capabilities.push_back("urn:ietf:params:netconf:capability:xpath:1.0");
      }
    }

    // Check other module-based capabilities
    const struct lys_module *with_defaults_mod = ly_ctx_get_module_implemented(ctx_, "ietf-netconf-with-defaults");
    if (with_defaults_mod) {
      capabilities.push_back("urn:ietf:params:netconf:capability:with-defaults:1.0");
    }

    const struct lys_module *yanglib_mod = ly_ctx_get_module_implemented(ctx_, "ietf-yang-library");
    if (yanglib_mod) {
      if (yanglib_mod->revision && !strcmp(yanglib_mod->revision, "2019-01-04")) {
        capabilities.push_back("urn:ietf:params:netconf:capability:yang-library:1.1");
      } else {
        capabilities.push_back("urn:ietf:params:netconf:capability:yang-library:1.0");
      }
    }

    const struct lys_module *notifications_mod = ly_ctx_get_module_implemented(ctx_, "ietf-notifications");
    if (notifications_mod) {
      capabilities.push_back("urn:ietf:params:netconf:capability:notification:1.0");
    }

    const struct lys_module *interleave_mod = ly_ctx_get_module_implemented(ctx_, "ietf-interleave");
    if (interleave_mod) {
      capabilities.push_back("urn:ietf:params:netconf:capability:interleave:1.0");
    }

    logger.debug("Generated " + std::to_string(capabilities.size()) + " NETCONF capabilities");
    return capabilities;
  }

} // namespace netd::shared
