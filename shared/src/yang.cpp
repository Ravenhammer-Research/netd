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

#include <shared/include/yang.hpp>
#include <shared/include/logger.hpp>
#include <libyang/libyang.h>
#include <libyang/tree_data.h>
#include <libyang/printer_data.h>
#include <libyang/parser_data.h>
#include <libyang/parser_schema.h>
#include <fstream>
#include <sstream>

namespace netd::shared {

	// Singleton implementation
	Yang& Yang::getInstance() {
		static Yang instance;
		return instance;
	}

	// Yang class implementation methods
	Yang::Yang() : ctx_(nullptr) {
		// Initialize libyang context
		ly_ctx_new(nullptr, 0, &ctx_);
		if (!ctx_) {
			auto& logger = Logger::getInstance();
			logger.error("Failed to create libyang context");
			throw std::runtime_error("Failed to create libyang context");
		}
		
		// Set up search paths for YANG modules
		ly_ctx_set_searchdir(ctx_, YANG_DIR "/standard/ietf/RFC");
		ly_ctx_set_searchdir(ctx_, YANG_DIR "/standard/iana/");
		ly_ctx_set_searchdir(ctx_, "/usr/local/share/yang/modules/libyang");
		ly_ctx_set_searchdir(ctx_, "/usr/local/share/yang/modules/libnetconf2");
		ly_ctx_set_searchdir(ctx_, "/usr/share/yang/modules/libyang");
		ly_ctx_set_searchdir(ctx_, "/usr/share/yang/modules/libnetconf2");
		
		// Load standard schemas
		loadStandardSchemas();
	}
	
	Yang::~Yang() {
		if (ctx_) {
			ly_ctx_destroy(ctx_);
		}
	}
	
	ly_ctx* Yang::getContext() const {
		return ctx_;
	}
	
	bool Yang::loadSchema(const std::string& schemaPath) {
		auto& logger = Logger::getInstance();
		
		if (!ctx_) {
			logger.error("YANG context not initialized");
			return false;
		}
		
		// Load schema from file
		struct lys_module* module = nullptr;
		if (lys_parse_path(ctx_, schemaPath.c_str(), LYS_IN_YANG, &module) != LY_SUCCESS) {
			logger.error("Failed to load YANG schema: " + schemaPath);
			return false;
		}
		
		logger.info("Loaded YANG schema: " + schemaPath);
		return true;
	}
	
	bool Yang::loadSchemaByName(const std::string& name, const std::string& revision) {
		auto& logger = Logger::getInstance();
		
		if (!ctx_) {
			logger.error("YANG context not initialized");
			return false;
		}
		
		// Search for the schema file in search paths
		char* localfile = nullptr;
		LYS_INFORMAT format;
		const char* const* searchpaths = ly_ctx_get_searchdirs(ctx_);
		if (lys_search_localfile(searchpaths, 0, name.c_str(), revision.empty() ? nullptr : revision.c_str(), &localfile, &format) != LY_SUCCESS || !localfile) {
			logger.error("Failed to find YANG schema: " + name);
			return false;
		}
		
		// Load the found schema
		struct lys_module* module = nullptr;
		if (lys_parse_path(ctx_, localfile, format, &module) != LY_SUCCESS) {
			logger.error("Failed to load YANG schema: " + std::string(localfile));
			free(localfile);
			return false;
		}
		
		logger.info("Loaded YANG schema: " + std::string(localfile));
		free(localfile);
		return true;
	}

	void Yang::loadStandardSchemas() {
		auto& logger = Logger::getInstance();
		
		// Load standard IETF schemas by name in dependency order
		std::vector<std::pair<std::string, std::string>> standardSchemas = {
			// Basic types first
			{"ietf-inet-types", "2013-07-15"},
			{"ietf-yang-types", "2013-07-15"},
			
			// IANA modules for TLS and SSH (using actual installed versions)
			{"iana-tls-cipher-suite-algs", "2022-06-16"},
			{"iana-ssh-public-key-algs", "2022-06-16"},
			{"iana-ssh-encryption-algs", "2022-06-16"},
			{"iana-ssh-key-exchange-algs", "2022-06-16"},
			{"iana-ssh-mac-algs", "2022-06-16"},
			
			// Crypto and keystore modules
			{"ietf-crypto-types", "2023-12-28"},
			{"ietf-keystore", "2023-12-28"},
			
			// TLS and SSH common modules
			{"ietf-tls-common", "2023-12-28"},
			{"ietf-ssh-common", "2023-12-28"},
			
			// TCP and transport modules
			{"ietf-tcp-common", "2023-12-28"},
			{"ietf-tcp-server", "2023-12-28"},
			{"ietf-ssh-server", "2023-12-28"},
			{"ietf-tls-server", "2023-12-28"},
			
			// NETCONF modules
			{"ietf-netconf", "2013-09-29"},
			{"ietf-netconf-acm", "2018-02-14"},
			{"ietf-netconf-server", "2023-12-28"},
			
			{"ietf-ip", "2018-02-22"},
			{"ietf-network-instance", "2019-01-21"},
			{"ietf-network-state", "2018-02-26"},
			{"ietf-network-topology-state", "2018-02-26"},
			{"ietf-network-topology", "2018-02-26"},
			{"ietf-network", "2018-02-26"},
			{"ietf-routing-types", "2017-12-04"},
			{"ietf-routing", "2018-03-13"}
		};
		
		for (const auto& schema : standardSchemas) {
			if (!loadSchemaByName(schema.first, schema.second)) {
				logger.warning("Failed to load standard schema: " + schema.first + "@" + schema.second);
			}
		}
	}

	// Static utility functions
	std::string Yang::yangToXml(const lyd_node* node) {
		if (!node) {
			return "";
		}
		
		char* xml_str = nullptr;
		if (lyd_print_mem(&xml_str, node, LYD_XML, LYD_PRINT_WITHSIBLINGS) != LY_SUCCESS) {
			return "";
		}
		
		std::string result(xml_str);
		free(xml_str);
		return result;
	}

	std::string Yang::yangToJson(const lyd_node* node) {
		if (!node) {
			return "";
		}
		
		char* json_str = nullptr;
		if (lyd_print_mem(&json_str, node, LYD_JSON, LYD_PRINT_WITHSIBLINGS) != LY_SUCCESS) {
			return "";
		}
		
		std::string result(json_str);
		free(json_str);
		return result;
	}

	lyd_node* Yang::xmlToYang(ly_ctx* ctx, const std::string& xml) {
		if (!ctx || xml.empty()) {
			return nullptr;
		}
		
		lyd_node* node = nullptr;
		if (lyd_parse_data_mem(ctx, xml.c_str(), LYD_XML, LYD_PARSE_STRICT, 0, &node) != LY_SUCCESS) {
			return nullptr;
		}
		
		return node;
	}

	lyd_node* Yang::jsonToYang(ly_ctx* ctx, const std::string& json) {
		if (!ctx || json.empty()) {
			return nullptr;
		}
		
		lyd_node* node = nullptr;
		if (lyd_parse_data_mem(ctx, json.c_str(), LYD_JSON, LYD_PARSE_STRICT, 0, &node) != LY_SUCCESS) {
			return nullptr;
		}
		
		return node;
	}

} // namespace netd::shared
