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

#ifndef NETD_SHARED_EXTENSION_HPP
#define NETD_SHARED_EXTENSION_HPP

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace netd::shared {

  /**
   * Extension capabilities that can be provided by loadable modules
   */
  enum class ExtensionCapability {
    NATIVE_BACKEND // Native backend implementation for OS-specific
                   // functionality
  };

  /**
   * Extension metadata
   */
  struct ExtensionInfo {
    std::string name;
    std::string version;
    std::string description;
    std::vector<ExtensionCapability> capabilities;
    std::string author;
    std::string license;
  };

  /**
   * Base class for NetD extensions
   */
  class NetdExtension {
  public:
    virtual ~NetdExtension() = default;

    /**
     * Get the capabilities provided by this extension
     * @return Vector of capabilities this extension provides
     */
    virtual std::vector<ExtensionCapability> getCapabilities() const = 0;

    /**
     * Get extension metadata
     * @return Extension information
     */
    virtual ExtensionInfo getInfo() const = 0;

    /**
     * Initialize the extension
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize() = 0;

    /**
     * Cleanup the extension
     */
    virtual void cleanup() = 0;

    /**
     * Check if extension is compatible with current NetD version
     * @param netd_version Current NetD version
     * @return true if compatible, false otherwise
     */
    virtual bool isCompatible(const std::string &netd_version) const = 0;

    /**
     * Load extensions from the extension directory
     * @return Map of loaded extensions by name
     */
    static std::map<std::string, std::shared_ptr<NetdExtension>>
    loadExtensions();

    /**
     * Get the extension search path
     * @return Vector of paths to search for extensions
     */
    static std::vector<std::string> getExtensionPaths();

  private:
    /**
     * Load a single extension from a file
     * @param filepath Path to the extension file
     * @return Shared pointer to loaded extension, or nullptr if failed
     */
    static std::shared_ptr<NetdExtension>
    loadExtension(const std::string &filepath);
  };

} // namespace netd::shared

#endif // NETD_SHARED_EXTENSION_HPP
