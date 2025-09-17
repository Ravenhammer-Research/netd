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

#include <cstring>
#include <cxxabi.h>
#include <execinfo.h>
#include <shared/include/exception.hpp>
#include <shared/include/logger.hpp>

namespace netd::shared {

  NetdError::NetdError(const std::string &message)
      : std::runtime_error(message) {
    // Capture stack trace when exception is created
    void *array[20];
    size_t size = backtrace(array, 20);
    stackTrace_.assign(array, array + size);
  }

  std::string
  NetdError::getStackTraceString(const std::vector<void *> &stackTrace) {
    char **strings = backtrace_symbols(const_cast<void **>(stackTrace.data()),
                                       stackTrace.size());

    std::string stackTraceStr = "Stack trace:\n";

    for (size_t i = 0; i < stackTrace.size(); i++) {
      std::string line = strings[i];

      // Try to find and demangle any mangled symbols in the line
      size_t pos = 0;
      while (pos < line.length()) {
        // Look for mangled symbols that start with _Z
        size_t start = line.find("_Z", pos);
        if (start == std::string::npos)
          break;

        // Find the end of the mangled symbol (before + or at end of line)
        size_t end = start + 2; // Skip _Z
        while (end < line.length() && line[end] != '+' && line[end] != ' ' &&
               line[end] != '>') {
          end++;
        }

        if (end > start + 2) {
          std::string mangled = line.substr(start, end - start);

          int status;
          char *demangled =
              abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);

          if (status == 0 && demangled) {
            line = line.substr(0, start) + demangled + line.substr(end);
            free(demangled);
            pos = start + strlen(demangled);
          } else {
            pos = end;
          }
        } else {
          pos = end;
        }
      }

      // Add frame number prefix (reverse order - #0 is most recent)
      char frameStr[32];
      snprintf(frameStr, sizeof(frameStr), "#%zu ", stackTrace.size() - 1 - i);
      stackTraceStr += frameStr + line + "\n";
    }
    free(strings);

    return stackTraceStr;
  }

} // namespace netd::shared
