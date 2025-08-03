/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TABLE_UTILS_H
#define TABLE_UTILS_H

#include <stddef.h>

#define MAX_COLUMNS 16

struct table_column {
    const char *header;
    int width;
    int min_width;
};

struct table_format {
    struct table_column columns[MAX_COLUMNS];
    int column_count;
    const char *title;
};

/* Initialize table format with headers and minimum widths */
void table_init(struct table_format *fmt, const char *title);

/* Add a column to the table format */
void table_add_column(struct table_format *fmt, const char *header, int min_width);

/* Update column width based on content length */
void table_update_width(struct table_format *fmt, int col_idx, int content_len);

/* Print table header */
void table_print_header(const struct table_format *fmt);

/* Print table row with proper formatting */
void table_print_row(const struct table_format *fmt, ...);

/* Print table row with multi-line support for multiple values */
void table_print_row_multiline(const struct table_format *fmt, int num_lines, ...);

/* Print table footer */
void table_print_footer(const struct table_format *fmt, const char *footer_text);

/* Display utilities */
void print_separator(int width);
void format_interface_flags(int flags, char *flag_str, size_t max_len);

#endif /* TABLE_UTILS_H */ 