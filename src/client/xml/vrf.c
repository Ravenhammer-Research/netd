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

/* Structure to hold VRF parsing context */
struct vrf_parse_context {
  struct vrf_data *vrfs;
  int max_vrfs;
  int vrf_count;
  struct vrf_data *current_vrf;
  int in_vrf;
  char current_tag[64];
  char temp_content[256];
};

/**
 * Start element handler for VRF XML parsing
 */
static void vrf_start_element(void *userData, const char *name,
                              const char **atts) {
  (void)atts; /* Suppress unused parameter warning */
  struct vrf_parse_context *ctx = (struct vrf_parse_context *)userData;

  if (strcmp(name, "vrf") == 0) {
    if (ctx->vrf_count < ctx->max_vrfs) {
      ctx->current_vrf = &ctx->vrfs[ctx->vrf_count];
      ctx->in_vrf = 1;
      memset(ctx->current_vrf, 0, sizeof(struct vrf_data));
    }
  } else if (ctx->in_vrf) {
    strlcpy(ctx->current_tag, name, sizeof(ctx->current_tag));
    ctx->temp_content[0] = '\0';
  }
}

/**
 * End element handler for VRF XML parsing
 */
static void vrf_end_element(void *userData, const char *name) {
  struct vrf_parse_context *ctx = (struct vrf_parse_context *)userData;

  if (strcmp(name, "vrf") == 0) {
    if (ctx->current_vrf) {
      ctx->vrf_count++;
    }
    ctx->in_vrf = 0;
    ctx->current_vrf = NULL;
  } else if (ctx->in_vrf && ctx->current_vrf) {
    if (strcmp(name, "name") == 0) {
      strlcpy(ctx->current_vrf->name, ctx->temp_content,
              sizeof(ctx->current_vrf->name));
    } else if (strcmp(name, "description") == 0) {
      strlcpy(ctx->current_vrf->description, ctx->temp_content,
              sizeof(ctx->current_vrf->description));
    } else if (strcmp(name, "id") == 0) {
      ctx->current_vrf->fib = atoi(ctx->temp_content);
    }
  }
}

/**
 * Character data handler for VRF XML parsing
 */
static void vrf_char_data(void *userData, const char *s, int len) {
  struct vrf_parse_context *ctx = (struct vrf_parse_context *)userData;

  if (ctx->in_vrf && ctx->current_vrf) {
    size_t current_len = strlen(ctx->temp_content);
    size_t remaining = sizeof(ctx->temp_content) - current_len - 1;

    if (remaining > 0) {
      size_t copy_len = (len < (int)remaining) ? (int)len : (int)remaining;
      memcpy(ctx->temp_content + current_len, s, copy_len);
      ctx->temp_content[current_len + copy_len] = '\0';
    }
  }
}

/**
 * Parse VRFs from XML using Expat
 * @param xml XML string containing VRF data
 * @param vrfs Array to store parsed VRFs
 * @param max_vrfs Maximum number of VRFs to parse
 * @return Number of VRFs parsed, or -1 on failure
 */
int parse_vrfs_from_xml(const char *xml, struct vrf_data *vrfs, int max_vrfs) {
  XML_Parser parser;
  struct vrf_parse_context ctx;

  if (!xml || !vrfs || max_vrfs <= 0) {
    return -1;
  }

  /* Initialize context */
  memset(&ctx, 0, sizeof(ctx));
  ctx.vrfs = vrfs;
  ctx.max_vrfs = max_vrfs;
  ctx.vrf_count = 0;

  /* Create parser */
  parser = XML_ParserCreate(NULL);
  if (!parser) {
    return -1;
  }

  /* Set handlers */
  XML_SetElementHandler(parser, vrf_start_element, vrf_end_element);
  XML_SetCharacterDataHandler(parser, vrf_char_data);
  XML_SetUserData(parser, &ctx);

  /* Parse XML */
  if (XML_Parse(parser, xml, strlen(xml), 1) != XML_STATUS_OK) {
    debug_log(DEBUG_ERROR, "XML_Parse failed: %s",
              XML_ErrorString(XML_GetErrorCode(parser)));
    XML_ParserFree(parser);
    return -1;
  }

  XML_ParserFree(parser);
  return ctx.vrf_count;
} 