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

#include <net.h>
#include <bsdxml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xml.h>

/**
 * Start element handler for interface parsing
 */
static void interface_start_element(void *userData, const char *name,
                                    const char **atts) {
  struct interface_parse_context *ctx =
      (struct interface_parse_context *)userData;

  /* Count attributes for debugging/logging */
  int attr_count = 0;
  if (atts) {
    while (atts[attr_count] && atts[attr_count + 1])
      attr_count += 2;
  }

  strncpy(ctx->current_tag, name, sizeof(ctx->current_tag) - 1);
  ctx->current_tag[sizeof(ctx->current_tag) - 1] = '\0';

  // debug_log(DEBUG, "XML: Start element: %s (in_interface=%d,
  // in_group=%d, in_alias=%d)",
  //           name, ctx->in_interface, ctx->in_group, ctx->in_alias);

  if (strcmp(name, "interface") == 0) {
    ctx->in_interface = 1;
    if (ctx->interface_count < ctx->max_interfaces) {
      ctx->current_interface = &ctx->interfaces[ctx->interface_count];
      memset(ctx->current_interface, 0, sizeof(struct interface_data));
      // debug_log(DEBUG, "Starting interface %d", ctx->interface_count);
    }
  } else if (strcmp(name, "alias") == 0) {
    ctx->in_alias = 1;
  } else if (strcmp(name, "ipv4") == 0 || strstr(name, "ipv4") != NULL) {
    ctx->in_ipv4 = 1;
  } else if (strcmp(name, "ipv6") == 0 || strstr(name, "ipv6") != NULL) {
    ctx->in_ipv6 = 1;
  }
}

/**
 * End element handler for interface parsing
 */
static void interface_end_element(void *userData, const char *name) {
  struct interface_parse_context *ctx =
      (struct interface_parse_context *)userData;

  if (strcmp(name, "interface") == 0) {
    if (ctx->in_interface && ctx->current_interface) {
      ctx->interface_count++;
      // debug_log(DEBUG, "Added interface %d: %s", ctx->interface_count,
      // ctx->current_interface->name);
    }
    ctx->in_interface = 0;
    ctx->current_interface = NULL;
  } else if (strcmp(name, "alias") == 0) {
    ctx->in_alias = 0;
  } else if (strcmp(name, "ipv4") == 0 || strstr(name, "ipv4") != NULL) {
    ctx->in_ipv4 = 0;
  } else if (strcmp(name, "ipv6") == 0 || strstr(name, "ipv6") != NULL) {
    ctx->in_ipv6 = 0;
  } else if (ctx->in_interface && ctx->current_interface) {
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

    // debug_log(DEBUG, "XML: End element: %s, content: '%s'", name,
    // content);

    /* Process the content when we hit the end of a tag */
    if (strcmp(name, "name") == 0) {
      strncpy(ctx->current_interface->name, content,
              sizeof(ctx->current_interface->name) - 1);
      ctx->current_interface->name[sizeof(ctx->current_interface->name) - 1] =
          '\0';
    } else if (strcmp(name, "enabled") == 0) {
      strncpy(ctx->current_interface->enabled, content,
              sizeof(ctx->current_interface->enabled) - 1);
      ctx->current_interface
          ->enabled[sizeof(ctx->current_interface->enabled) - 1] = '\0';
    } else if (strcmp(name, "bind-ni-name") == 0 ||
               strstr(name, "bind-ni-name") != NULL) {
      strncpy(ctx->current_interface->fib, content,
              sizeof(ctx->current_interface->fib) - 1);
      ctx->current_interface->fib[sizeof(ctx->current_interface->fib) - 1] =
          '\0';
    } else if (strcmp(name, "mtu") == 0) {
      strncpy(ctx->current_interface->mtu, content,
              sizeof(ctx->current_interface->mtu) - 1);
      ctx->current_interface->mtu[sizeof(ctx->current_interface->mtu) - 1] =
          '\0';
    } else if (strcmp(name, "flags") == 0 || strstr(name, "flags") != NULL) {
      strncpy(ctx->current_interface->flags, content,
              sizeof(ctx->current_interface->flags) - 1);
      ctx->current_interface->flags[sizeof(ctx->current_interface->flags) - 1] =
          '\0';
    } else if (strcmp(name, "type") == 0) {
      strncpy(ctx->current_interface->type, content,
              sizeof(ctx->current_interface->type) - 1);
      ctx->current_interface->type[sizeof(ctx->current_interface->type) - 1] =
          '\0';
      /* Also store the interface type in the context for later use */
      strncpy(ctx->interface_type, content, sizeof(ctx->interface_type) - 1);
      ctx->interface_type[sizeof(ctx->interface_type) - 1] = '\0';
    /* Note: Interface-specific fields (bridge, vlan, wifi, etc.) are now handled 
       in separate parsing functions for each interface type. This parser only 
       handles the base interface_data fields. */
    } else if (strcmp(name, "group") == 0 || strstr(name, "group") != NULL) {
      strncpy(ctx->current_interface->groups, content,
              sizeof(ctx->current_interface->groups) - 1);
      ctx->current_interface
          ->groups[sizeof(ctx->current_interface->groups) - 1] = '\0';

    } else if (strcmp(name, "ip") == 0) {
      /* This is an IP address under ipv4 or ipv6 container */
      if (ctx->in_ipv4) {
        if (ctx->current_interface->addr[0] != '\0') {
          size_t current_len = strlen(ctx->current_interface->addr);
          if (current_len + 1 < sizeof(ctx->current_interface->addr)) {
            ctx->current_interface->addr[current_len] = ',';
            ctx->current_interface->addr[current_len + 1] = '\0';
          }
        }
        size_t current_len = strlen(ctx->current_interface->addr);
        size_t remaining =
            sizeof(ctx->current_interface->addr) - current_len - 1;
        if (remaining > 0) {
          size_t copy_len =
              (strlen(content) < remaining) ? strlen(content) : remaining;
          memcpy(ctx->current_interface->addr + current_len, content, copy_len);
          ctx->current_interface->addr[current_len + copy_len] = '\0';
        }
      } else if (ctx->in_ipv6) {
        if (ctx->current_interface->addr6[0] != '\0') {
          size_t current_len = strlen(ctx->current_interface->addr6);
          if (current_len + 1 < sizeof(ctx->current_interface->addr6)) {
            ctx->current_interface->addr6[current_len] = ',';
            ctx->current_interface->addr6[current_len + 1] = '\0';
          }
        }
        size_t current_len = strlen(ctx->current_interface->addr6);
        size_t remaining =
            sizeof(ctx->current_interface->addr6) - current_len - 1;
        if (remaining > 0) {
          size_t copy_len =
              (strlen(content) < remaining) ? strlen(content) : remaining;
          memcpy(ctx->current_interface->addr6 + current_len, content,
                 copy_len);
          ctx->current_interface->addr6[current_len + copy_len] = '\0';
        }
      }
    }

    /* Clear temp content for next tag */
    ctx->temp_content[0] = '\0';
  }
}

/**
 * Character data handler for interface parsing
 */
static void interface_char_data(void *userData, const char *s, int len) {
  struct interface_parse_context *ctx =
      (struct interface_parse_context *)userData;

  if (!ctx->in_interface || !ctx->current_interface) {
    return;
  }

  /* Copy character data to temp buffer, trimming whitespace */
  size_t current_len = strlen(ctx->temp_content);
  size_t remaining = sizeof(ctx->temp_content) - current_len - 1;

  if (remaining > 0) {
    size_t copy_len = (len < (int)remaining) ? (size_t)len : remaining;
    memcpy(ctx->temp_content + current_len, s, copy_len);
    ctx->temp_content[current_len + copy_len] = '\0';
  }

  // debug_log(DEBUG, "XML: Char data: '%.*s' (len=%d)", len, s, len);
}

/**
 * Parse all interfaces from XML using Expat
 * @param xml XML string containing interface data
 * @param interfaces Array to store parsed interfaces
 * @param max_interfaces Maximum number of interfaces to parse
 * @return Number of interfaces parsed, or -1 on failure
 */
int parse_interfaces_from_xml(const char *xml,
                              struct interface_data *interfaces,
                              int max_interfaces) {
  XML_Parser parser;
  struct interface_parse_context ctx;

  if (!xml || !interfaces || max_interfaces <= 0) {
    return -1;
  }

  /* Validate XML input */
  if (strlen(xml) == 0) {
    return -1;
  }

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
    debug_log(ERROR, "XML_Parse failed");
    XML_ParserFree(parser);
    return -1;
  }

  /* Clean up */
  XML_ParserFree(parser);

  // debug_log(DEBUG, "parse_interfaces_from_xml: parsed %d interfaces",
  // ctx.interface_count);
  return ctx.interface_count;
}

/**
 * Start element handler for bridge interface parsing
 */
static void bridge_start_element(void *userData, const char *name,
                                 const char **atts) {
  struct bridge_parse_context *ctx = (struct bridge_parse_context *)userData;

  /* Count attributes for debugging/logging */
  int attr_count = 0;
  if (atts) {
    while (atts[attr_count] && atts[attr_count + 1])
      attr_count += 2;
  }

  strncpy(ctx->current_tag, name, sizeof(ctx->current_tag) - 1);
  ctx->current_tag[sizeof(ctx->current_tag) - 1] = '\0';

  if (strcmp(name, "interface") == 0) {
    ctx->in_interface = 1;
    if (ctx->interface_count < ctx->max_interfaces) {
      ctx->current_interface = &ctx->interfaces[ctx->interface_count];
      memset(ctx->current_interface, 0, sizeof(struct bridge_interface_data));
    }
  } else if (strcmp(name, "bridge-members") == 0 || strstr(name, "bridge-members") != NULL) {
    ctx->in_bridge_members = 1;
  } else if (strcmp(name, "ipv4") == 0 || strstr(name, "ipv4") != NULL) {
    ctx->in_ipv4 = 1;
  } else if (strcmp(name, "ipv6") == 0 || strstr(name, "ipv6") != NULL) {
    ctx->in_ipv6 = 1;
  }
}

/**
 * End element handler for bridge interface parsing
 */
static void bridge_end_element(void *userData, const char *name) {
  struct bridge_parse_context *ctx = (struct bridge_parse_context *)userData;

  if (strcmp(name, "interface") == 0) {
    if (ctx->in_interface && ctx->current_interface) {
      ctx->interface_count++;
    }
    ctx->in_interface = 0;
    ctx->current_interface = NULL;
  } else if (strcmp(name, "bridge-members") == 0 || strstr(name, "bridge-members") != NULL) {
    ctx->in_bridge_members = 0;
  } else if (strcmp(name, "ipv4") == 0 || strstr(name, "ipv4") != NULL) {
    ctx->in_ipv4 = 0;
  } else if (strcmp(name, "ipv6") == 0 || strstr(name, "ipv6") != NULL) {
    ctx->in_ipv6 = 0;
  } else if (ctx->in_interface && ctx->current_interface) {
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
    if (strcmp(name, "name") == 0) {
      strncpy(ctx->current_interface->base.name, content,
              sizeof(ctx->current_interface->base.name) - 1);
      ctx->current_interface->base.name[sizeof(ctx->current_interface->base.name) - 1] = '\0';
    } else if (strcmp(name, "enabled") == 0) {
      strncpy(ctx->current_interface->base.enabled, content,
              sizeof(ctx->current_interface->base.enabled) - 1);
      ctx->current_interface->base.enabled[sizeof(ctx->current_interface->base.enabled) - 1] = '\0';
    } else if (strcmp(name, "bind-ni-name") == 0 || strstr(name, "bind-ni-name") != NULL) {
      strncpy(ctx->current_interface->base.fib, content,
              sizeof(ctx->current_interface->base.fib) - 1);
      ctx->current_interface->base.fib[sizeof(ctx->current_interface->base.fib) - 1] = '\0';
    } else if (strcmp(name, "mtu") == 0) {
      strncpy(ctx->current_interface->base.mtu, content,
              sizeof(ctx->current_interface->base.mtu) - 1);
      ctx->current_interface->base.mtu[sizeof(ctx->current_interface->base.mtu) - 1] = '\0';
    } else if (strcmp(name, "flags") == 0 || strstr(name, "flags") != NULL) {
      strncpy(ctx->current_interface->base.flags, content,
              sizeof(ctx->current_interface->base.flags) - 1);
      ctx->current_interface->base.flags[sizeof(ctx->current_interface->base.flags) - 1] = '\0';
    } else if (strcmp(name, "type") == 0) {
      strncpy(ctx->current_interface->base.type, content,
              sizeof(ctx->current_interface->base.type) - 1);
      ctx->current_interface->base.type[sizeof(ctx->current_interface->base.type) - 1] = '\0';
    } else if (strcmp(name, "group") == 0 || strstr(name, "group") != NULL) {
      strncpy(ctx->current_interface->base.groups, content,
              sizeof(ctx->current_interface->base.groups) - 1);
      ctx->current_interface->base.groups[sizeof(ctx->current_interface->base.groups) - 1] = '\0';
    } else if (strcmp(name, "bridge-members") == 0 || strstr(name, "bridge-members") != NULL) {
      strncpy(ctx->current_interface->bridge_members, content,
              sizeof(ctx->current_interface->bridge_members) - 1);
      ctx->current_interface->bridge_members[sizeof(ctx->current_interface->bridge_members) - 1] = '\0';
    } else if (strcmp(name, "ip") == 0) {
      /* This is an IP address under ipv4 or ipv6 container */
      if (ctx->in_ipv4) {
        if (ctx->current_interface->base.addr[0] != '\0') {
          size_t current_len = strlen(ctx->current_interface->base.addr);
          if (current_len + 1 < sizeof(ctx->current_interface->base.addr)) {
            ctx->current_interface->base.addr[current_len] = ',';
            ctx->current_interface->base.addr[current_len + 1] = '\0';
          }
        }
        size_t current_len = strlen(ctx->current_interface->base.addr);
        size_t remaining = sizeof(ctx->current_interface->base.addr) - current_len - 1;
        if (remaining > 0) {
          size_t copy_len = (strlen(content) < remaining) ? strlen(content) : remaining;
          memcpy(ctx->current_interface->base.addr + current_len, content, copy_len);
          ctx->current_interface->base.addr[current_len + copy_len] = '\0';
        }
      } else if (ctx->in_ipv6) {
        if (ctx->current_interface->base.addr6[0] != '\0') {
          size_t current_len = strlen(ctx->current_interface->base.addr6);
          if (current_len + 1 < sizeof(ctx->current_interface->base.addr6)) {
            ctx->current_interface->base.addr6[current_len] = ',';
            ctx->current_interface->base.addr6[current_len + 1] = '\0';
          }
        }
        size_t current_len = strlen(ctx->current_interface->base.addr6);
        size_t remaining = sizeof(ctx->current_interface->base.addr6) - current_len - 1;
        if (remaining > 0) {
          size_t copy_len = (strlen(content) < remaining) ? strlen(content) : remaining;
          memcpy(ctx->current_interface->base.addr6 + current_len, content, copy_len);
          ctx->current_interface->base.addr6[current_len + copy_len] = '\0';
        }
      }
    }

    /* Clear temp content for next tag */
    ctx->temp_content[0] = '\0';
  }
}

/**
 * Character data handler for bridge interface parsing
 */
static void bridge_char_data(void *userData, const char *s, int len) {
  struct bridge_parse_context *ctx = (struct bridge_parse_context *)userData;

  if (!ctx->in_interface || !ctx->current_interface) {
    return;
  }

  /* Copy character data to temp buffer, trimming whitespace */
  size_t current_len = strlen(ctx->temp_content);
  size_t remaining = sizeof(ctx->temp_content) - current_len - 1;

  if (remaining > 0) {
    size_t copy_len = (len < (int)remaining) ? (size_t)len : remaining;
    memcpy(ctx->temp_content + current_len, s, copy_len);
    ctx->temp_content[current_len + copy_len] = '\0';
  }
}

/**
 * Parse bridge interfaces from XML using Expat
 * @param xml XML string containing bridge interface data
 * @param interfaces Array to store parsed bridge interfaces
 * @param max_interfaces Maximum number of interfaces to parse
 * @return Number of interfaces parsed, or -1 on failure
 */
int parse_bridge_interfaces_from_xml(const char *xml,
                                     struct bridge_interface_data *interfaces,
                                     int max_interfaces) {
  XML_Parser parser;
  struct bridge_parse_context ctx;

  if (!xml || !interfaces || max_interfaces <= 0) {
    return -1;
  }

  /* Validate XML input */
  if (strlen(xml) == 0) {
    return -1;
  }

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
  XML_SetElementHandler(parser, bridge_start_element, bridge_end_element);
  XML_SetCharacterDataHandler(parser, bridge_char_data);
  XML_SetUserData(parser, &ctx);

  /* Parse the XML */
  if (XML_Parse(parser, xml, strlen(xml), 1) != XML_STATUS_OK) {
    debug_log(ERROR, "XML_Parse failed for bridge interfaces");
    XML_ParserFree(parser);
    return -1;
  }

  /* Clean up */
  XML_ParserFree(parser);

  debug_log(INFO, "Parsed %d bridge interfaces from XML", ctx.interface_count);
  return ctx.interface_count;
} 