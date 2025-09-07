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

#include <shared/include/pretty.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <regex.h>
#include <cstring>

namespace netd::shared {

  // ANSI color codes
  const std::string Pretty::RESET = "\033[0m";
  const std::string Pretty::BOLD = "\033[1m";
  const std::string Pretty::DIM = "\033[2m";
  
  // 24-bit RGB colors
  const std::string Pretty::RED = "\033[38;2;220;50;47m";        // #DC322F
  const std::string Pretty::GREEN = "\033[38;2;133;153;0m";      // #859900
  const std::string Pretty::YELLOW = "\033[38;2;181;137;0m";     // #B58900
  const std::string Pretty::BLUE = "\033[38;2;38;139;210m";      // #268BD2
  const std::string Pretty::MAGENTA = "\033[38;2;211;54;130m";   // #D33682
  const std::string Pretty::CYAN = "\033[38;2;42;161;152m";      // #2AA198
  const std::string Pretty::WHITE = "\033[38;2;253;246;227m";    // #FDF6E3
  const std::string Pretty::GRAY = "\033[38;2;93;93;93m";        // #5D5D5D
  
  // 24-bit RGB background colors
  const std::string Pretty::BG_RED = "\033[48;2;220;50;47m";
  const std::string Pretty::BG_GREEN = "\033[48;2;133;153;0m";
  const std::string Pretty::BG_YELLOW = "\033[48;2;181;137;0m";
  const std::string Pretty::BG_BLUE = "\033[48;2;38;139;210m";
  const std::string Pretty::BG_MAGENTA = "\033[48;2;211;54;130m";
  const std::string Pretty::BG_CYAN = "\033[48;2;42;161;152m";
  const std::string Pretty::BG_WHITE = "\033[48;2;253;246;227m";
  const std::string Pretty::BG_GRAY = "\033[48;2;93;93;93m";

  std::string Pretty::colorize(const std::string &text, const std::string &color) {
    return color + text + RESET;
  }

  std::string Pretty::bold(const std::string &text) {
    return colorize(text, BOLD);
  }

  std::string Pretty::dim(const std::string &text) {
    return colorize(text, DIM);
  }

  std::string Pretty::red(const std::string &text) {
    return colorize(text, RED);
  }

  std::string Pretty::green(const std::string &text) {
    return colorize(text, GREEN);
  }

  std::string Pretty::yellow(const std::string &text) {
    return colorize(text, YELLOW);
  }

  std::string Pretty::blue(const std::string &text) {
    return colorize(text, BLUE);
  }

  std::string Pretty::magenta(const std::string &text) {
    return colorize(text, MAGENTA);
  }

  std::string Pretty::cyan(const std::string &text) {
    return colorize(text, CYAN);
  }

  std::string Pretty::white(const std::string &text) {
    return colorize(text, WHITE);
  }

  std::string Pretty::gray(const std::string &text) {
    return colorize(text, GRAY);
  }

  std::string Pretty::bgRed(const std::string &text) {
    return colorize(text, BG_RED);
  }

  std::string Pretty::bgGreen(const std::string &text) {
    return colorize(text, BG_GREEN);
  }

  std::string Pretty::bgYellow(const std::string &text) {
    return colorize(text, BG_YELLOW);
  }

  std::string Pretty::bgBlue(const std::string &text) {
    return colorize(text, BG_BLUE);
  }

  std::string Pretty::bgMagenta(const std::string &text) {
    return colorize(text, BG_MAGENTA);
  }

  std::string Pretty::bgCyan(const std::string &text) {
    return colorize(text, BG_CYAN);
  }

  std::string Pretty::bgWhite(const std::string &text) {
    return colorize(text, BG_WHITE);
  }

  std::string Pretty::bgGray(const std::string &text) {
    return colorize(text, BG_GRAY);
  }

  std::string Pretty::table(const std::vector<std::vector<std::string>> &rows, 
                           const std::vector<std::string> &headers) {
    if (rows.empty()) {
      return "";
    }

    // Calculate column widths
    std::vector<int> colWidths;
    if (!headers.empty()) {
      colWidths.resize(headers.size());
      for (size_t i = 0; i < headers.size(); ++i) {
        colWidths[i] = static_cast<int>(headers[i].length());
      }
    } else {
      colWidths.resize(rows[0].size());
    }

    for (const auto &row : rows) {
      for (size_t i = 0; i < row.size() && i < colWidths.size(); ++i) {
        colWidths[i] = std::max(colWidths[i], static_cast<int>(row[i].length()));
      }
    }

    std::ostringstream result;
    
    // Print headers if provided
    if (!headers.empty()) {
      result << "┌";
      for (size_t i = 0; i < colWidths.size(); ++i) {
        result << repeat("─", colWidths[i] + 2);
        if (i < colWidths.size() - 1) {
          result << "┬";
        }
      }
      result << "┐\n";
      
      result << "│";
      for (size_t i = 0; i < headers.size() && i < colWidths.size(); ++i) {
        result << " " << left(headers[i], colWidths[i]) << " │";
      }
      result << "\n";
      
      result << "├";
      for (size_t i = 0; i < colWidths.size(); ++i) {
        result << repeat("─", colWidths[i] + 2);
        if (i < colWidths.size() - 1) {
          result << "┼";
        }
      }
      result << "┤\n";
    }

    // Print rows
    for (const auto &row : rows) {
      result << "│";
      for (size_t i = 0; i < row.size() && i < colWidths.size(); ++i) {
        result << " " << left(row[i], colWidths[i]) << " │";
      }
      result << "\n";
    }

    // Print bottom border
    result << "└";
    for (size_t i = 0; i < colWidths.size(); ++i) {
      result << repeat("─", colWidths[i] + 2);
      if (i < colWidths.size() - 1) {
        result << "┴";
      }
    }
    result << "┘";

    return result.str();
  }

  std::string Pretty::box(const std::string &text, const std::string &title) {
    std::vector<std::string> lines;
    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
      lines.push_back(line);
    }

    if (lines.empty()) {
      return "";
    }

    int maxWidth = 0;
    for (const auto &l : lines) {
      maxWidth = std::max(maxWidth, static_cast<int>(l.length()));
    }
    if (!title.empty()) {
      maxWidth = std::max(maxWidth, static_cast<int>(title.length() + 2));
    }

    std::ostringstream result;
    
    // Top border
    result << "┌";
    if (!title.empty()) {
      result << " " << title << " ";
      int remaining = maxWidth - static_cast<int>(title.length());
      if (remaining > 0) {
        result << repeat("─", remaining);
      }
    } else {
      result << repeat("─", maxWidth + 2);
    }
    result << "┐\n";

    // Content
    for (const auto &l : lines) {
      result << "│ " << left(l, maxWidth) << " │\n";
    }

    // Bottom border
    result << "└" << repeat("─", maxWidth + 2) << "┘";

    return result.str();
  }

  std::string Pretty::horizontalLine(int width, const std::string &ch) {
    return repeat(ch, width);
  }

  std::string Pretty::verticalLine(int height, const std::string &ch) {
    std::ostringstream result;
    for (int i = 0; i < height; ++i) {
      result << ch << "\n";
    }
    return result.str();
  }

  std::string Pretty::progressBar(int current, int total, int width, 
                                 const std::string &fill, const std::string &empty) {
    if (total <= 0) {
      return repeat(empty, width);
    }

    int filled = (current * width) / total;
    int remaining = width - filled;

    return repeat(fill, filled) + repeat(empty, remaining);
  }

  std::string Pretty::center(const std::string &text, int width) {
    if (static_cast<int>(text.length()) >= width) {
      return text.substr(0, width);
    }

    int padding = (width - static_cast<int>(text.length())) / 2;
    return repeat(" ", padding) + text + repeat(" ", width - static_cast<int>(text.length()) - padding);
  }

  std::string Pretty::left(const std::string &text, int width) {
    if (static_cast<int>(text.length()) >= width) {
      return text.substr(0, width);
    }
    return text + repeat(" ", width - static_cast<int>(text.length()));
  }

  std::string Pretty::right(const std::string &text, int width) {
    if (static_cast<int>(text.length()) >= width) {
      return text.substr(0, width);
    }
    return repeat(" ", width - static_cast<int>(text.length())) + text;
  }

  std::vector<std::string> Pretty::wrap(const std::string &text, int width) {
    std::vector<std::string> lines;
    std::istringstream iss(text);
    std::string word;
    std::string currentLine;

    while (iss >> word) {
      if (currentLine.empty()) {
        currentLine = word;
      } else if (static_cast<int>(currentLine.length() + 1 + word.length()) <= width) {
        currentLine += " " + word;
      } else {
        lines.push_back(currentLine);
        currentLine = word;
      }
    }

    if (!currentLine.empty()) {
      lines.push_back(currentLine);
    }

    return lines;
  }

  std::string Pretty::wrapToString(const std::string &text, int width) {
    auto lines = wrap(text, width);
    std::ostringstream result;
    for (size_t i = 0; i < lines.size(); ++i) {
      result << lines[i];
      if (i < lines.size() - 1) {
        result << "\n";
      }
    }
    return result.str();
  }

  std::string Pretty::repeat(const std::string &str, int count) {
    std::ostringstream result;
    for (int i = 0; i < count; ++i) {
      result << str;
    }
    return result.str();
  }

  std::string Pretty::pad(const std::string &text, int width, char padChar) {
    if (static_cast<int>(text.length()) >= width) {
      return text;
    }
    return text + repeat(std::string(1, padChar), width - static_cast<int>(text.length()));
  }

  std::string Pretty::truncate(const std::string &text, int maxLength, const std::string &suffix) {
    if (static_cast<int>(text.length()) <= maxLength) {
      return text;
    }
    return text.substr(0, maxLength - static_cast<int>(suffix.length())) + suffix;
  }

  std::string Pretty::rgb(int r, int g, int b) {
    std::ostringstream oss;
    oss << "\033[38;2;" << r << ";" << g << ";" << b << "m";
    return oss.str();
  }

  std::string Pretty::bgRgb(int r, int g, int b) {
    std::ostringstream oss;
    oss << "\033[48;2;" << r << ";" << g << ";" << b << "m";
    return oss.str();
  }

  std::string Pretty::hex(const std::string &hexColor) {
    if (hexColor.length() != 7 || hexColor[0] != '#') {
      return "";
    }
    
    int r = std::stoi(hexColor.substr(1, 2), nullptr, 16);
    int g = std::stoi(hexColor.substr(3, 2), nullptr, 16);
    int b = std::stoi(hexColor.substr(5, 2), nullptr, 16);
    
    return rgb(r, g, b);
  }

  std::string Pretty::bgHex(const std::string &hexColor) {
    if (hexColor.length() != 7 || hexColor[0] != '#') {
      return "";
    }
    
    int r = std::stoi(hexColor.substr(1, 2), nullptr, 16);
    int g = std::stoi(hexColor.substr(3, 2), nullptr, 16);
    int b = std::stoi(hexColor.substr(5, 2), nullptr, 16);
    
    return bgRgb(r, g, b);
  }

  std::vector<std::string> Pretty::extractFilenames(const std::string &text) {
    std::vector<std::string> filenames;
    regex_t regex;
    const char *pattern = R"(([a-zA-Z0-9_/\.-]+\.(cpp|c|hpp|h|cc|cxx|hxx))(?:\s|$))";
    
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
      return filenames;
    }
    
    regmatch_t match;
    const char *text_cstr = text.c_str();
    size_t offset = 0;
    
    while (regexec(&regex, text_cstr + offset, 1, &match, 0) == 0) {
      if (match.rm_so != -1) {
        std::string filename(text_cstr + offset + match.rm_so, match.rm_eo - match.rm_so);
        filenames.push_back(filename);
        offset += match.rm_eo;
      } else {
        break;
      }
    }
    
    regfree(&regex);
    return filenames;
  }

  std::vector<std::string> Pretty::extractFunctionNames(const std::string &text) {
    std::vector<std::string> functions;
    regex_t regex;
    const char *pattern = R"(([a-zA-Z_][a-zA-Z0-9_]*::[a-zA-Z_][a-zA-Z0-9_]*|[a-zA-Z_][a-zA-Z0-9_]*)\s*\()";
    
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
      return functions;
    }
    
    regmatch_t match;
    const char *text_cstr = text.c_str();
    size_t offset = 0;
    
    while (regexec(&regex, text_cstr + offset, 1, &match, 0) == 0) {
      if (match.rm_so != -1) {
        std::string function(text_cstr + offset + match.rm_so, match.rm_eo - match.rm_so - 1); // Remove the '('
        functions.push_back(function);
        offset += match.rm_eo;
      } else {
        break;
      }
    }
    
    regfree(&regex);
    return functions;
  }

  std::string Pretty::colorizeStackTrace(const std::string &stackTrace) {
    std::string result = stackTrace;
    
    // Colorize frame numbers (#0, #1, etc.)
    regex_t regex;
    if (regcomp(&regex, "#[0-9]+", REG_EXTENDED) == 0) {
      regmatch_t match;
      size_t offset = 0;
      while (regexec(&regex, result.c_str() + offset, 1, &match, 0) == 0) {
        size_t start = offset + match.rm_so;
        size_t length = match.rm_eo - match.rm_so;
        std::string frameNum = result.substr(start, length);
        std::string colored = bold(yellow(frameNum));
        result.replace(start, length, colored);
        offset = start + colored.length();
      }
      regfree(&regex);
    }
    
    // Colorize memory addresses (0x...)
    if (regcomp(&regex, "0x[0-9a-fA-F]+", REG_EXTENDED) == 0) {
      regmatch_t match;
      size_t offset = 0;
      while (regexec(&regex, result.c_str() + offset, 1, &match, 0) == 0) {
        size_t start = offset + match.rm_so;
        size_t length = match.rm_eo - match.rm_so;
        std::string address = result.substr(start, length);
        std::string colored = dim(gray(address));
        result.replace(start, length, colored);
        offset = start + colored.length();
      }
      regfree(&regex);
    }
    
    // Colorize function names (demangled C++ names)
    if (regcomp(&regex, "<[^>]+>", REG_EXTENDED) == 0) {
      regmatch_t match;
      size_t offset = 0;
      while (regexec(&regex, result.c_str() + offset, 1, &match, 0) == 0) {
        size_t start = offset + match.rm_so;
        size_t length = match.rm_eo - match.rm_so;
        std::string func = result.substr(start, length);
        std::string colored = bold(cyan(func));
        result.replace(start, length, colored);
        offset = start + colored.length();
      }
      regfree(&regex);
    }
    
    // Colorize file paths
    if (regcomp(&regex, "/[^\\s]+", REG_EXTENDED) == 0) {
      regmatch_t match;
      size_t offset = 0;
      while (regexec(&regex, result.c_str() + offset, 1, &match, 0) == 0) {
        size_t start = offset + match.rm_so;
        size_t length = match.rm_eo - match.rm_so;
        std::string path = result.substr(start, length);
        std::string colored = green(path);
        result.replace(start, length, colored);
        offset = start + colored.length();
      }
      regfree(&regex);
    }
    
    // Colorize line numbers
    if (regcomp(&regex, ":[0-9]+", REG_EXTENDED) == 0) {
      regmatch_t match;
      size_t offset = 0;
      while (regexec(&regex, result.c_str() + offset, 1, &match, 0) == 0) {
        size_t start = offset + match.rm_so;
        size_t length = match.rm_eo - match.rm_so;
        std::string lineNum = result.substr(start, length);
        std::string colored = bold(blue(lineNum));
        result.replace(start, length, colored);
        offset = start + colored.length();
      }
      regfree(&regex);
    }
    
    return result;
  }

} // namespace netd::shared
