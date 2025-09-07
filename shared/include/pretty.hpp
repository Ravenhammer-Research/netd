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

#ifndef NETD_PRETTY_HPP
#define NETD_PRETTY_HPP

#include <string>
#include <vector>
#include <sstream>

namespace netd::shared {

  class Pretty {
  public:
    // ANSI color codes
    static const std::string RESET;
    static const std::string BOLD;
    static const std::string DIM;
    static const std::string RED;
    static const std::string GREEN;
    static const std::string YELLOW;
    static const std::string BLUE;
    static const std::string MAGENTA;
    static const std::string CYAN;
    static const std::string WHITE;
    static const std::string GRAY;
    
    // Background colors
    static const std::string BG_RED;
    static const std::string BG_GREEN;
    static const std::string BG_YELLOW;
    static const std::string BG_BLUE;
    static const std::string BG_MAGENTA;
    static const std::string BG_CYAN;
    static const std::string BG_WHITE;
    static const std::string BG_GRAY;
    
    // Text formatting functions
    static std::string colorize(const std::string &text, const std::string &color);
    static std::string bold(const std::string &text);
    static std::string dim(const std::string &text);
    static std::string red(const std::string &text);
    static std::string green(const std::string &text);
    static std::string yellow(const std::string &text);
    static std::string blue(const std::string &text);
    static std::string magenta(const std::string &text);
    static std::string cyan(const std::string &text);
    static std::string white(const std::string &text);
    static std::string gray(const std::string &text);
    
    // Background formatting functions
    static std::string bgRed(const std::string &text);
    static std::string bgGreen(const std::string &text);
    static std::string bgYellow(const std::string &text);
    static std::string bgBlue(const std::string &text);
    static std::string bgMagenta(const std::string &text);
    static std::string bgCyan(const std::string &text);
    static std::string bgWhite(const std::string &text);
    static std::string bgGray(const std::string &text);
    
    // Table formatting
    static std::string table(const std::vector<std::vector<std::string>> &rows, 
                           const std::vector<std::string> &headers = {});
    
    // Box drawing
    static std::string box(const std::string &text, const std::string &title = "");
    static std::string horizontalLine(int width, const std::string &ch = "─");
    static std::string verticalLine(int height, const std::string &ch = "│");
    
    // Progress bars
    static std::string progressBar(int current, int total, int width = 50, 
                                 const std::string &fill = "█", const std::string &empty = "░");
    
    // Text alignment
    static std::string center(const std::string &text, int width);
    static std::string left(const std::string &text, int width);
    static std::string right(const std::string &text, int width);
    
    // Text wrapping
    static std::vector<std::string> wrap(const std::string &text, int width);
    static std::string wrapToString(const std::string &text, int width);
    
    // Utility functions
    static std::string repeat(const std::string &str, int count);
    static std::string pad(const std::string &text, int width, char padChar = ' ');
    static std::string truncate(const std::string &text, int maxLength, const std::string &suffix = "...");
    
    // String formatting and interpolation
    template<typename... Args>
    static std::string fmt(const std::string &format, Args&&... args) {
      std::ostringstream oss;
      formatString(oss, format, std::forward<Args>(args)...);
      return oss.str();
    }

    // Regex utilities for parsing
    static std::vector<std::string> extractFilenames(const std::string &text);
    static std::vector<std::string> extractFunctionNames(const std::string &text);
    static std::string colorizeStackTrace(const std::string &stackTrace);
    
  private:
    // Helper function for string formatting
    template<typename T>
    static void formatString(std::ostringstream &oss, const std::string &format, T&& value) {
      size_t pos = 0;
      while (pos < format.length()) {
        size_t next = format.find("{}", pos);
        if (next == std::string::npos) {
          oss << format.substr(pos);
          break;
        }
        oss << format.substr(pos, next - pos);
        oss << value;
        pos = next + 2;
      }
    }
    
    template<typename T, typename... Args>
    static void formatString(std::ostringstream &oss, const std::string &format, T&& value, Args&&... args) {
      size_t pos = 0;
      size_t next = format.find("{}", pos);
      if (next == std::string::npos) {
        oss << format;
        return;
      }
      oss << format.substr(pos, next - pos);
      oss << value;
      pos = next + 2;
      formatString(oss, format.substr(pos), std::forward<Args>(args)...);
    }
    
    // 24-bit RGB colors
    static std::string rgb(int r, int g, int b);
    static std::string bgRgb(int r, int g, int b);
    static std::string hex(const std::string &hexColor);
    static std::string bgHex(const std::string &hexColor);
  };

} // namespace netd::shared

#endif // NETD_PRETTY_HPP
