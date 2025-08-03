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

#include "table_utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <net/if.h>

void table_init(struct table_format *fmt, const char *title)
{
    memset(fmt, 0, sizeof(*fmt));
    fmt->title = title;
    fmt->column_count = 0;
}

void table_add_column(struct table_format *fmt, const char *header, int min_width)
{
    if (fmt->column_count >= MAX_COLUMNS) return;
    
    fmt->columns[fmt->column_count].header = header;
    fmt->columns[fmt->column_count].min_width = min_width;
    fmt->columns[fmt->column_count].width = min_width;
    fmt->column_count++;
}

void table_update_width(struct table_format *fmt, int col_idx, int content_len)
{
    if (col_idx < 0 || col_idx >= fmt->column_count) return;
    
    if (content_len > fmt->columns[col_idx].width) {
        fmt->columns[col_idx].width = content_len;
    }
}

void table_print_header(const struct table_format *fmt)
{
    if (fmt->title) {
        printf("\n%s:\n", fmt->title);
    }
    
    for (int i = 0; i < fmt->column_count; i++) {
        printf("%-*s", fmt->columns[i].width, fmt->columns[i].header);
        if (i < fmt->column_count - 1) printf(" ");
    }
    printf("\n");
}

void table_print_row(const struct table_format *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    for (int i = 0; i < fmt->column_count; i++) {
        const char *value = va_arg(args, const char*);
        if (!value) value = "";
        
        printf("%-*s", fmt->columns[i].width, value);
        if (i < fmt->column_count - 1) printf(" ");
    }
    printf("\n");
    
    va_end(args);
}

void table_print_row_multiline(const struct table_format *fmt, int num_lines, ...)
{
    va_list args;
    va_start(args, num_lines);
    
    /* For each line */
    for (int line = 0; line < num_lines; line++) {
        /* For each column */
        for (int i = 0; i < fmt->column_count; i++) {
            const char *value = va_arg(args, const char*);
            if (!value) value = "";
            
            printf("%-*s", fmt->columns[i].width, value);
            if (i < fmt->column_count - 1) printf(" ");
        }
        printf("\n");
    }
    
    va_end(args);
}

void table_print_footer(const struct table_format *fmt, const char *footer_text)
{
    (void)fmt;
    if (footer_text) {
        printf("%s\n", footer_text);
    }
}

/**
 * Print a horizontal line for table formatting
 * @param width Total width of the line
 */
void print_separator(int width)
{
    for (int i = 0; i < width; i++) {
        printf("-");
    }
    printf("\n");
}

/**
 * Format interface flags into a readable string
 * @param flags Flag value
 * @param flag_str Buffer to store formatted flags
 * @param max_len Maximum length of flag buffer
 */
void format_interface_flags(int flags, char *flag_str, size_t max_len)
{
    (void)max_len; /* Suppress unused parameter warning */
    flag_str[0] = '\0';
    
    if (flags & IFF_UP) strcat(flag_str, "U");
    if (flags & IFF_BROADCAST) strcat(flag_str, "B");
    if (flags & IFF_RUNNING) strcat(flag_str, "R");
    if (flags & IFF_PROMISC) strcat(flag_str, "P");
    if (flags & IFF_MULTICAST) strcat(flag_str, "M");
    if (flags & IFF_LOOPBACK) strcat(flag_str, "L");
} 