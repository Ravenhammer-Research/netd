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

#include <algorithm>
#include <client/include/tui.hpp>
#include <string>
#include <vector>

namespace netd::client::tui {

  // Wrap text to fit within specified width
  std::vector<std::string> TUI::wrapText(const std::string &text, int width) {
    std::vector<std::string> lines;

    if (text.empty() || width <= 0) {
      return lines;
    }

    // Simple approach: just break at character boundaries if needed
    // This preserves the original text structure better
    std::string current_line;

    for (char c : text) {
      if (c == '\n') {
        // Force line break
        lines.push_back(current_line);
        current_line.clear();
      } else {
        // Add character if it fits
        if (current_line.length() + 1 <= static_cast<size_t>(width)) {
          current_line += c;
        } else {
          // Line is full, start new line with this character
          lines.push_back(current_line);
          current_line = c;
        }
      }
    }

    // Add final line if not empty
    if (!current_line.empty()) {
      lines.push_back(current_line);
    }

    return lines;
  }

  // Wrap text to fit screen width
  std::vector<std::string> TUI::wrapTextToScreen(const std::string &text) {
    int screen_width = getScreenSizeX();
    return wrapText(text, screen_width);
  }

  // Wrap text with indentation
  std::vector<std::string> TUI::wrapTextWithIndent(const std::string &text,
                                                   int width, int indent) {
    std::vector<std::string> lines = wrapText(text, width - indent);

    // Add indentation to all lines except the first
    for (size_t i = 1; i < lines.size(); i++) {
      lines[i] = std::string(indent, ' ') + lines[i];
    }

    return lines;
  }

  // Truncate text to fit width with ellipsis
  std::string TUI::truncateText(const std::string &text, int width) {
    if (text.length() <= static_cast<size_t>(width)) {
      return text;
    }

    if (width <= 3) {
      return std::string(width, '.');
    }

    return text.substr(0, width - 3) + "...";
  }

  // Split text into words
  std::vector<std::string> TUI::splitWords(const std::string &text) {
    std::vector<std::string> words;
    std::string current_word;

    for (char c : text) {
      if (c == ' ' || c == '\t' || c == '\n') {
        if (!current_word.empty()) {
          words.push_back(current_word);
          current_word.clear();
        }
      } else {
        current_word += c;
      }
    }

    if (!current_word.empty()) {
      words.push_back(current_word);
    }

    return words;
  }

  // Get display width of text (accounting for multi-byte characters)
  int TUI::getTextWidth(const std::string &text) {
    // Simple implementation - assumes single-byte characters
    // TODO: Implement proper multi-byte character support
    return static_cast<int>(text.length());
  }

  // Format text with word wrapping for display
  void TUI::putWrappedText(const std::string &text) {
    std::vector<std::string> lines = wrapTextToScreen(text);

    for (const auto &line : lines) {
      putLine(line);
    }
  }

  // Format text with indentation and word wrapping
  void TUI::putIndentedText(const std::string &text, int indent) {
    int screen_width = getScreenSizeX();
    std::vector<std::string> lines =
        wrapTextWithIndent(text, screen_width, indent);

    for (const auto &line : lines) {
      putLine(line);
    }
  }

} // namespace netd::client::tui