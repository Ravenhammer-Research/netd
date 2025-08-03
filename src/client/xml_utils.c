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
#include "if_table.h"
#include <bsdxml.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Structure to hold parsing context */
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
static void start_element(void *userData, const char *name, const char **atts)
{
    struct xml_parse_context *ctx = (struct xml_parse_context *)userData;
    
    /* Count attributes for debugging/logging */
    int attr_count = 0;
    if (atts) {
        while (atts[attr_count] && atts[attr_count+1]) attr_count += 2;
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
static void end_element(void *userData, const char *name)
{
    struct xml_parse_context *ctx = (struct xml_parse_context *)userData;
    
    if (strcmp(name, ctx->target_tag) == 0) {
        ctx->in_target = 0;
    }
    ctx->depth--;
}

/**
 * Character data handler for Expat
 */
static void char_data(void *userData, const char *s, int len)
{
    struct xml_parse_context *ctx = (struct xml_parse_context *)userData;
    
    if (ctx->in_target && ctx->content && ctx->max_len > 0) {
        size_t current_len = strlen(ctx->content);
        size_t remaining = ctx->max_len - current_len - 1;
        
        if (remaining > 0) {
            size_t copy_len = (len < (int)remaining) ? (size_t)len : remaining;
            strncat(ctx->content, s, copy_len);
        }
    }
}

/**
 * Find tag content using Expat XML parsing
 * @param xml XML string
 * @param tag Tag name to find
 * @param content Buffer to store content
 * @param max_len Maximum length of content buffer
 * @return 0 on success, -1 on failure
 */
int find_tag_content(const char *xml, const char *tag, char *content, size_t max_len)
{
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
char* extract_xml_content(const char *xml, const char *tag, char *buffer, size_t max_len)
{
    char start_tag[256], end_tag[256];
    const char *start, *end;
    
    snprintf(start_tag, sizeof(start_tag), "<%s>", tag);
    snprintf(end_tag, sizeof(end_tag), "</%s>", tag);
    
    start = strstr(xml, start_tag);
    if (!start) return NULL;
    start += strlen(start_tag);
    
    end = strstr(start, end_tag);
    if (!end) return NULL;
    
    size_t len = end - start;
    if (len >= max_len) len = max_len - 1;
    
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
char* extract_xml_content_bounded(const char *xml, const char *end_boundary, const char *tag, char *buffer, size_t max_len)
{
    char start_tag[256], end_tag[256];
    const char *start, *end;
    
    snprintf(start_tag, sizeof(start_tag), "<%s>", tag);
    snprintf(end_tag, sizeof(end_tag), "</%s>", tag);
    
    start = strstr(xml, start_tag);
    if (!start || start >= end_boundary) return NULL;
    start += strlen(start_tag);
    
    end = strstr(start, end_tag);
    if (!end || end >= end_boundary) return NULL;
    
    size_t len = end - start;
    if (len >= max_len) len = max_len - 1;
    
    strncpy(buffer, start, len);
    buffer[len] = '\0';
    
    return buffer;
}

/* Structure to hold interface parsing context */
struct interface_parse_context {
    struct interface_data *interfaces;
    int max_interfaces;
    int interface_count;
    struct interface_data *current_interface;
    int in_interface;
    int in_group;
    int in_alias;
    char current_tag[64];
    char temp_content[256];
};

/**
 * Start element handler for interface parsing
 */
static void interface_start_element(void *userData, const char *name, const char **atts)
{
    struct interface_parse_context *ctx = (struct interface_parse_context *)userData;
    
    /* Count attributes for debugging/logging */
    int attr_count = 0;
    if (atts) {
        while (atts[attr_count] && atts[attr_count+1]) attr_count += 2;
    }
    
    strncpy(ctx->current_tag, name, sizeof(ctx->current_tag) - 1);
    ctx->current_tag[sizeof(ctx->current_tag) - 1] = '\0';
    
    debug_log(DEBUG_TRACE, "XML: Start element: %s (in_interface=%d, in_group=%d, in_alias=%d)", 
              name, ctx->in_interface, ctx->in_group, ctx->in_alias);
    
    if (strcmp(name, "interface") == 0) {
        ctx->in_interface = 1;
        if (ctx->interface_count < ctx->max_interfaces) {
            ctx->current_interface = &ctx->interfaces[ctx->interface_count];
            memset(ctx->current_interface, 0, sizeof(struct interface_data));
            debug_log(DEBUG_DEBUG, "Starting interface %d", ctx->interface_count);
        }
    } else if (strcmp(name, "group") == 0) {
        ctx->in_group = 1;
    } else if (strcmp(name, "alias") == 0) {
        ctx->in_alias = 1;
    }
}

/**
 * End element handler for interface parsing
 */
static void interface_end_element(void *userData, const char *name)
{
    struct interface_parse_context *ctx = (struct interface_parse_context *)userData;
    
    if (strcmp(name, "interface") == 0) {
        if (ctx->in_interface && ctx->current_interface) {
            ctx->interface_count++;
            debug_log(DEBUG_DEBUG, "Added interface %d: %s", ctx->interface_count, ctx->current_interface->name);
        }
        ctx->in_interface = 0;
        ctx->current_interface = NULL;
    } else if (strcmp(name, "group") == 0) {
        ctx->in_group = 0;
    } else if (strcmp(name, "alias") == 0) {
        ctx->in_alias = 0;
    } else if (ctx->in_interface && ctx->current_interface) {
        /* Sanitize the content by trimming whitespace */
        char *content = ctx->temp_content;
        while (*content == ' ' || *content == '\t' || *content == '\n' || *content == '\r') {
            content++;
        }
        char *end = content + strlen(content) - 1;
        while (end > content && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
            end--;
        }
        *(end + 1) = '\0';
        
        debug_log(DEBUG_TRACE, "XML: End element: %s, content: '%s'", name, content);
        
        /* Process the content when we hit the end of a tag */
        if (strcmp(name, "name") == 0) {
            strncpy(ctx->current_interface->name, content, sizeof(ctx->current_interface->name) - 1);
            ctx->current_interface->name[sizeof(ctx->current_interface->name) - 1] = '\0';
        } else if (strcmp(name, "enabled") == 0) {
            strncpy(ctx->current_interface->enabled, content, sizeof(ctx->current_interface->enabled) - 1);
            ctx->current_interface->enabled[sizeof(ctx->current_interface->enabled) - 1] = '\0';
        } else if (strcmp(name, "vrf") == 0) {
            strncpy(ctx->current_interface->fib, content, sizeof(ctx->current_interface->fib) - 1);
            ctx->current_interface->fib[sizeof(ctx->current_interface->fib) - 1] = '\0';
        } else if (strcmp(name, "mtu") == 0) {
            strncpy(ctx->current_interface->mtu, content, sizeof(ctx->current_interface->mtu) - 1);
            ctx->current_interface->mtu[sizeof(ctx->current_interface->mtu) - 1] = '\0';
        } else if (strcmp(name, "flags") == 0) {
            strncpy(ctx->current_interface->flags, content, sizeof(ctx->current_interface->flags) - 1);
            ctx->current_interface->flags[sizeof(ctx->current_interface->flags) - 1] = '\0';
        } else if (strcmp(name, "primary-address") == 0) {
            strncpy(ctx->current_interface->addr, content, sizeof(ctx->current_interface->addr) - 1);
            ctx->current_interface->addr[sizeof(ctx->current_interface->addr) - 1] = '\0';
        } else if (strcmp(name, "primary-address6") == 0) {
            strncpy(ctx->current_interface->addr6, content, sizeof(ctx->current_interface->addr6) - 1);
            ctx->current_interface->addr6[sizeof(ctx->current_interface->addr6) - 1] = '\0';
        } else if (strcmp(name, "address") == 0 && ctx->in_alias) {
            if (ctx->current_interface->addr[0] != '\0') {
                strcat(ctx->current_interface->addr, ",");
            }
            strncat(ctx->current_interface->addr, content, sizeof(ctx->current_interface->addr) - strlen(ctx->current_interface->addr) - 1);
        } else if (strcmp(name, "address6") == 0 && ctx->in_alias) {
            if (ctx->current_interface->addr6[0] != '\0') {
                strcat(ctx->current_interface->addr6, ",");
            }
            strncat(ctx->current_interface->addr6, content, sizeof(ctx->current_interface->addr6) - strlen(ctx->current_interface->addr6) - 1);
        } else if (strcmp(name, "bridge-members") == 0) {
            strncpy(ctx->current_interface->bridge_members, content, sizeof(ctx->current_interface->bridge_members) - 1);
            ctx->current_interface->bridge_members[sizeof(ctx->current_interface->bridge_members) - 1] = '\0';
        } else if (strcmp(name, "group-name") == 0 && ctx->in_group) {
            /* Skip "all" group - it's not a real group */
            if (strcmp(content, "all") != 0) {
                if (ctx->current_interface->groups[0] != '\0') {
                    strcat(ctx->current_interface->groups, ",");
                }
                strncat(ctx->current_interface->groups, content, sizeof(ctx->current_interface->groups) - strlen(ctx->current_interface->groups) - 1);
            }
        }
        
        /* Clear temp content for next tag */
        ctx->temp_content[0] = '\0';
    }
}

/**
 * Character data handler for interface parsing
 */
static void interface_char_data(void *userData, const char *s, int len)
{
    struct interface_parse_context *ctx = (struct interface_parse_context *)userData;
    
    if (!ctx->in_interface || !ctx->current_interface) {
        return;
    }
    
    /* Copy character data to temp buffer, trimming whitespace */
    size_t current_len = strlen(ctx->temp_content);
    size_t remaining = sizeof(ctx->temp_content) - current_len - 1;
    
    if (remaining > 0) {
        size_t copy_len = (len < (int)remaining) ? (size_t)len : remaining;
        strncat(ctx->temp_content, s, copy_len);
    }
    
    // debug_log(DEBUG_TRACE, "XML: Char data: '%.*s' (len=%d)", len, s, len);
}

/**
 * Parse all interfaces from XML using Expat
 * @param xml XML string containing interface data
 * @param interfaces Array to store parsed interfaces
 * @param max_interfaces Maximum number of interfaces to parse
 * @return Number of interfaces parsed, or -1 on failure
 */
int parse_interfaces_from_xml(const char *xml, struct interface_data *interfaces, int max_interfaces)
{
    XML_Parser parser;
    struct interface_parse_context ctx;
    
    if (!xml || !interfaces || max_interfaces <= 0) {
        return -1;
    }
    
    /* Debug: show last 50 chars of XML */
    size_t xml_len = strlen(xml);
    if (xml_len > 50) {
        debug_log(DEBUG_DEBUG, "XML ends with: %.50s", xml + xml_len - 50);
    }
    debug_log(DEBUG_DEBUG, "XML length: %zu", xml_len);
    
    /* Initialize context */
    memset(&ctx, 0, sizeof(ctx));
    ctx.interfaces = interfaces;
    ctx.max_interfaces = max_interfaces;
    
    /* Create XML parser */
    parser = XML_ParserCreate(NULL);
    if (!parser) {
        return -1;
    }
    
    /* Set up handlers */
    XML_SetElementHandler(parser, interface_start_element, interface_end_element);
    XML_SetCharacterDataHandler(parser, interface_char_data);
    XML_SetUserData(parser, &ctx);
    
        /* Parse the XML */
    if (XML_Parse(parser, xml, strlen(xml), 1) != XML_STATUS_OK) {
        debug_log(DEBUG_ERROR, "XML_Parse failed");
        XML_ParserFree(parser);
        return -1;
    }

    /* Clean up */
    XML_ParserFree(parser);

    debug_log(DEBUG_DEBUG, "parse_interfaces_from_xml: parsed %d interfaces", ctx.interface_count);
    return ctx.interface_count;
}

 