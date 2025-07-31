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

#include "net.h"
#include <string.h>
#include <stdio.h>

/**
 * Simple XML parsing helper - find tag content
 * @param xml XML string
 * @param tag Tag name to find
 * @param content Buffer to store content
 * @param max_len Maximum length of content buffer
 * @return 0 on success, -1 on failure
 */
int find_tag_content(const char *xml, const char *tag, char *content, size_t max_len)
{
    char start_tag[256];
    char end_tag[256];
    const char *start, *end;
    
    snprintf(start_tag, sizeof(start_tag), "<%s", tag);
    snprintf(end_tag, sizeof(end_tag), "</%s>", tag);
    
    start = strstr(xml, start_tag);
    if (!start) {
        return -1;
    }
    
    /* Find the closing '>' of the start tag */
    start = strchr(start, '>');
    if (!start) {
        return -1;
    }
    start++; /* Skip the '>' */
    
    end = strstr(start, end_tag);
    if (!end) {
        return -1;
    }
    
    size_t len = end - start;
    if (len >= max_len) {
        len = max_len - 1;
    }
    
    strncpy(content, start, len);
    content[len] = '\0';
    
    return 0;
} 