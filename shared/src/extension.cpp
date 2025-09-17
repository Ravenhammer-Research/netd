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

#include <climits>
#include <dirent.h>
#include <dlfcn.h>
#include <filesystem>
#include <shared/include/exception.hpp>
#include <shared/include/extension.hpp>
#include <shared/include/logger.hpp>
#include <sys/stat.h>
#include <unistd.h>

namespace netd::shared {

  std::map<std::string, std::shared_ptr<NetdExtension>>
  NetdExtension::loadExtensions() {
    auto &logger = Logger::getInstance();
    std::map<std::string, std::shared_ptr<NetdExtension>> extensions;

    logger.info("Loading NetD extensions...");

    auto paths = getExtensionPaths();
    for (const auto &path : paths) {
      logger.debug("Searching for extensions in: " + path);

      if (!std::filesystem::exists(path) ||
          !std::filesystem::is_directory(path)) {
        continue;
      }

      try {
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
          if (entry.is_regular_file() && entry.path().extension() == ".so") {
            std::string filepath = entry.path().string();
            logger.debug("Found extension file: " + filepath);

            auto extension = loadExtension(filepath);
            if (extension) {
              auto info = extension->getInfo();
              extensions[info.name] = extension;
              logger.info("Loaded extension: " + info.name + " v" +
                          info.version);
            }
          }
        }
      } catch (const std::filesystem::filesystem_error &e) {
        logger.warning("Error reading extension directory " + path + ": " +
                       e.what());
      }
    }

    logger.info("Loaded " + std::to_string(extensions.size()) + " extensions");
    return extensions;
  }

  std::vector<std::string> NetdExtension::getExtensionPaths() {
    std::vector<std::string> paths;

#ifdef DEBUG_BUILD
    // Check if extensions directory exists relative to current working
    // directory
    struct stat st;
    if (stat("extensions", &st) == 0 && S_ISDIR(st.st_mode)) {
      char realpath_ext[PATH_MAX];
      char realpath_dev[PATH_MAX];
      if (realpath("extensions", realpath_ext) != nullptr &&
          realpath(EXTENSION_DEV_DIR, realpath_dev) != nullptr &&
          strcmp(realpath_ext, realpath_dev) == 0) {
        paths.push_back(EXTENSION_DEV_DIR);
      }
    }
#endif

    // Add production extension directory
    paths.push_back(EXTENSION_DIR);

    return paths;
  }

  std::shared_ptr<NetdExtension>
  NetdExtension::loadExtension(const std::string &filepath) {
    auto &logger = Logger::getInstance();

    // Load the shared library
    void *handle = dlopen(filepath.c_str(), RTLD_LAZY);
    if (!handle) {
      logger.error("Failed to load extension " + filepath + ": " + dlerror());
      return nullptr;
    }

    // Clear any existing error
    dlerror();

    // Get the create function
    typedef NetdExtension *(*CreateExtensionFunc)();
    CreateExtensionFunc create_func =
        (CreateExtensionFunc)dlsym(handle, "createExtension");

    const char *dlsym_error = dlerror();
    if (dlsym_error) {
      logger.error("Failed to find createExtension symbol in " + filepath +
                   ": " + dlsym_error);
      dlclose(handle);
      return nullptr;
    }

    // Create the extension instance
    NetdExtension *extension_ptr = create_func();
    if (!extension_ptr) {
      logger.error("Failed to create extension instance from " + filepath);
      dlclose(handle);
      return nullptr;
    }

    // Check compatibility
    if (!extension_ptr->isCompatible(PROJECT_VERSION)) {
      logger.warning("Extension " + filepath +
                     " is not compatible with NetD version " + PROJECT_VERSION);
      delete extension_ptr;
      dlclose(handle);
      return nullptr;
    }

    // Initialize the extension
    if (!extension_ptr->initialize()) {
      logger.error("Failed to initialize extension from " + filepath);
      delete extension_ptr;
      dlclose(handle);
      return nullptr;
    }

    // Create a shared_ptr with custom deleter to handle dlclose
    auto deleter = [handle](NetdExtension *ext) {
      if (ext) {
        ext->cleanup();
        delete ext;
      }
      dlclose(handle);
    };

    return std::shared_ptr<NetdExtension>(extension_ptr, deleter);
  }

} // namespace netd::shared
