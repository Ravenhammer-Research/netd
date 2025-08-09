/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "net.h"
#include <bsdxml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Structure to hold parsing context for find_tag_content */
struct xml_parse_context {
  const char *target_tag;
  char *content;
  size_t max_len;
  int found;
  int in_target;
  int depth;
};

/**
 * Start element handler for Expat
 */
static void start_element(void *userData, const char *name, const char **atts) {
  struct xml_parse_context *ctx = (struct xml_parse_context *)userData;

  /* Count attributes for debugging/logging */
  int attr_count = 0;
  if (atts) {
    while (atts[attr_count] && atts[attr_count + 1])
      attr_count += 2;
  }

  if (strcmp(name, ctx->target_tag) == 0) {
    ctx->in_target = 1;
    ctx->found = 1;
    if (ctx->content) {
      ctx->content[0] = '\0';
    }
  }
  ctx->depth++;
}

/**
 * End element handler for Expat
 */
static void end_element(void *userData, const char *name) {
  struct xml_parse_context *ctx = (struct xml_parse_context *)userData;

  if (strcmp(name, ctx->target_tag) == 0) {
    ctx->in_target = 0;
  }
  ctx->depth--;
}

/**
 * Character data handler for Expat
 */
static void char_data(void *userData, const char *s, int len) {
  struct xml_parse_context *ctx = (struct xml_parse_context *)userData;

  if (ctx->in_target && ctx->content && ctx->max_len > 0) {
    size_t current_len = strlen(ctx->content);
    size_t remaining = ctx->max_len - current_len - 1;

    if (remaining > 0) {
      size_t copy_len = (len < (int)remaining) ? (size_t)len : remaining;
      memcpy(ctx->content + current_len, s, copy_len);
      ctx->content[current_len + copy_len] = '\0';
    }
  }
}

/**
 * Find content between XML tags
 * @param xml XML string to search
 * @param tag_start Start tag to search for
 * @param tag_end End tag to search for
 * @param result Buffer to store the result
 * @param max_len Maximum length of result buffer
 * @return 0 on success, -1 on failure
 */
int find_tag_content(const char *xml, const char *tag_start, const char *tag_end,
                     char *result, int max_len) {
  if (!xml || !tag_start || !tag_end || !result || max_len <= 0) {
    return -1;
  }

  char *start_pos = strstr(xml, tag_start);
  if (!start_pos) {
    return -1;
  }

  start_pos += strlen(tag_start);
  char *end_pos = strstr(start_pos, tag_end);
  if (!end_pos) {
    return -1;
  }

  int content_len = end_pos - start_pos;
  if (content_len >= max_len) {
    content_len = max_len - 1;
  }

  strncpy(result, start_pos, content_len);
  result[content_len] = '\0';

  return 0;
}

/**
 * Extract content from XML tag
 * @param xml XML string to search
 * @param tag Tag name to search for
 * @param result Buffer to store the result
 * @param max_len Maximum length of result buffer
 * @return 0 on success, -1 on failure
 */
int extract_xml_content(const char *xml, const char *tag, char *result,
                        int max_len) {
  if (!xml || !tag || !result || max_len <= 0) {
    return -1;
  }

  char start_tag[128];
  char end_tag[128];

  snprintf(start_tag, sizeof(start_tag), "<%s>", tag);
  snprintf(end_tag, sizeof(end_tag), "</%s>", tag);

  return find_tag_content(xml, start_tag, end_tag, result, max_len);
}

/**
 * Check if XML contains a specific tag
 * @param xml XML string to search
 * @param tag Tag name to search for
 * @return 1 if tag found, 0 if not found
 */
int xml_contains_tag(const char *xml, const char *tag) {
  if (!xml || !tag) {
    return 0;
  }

  char start_tag[128];
  snprintf(start_tag, sizeof(start_tag), "<%s>", tag);
  
  return strstr(xml, start_tag) != NULL;
}

/**
 * Count occurrences of a tag in XML
 * @param xml XML string to search
 * @param tag Tag name to count
 * @return Number of occurrences
 */
int count_xml_tags(const char *xml, const char *tag) {
  if (!xml || !tag) {
    return 0;
  }

  int count = 0;
  const char *pos = xml;
  char start_tag[128];
  snprintf(start_tag, sizeof(start_tag), "<%s>", tag);

  while ((pos = strstr(pos, start_tag)) != NULL) {
    count++;
    pos += strlen(start_tag);
  }

  return count;
}

/**
 * Validate basic XML structure
 * @param xml XML string to validate
 * @return 1 if valid, 0 if invalid
 */
int validate_xml_structure(const char *xml) {
  if (!xml) {
    return 0;
  }

  /* Check for basic XML structure */
  if (strstr(xml, "<?xml") == NULL && strstr(xml, "<") == NULL) {
    return 0;
  }

  /* Check for balanced tags (basic check) */
  int open_count = 0;
  int close_count = 0;
  const char *pos = xml;

  while (*pos) {
    if (*pos == '<') {
      if (*(pos + 1) == '/') {
        close_count++;
      } else if (*(pos + 1) != '!' && *(pos + 1) != '?') {
        open_count++;
      }
    }
    pos++;
  }

  return open_count == close_count;
}

/**
 * Find tag content using Expat XML parsing
 * @param xml XML string
 * @param tag Tag name to find
 * @param content Buffer to store content
 * @param max_len Maximum length of content buffer
 * @return 0 on success, -1 on failure
 */
int find_tag_content_expat(const char *xml, const char *tag, char *content,
                           size_t max_len) {
  XML_Parser parser;
  struct xml_parse_context ctx;
  int result = -1;

  if (!xml || !tag || !content || max_len == 0) {
    return -1;
  }

  /* Initialize context */
  ctx.target_tag = tag;
  ctx.content = content;
  ctx.max_len = max_len;
  ctx.found = 0;
  ctx.in_target = 0;
  ctx.depth = 0;

  /* Initialize content buffer */
  content[0] = '\0';

  /* Create XML parser */
  parser = XML_ParserCreate(NULL);
  if (!parser) {
    return -1;
  }

  /* Set up handlers */
  XML_SetElementHandler(parser, start_element, end_element);
  XML_SetCharacterDataHandler(parser, char_data);
  XML_SetUserData(parser, &ctx);

  /* Parse the XML */
  if (XML_Parse(parser, xml, strlen(xml), 1) == XML_STATUS_OK) {
    if (ctx.found) {
      result = 0;
    }
  }

  /* Clean up */
  XML_ParserFree(parser);

  return result;
}

/**
 * Extract content between XML tags using simple string parsing
 * @param xml XML string to search in
 * @param tag Tag name to extract content from
 * @param buffer Buffer to store the extracted content
 * @param max_len Maximum length of the buffer
 * @return Pointer to buffer on success, NULL on failure
 */
char *extract_xml_content_simple(const char *xml, const char *tag, char *buffer,
                                 size_t max_len) {
  char start_tag[256], end_tag[256];
  const char *start, *end;

  snprintf(start_tag, sizeof(start_tag), "<%s>", tag);
  snprintf(end_tag, sizeof(end_tag), "</%s>", tag);

  start = strstr(xml, start_tag);
  if (!start)
    return NULL;
  start += strlen(start_tag);

  end = strstr(start, end_tag);
  if (!end)
    return NULL;

  size_t len = end - start;
  if (len >= max_len)
    len = max_len - 1;

  strncpy(buffer, start, len);
  buffer[len] = '\0';

  return buffer;
}

/**
 * Extract content between XML tags within a bounded region
 * @param xml XML string to search in
 * @param end_boundary End boundary of search region
 * @param tag Tag name to extract content from
 * @param buffer Buffer to store the extracted content
 * @param max_len Maximum length of the buffer
 * @return Pointer to buffer on success, NULL on failure
 */
char *extract_xml_content_bounded(const char *xml, const char *end_boundary,
                                  const char *tag, char *buffer,
                                  size_t max_len) {
  char start_tag[256], end_tag[256];
  const char *start, *end;

  snprintf(start_tag, sizeof(start_tag), "<%s>", tag);
  snprintf(end_tag, sizeof(end_tag), "</%s>", tag);

  start = strstr(xml, start_tag);
  if (!start || start >= end_boundary)
    return NULL;
  start += strlen(start_tag);

  end = strstr(start, end_tag);
  if (!end || end >= end_boundary)
    return NULL;

  size_t len = end - start;
  if (len >= max_len)
    len = max_len - 1;

  strncpy(buffer, start, len);
  buffer[len] = '\0';

  return buffer;
} 