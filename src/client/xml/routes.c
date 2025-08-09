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

/* Structure to hold route parsing context */
struct route_parse_context {
  struct route_data *routes;
  int max_routes;
  int route_count;
  struct route_data *current_route;
  int in_route;
  char current_tag[64];
  char temp_content[256];
};

/**
 * Start element handler for route parsing
 */
static void route_start_element(void *userData, const char *name,
                                 const char **atts) {
  struct route_parse_context *ctx =
      (struct route_parse_context *)userData;

  /* Debug log the attributes if they exist */
  if (atts && atts[0]) {
    debug_log(DEBUG_DEBUG, "Route start element: %s with attributes", name);
    for (int i = 0; atts[i] && atts[i+1]; i += 2) {
      debug_log(DEBUG_DEBUG, "  %s = %s", atts[i], atts[i+1]);
    }
  }

  strncpy(ctx->current_tag, name, sizeof(ctx->current_tag) - 1);
  ctx->current_tag[sizeof(ctx->current_tag) - 1] = '\0';

  if (strcmp(name, "route") == 0) {
    ctx->in_route = 1;
    if (ctx->route_count < ctx->max_routes) {
      ctx->current_route = &ctx->routes[ctx->route_count];
      memset(ctx->current_route, 0, sizeof(struct route_data));
    }
  }
}

/**
 * End element handler for route parsing
 */
static void route_end_element(void *userData, const char *name) {
  struct route_parse_context *ctx =
      (struct route_parse_context *)userData;

  if (strcmp(name, "route") == 0) {
    if (ctx->in_route && ctx->current_route) {
      ctx->route_count++;
    }
    ctx->in_route = 0;
    ctx->current_route = NULL;
  } else if (ctx->in_route && ctx->current_route) {
    /* Sanitize the content by trimming whitespace */
    char *content = ctx->temp_content;
    while (*content == ' ' || *content == '\t' || *content == '\n' ||
           *content == '\r') {
      content++;
    }
    char *end = content + strlen(content) - 1;
    while (end > content &&
           (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
      end--;
    }
    *(end + 1) = '\0';

    /* Process the content when we hit the end of a tag */
    if (strcmp(name, "destination") == 0) {
      strncpy(ctx->current_route->destination, content,
              sizeof(ctx->current_route->destination) - 1);
      ctx->current_route
          ->destination[sizeof(ctx->current_route->destination) - 1] = '\0';
    } else if (strcmp(name, "gateway") == 0) {
      strncpy(ctx->current_route->gateway, content,
              sizeof(ctx->current_route->gateway) - 1);
      ctx->current_route
          ->gateway[sizeof(ctx->current_route->gateway) - 1] = '\0';
    } else if (strcmp(name, "interface") == 0) {
      strncpy(ctx->current_route->interface, content,
              sizeof(ctx->current_route->interface) - 1);
      ctx->current_route
          ->interface[sizeof(ctx->current_route->interface) - 1] = '\0';
    } else if (strcmp(name, "flags") == 0) {
      /* Parse flags as integer */
      ctx->current_route->flags = atoi(content);
    } else if (strcmp(name, "prefix_length") == 0) {
      /* Parse prefix_length as integer */
      ctx->current_route->prefix_length = atoi(content);
    } else if (strcmp(name, "expire") == 0) {
      /* Parse expire as integer */
      ctx->current_route->expire = atoi(content);
    } else if (strcmp(name, "scope_interface") == 0) {
      strncpy(ctx->current_route->scope_interface, content,
              sizeof(ctx->current_route->scope_interface) - 1);
      ctx->current_route
          ->scope_interface[sizeof(ctx->current_route->scope_interface) - 1] = '\0';
    }

    /* Clear temp content for next tag */
    ctx->temp_content[0] = '\0';
  }
}

/**
 * Character data handler for route parsing
 */
static void route_char_data(void *userData, const char *s, int len) {
  struct route_parse_context *ctx =
      (struct route_parse_context *)userData;

  if (!ctx->in_route || !ctx->current_route) {
    return;
  }

  /* Copy character data to temp buffer */
  size_t current_len = strlen(ctx->temp_content);
  size_t remaining = sizeof(ctx->temp_content) - current_len - 1;

  if (remaining > 0) {
    size_t copy_len = (len < (int)remaining) ? (size_t)len : remaining;
    memcpy(ctx->temp_content + current_len, s, copy_len);
    ctx->temp_content[current_len + copy_len] = '\0';
  }
}

/**
 * Parse all routes from XML using Expat
 * @param xml XML string containing route data
 * @param routes Array to store parsed routes
 * @param max_routes Maximum number of routes to parse
 * @return Number of routes parsed, or -1 on failure
 */
int parse_routes_from_xml(const char *xml, struct route_data *routes,
                           int max_routes) {
  XML_Parser parser;
  struct route_parse_context ctx;

  if (!xml || !routes || max_routes <= 0) {
    return -1;
  }

  /* Validate XML input */
  if (strlen(xml) == 0) {
    return -1;
  }

  /* Initialize context */
  memset(&ctx, 0, sizeof(ctx));
  ctx.routes = routes;
  ctx.max_routes = max_routes;

  /* Create XML parser */
  parser = XML_ParserCreate(NULL);
  if (!parser) {
    return -1;
  }

  /* Set up handlers */
  XML_SetElementHandler(parser, route_start_element, route_end_element);
  XML_SetCharacterDataHandler(parser, route_char_data);
  XML_SetUserData(parser, &ctx);

  /* Parse the XML */
  if (XML_Parse(parser, xml, strlen(xml), 1) != XML_STATUS_OK) {
    debug_log(DEBUG_ERROR, "XML_Parse failed");
    XML_ParserFree(parser);
    return -1;
  }

  /* Clean up */
  XML_ParserFree(parser);

  return ctx.route_count;
} 