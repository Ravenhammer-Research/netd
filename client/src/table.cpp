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
#include <client/include/table.hpp>
#include <iomanip>
#include <sstream>
#include <string>

namespace netd::client {

  constexpr size_t COLUMN_SPACING = 0x02;
  constexpr char SEPARATOR_CHAR = 0x2D;
  constexpr char NEWLINE_CHAR = 0x0A;
  constexpr char SPACE_CHAR = 0x20;
  constexpr const char *EMPTY = "";

  Table::Table() {}

  void Table::addColumn(const std::string &name) { columns_.push_back(name); }

  void Table::addRow(const std::vector<std::string> &values) {
    rows_.push_back(values);
  }

  void Table::clear() {
    columns_.clear();
    rows_.clear();
  }

  std::string Table::format() const {
    if (columns_.empty()) {
      return EMPTY;
    }

    std::vector<size_t> widths = calculateColumnWidths();
    std::ostringstream oss;

    // Format header
    oss << formatRow(columns_, widths) << NEWLINE_CHAR;
    oss << formatSeparator(widths) << NEWLINE_CHAR;

    // Format rows
    for (const auto &row : rows_) {
      oss << formatRow(row, widths) << NEWLINE_CHAR;
    }

    return oss.str();
  }

  std::vector<size_t> Table::calculateColumnWidths() const {
    std::vector<size_t> widths(columns_.size(), 0);

    // Calculate width for each column
    for (size_t i = 0; i < columns_.size(); ++i) {
      // Start with column header width
      widths[i] = columns_[i].length();

      // Check all rows for this column
      for (const auto &row : rows_) {
        if (i < row.size()) {
          widths[i] = std::max(widths[i], row[i].length());
        }
      }
    }

    return widths;
  }

  std::string Table::formatRow(const std::vector<std::string> &values,
                               const std::vector<size_t> &widths) const {
    std::ostringstream oss;

    for (size_t i = 0; i < columns_.size(); ++i) {
      std::string value = (i < values.size()) ? values[i] : "";
      oss << std::left << std::setw(widths[i]) << value;

      if (i < columns_.size() - 1) {
        oss << std::string(COLUMN_SPACING, SPACE_CHAR);
      }
    }

    return oss.str();
  }

  std::string Table::formatSeparator(const std::vector<size_t> &widths) const {
    std::ostringstream oss;

    for (size_t i = 0; i < widths.size(); ++i) {
      oss << std::string(widths[i], SEPARATOR_CHAR);

      if (i < widths.size() - 1) {
        oss << std::string(COLUMN_SPACING, SPACE_CHAR);
      }
    }

    return oss.str();
  }

} // namespace netd::client