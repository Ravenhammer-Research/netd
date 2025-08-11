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

#include <xml.h>
#include <bsdxml.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct vrf_name_data {
  char vrf_name[64];
  bool found;
  bool in_routing_instance;
  bool captured;
};

/* Callback for extracting VRF name */
static void vrf_name_start_element(void *userData, const XML_Char *name,
                                   const XML_Char **atts) {
  struct vrf_name_data *data = (struct vrf_name_data *)userData;

  debug_log(DEBUG1, "vrf_name_start_element called with name=%s, atts=%p", name, atts);

  if (strcmp(name, "routing-instance") == 0) {
    data->in_routing_instance = true;
  } else if (strcmp(name, "name") == 0 && data->in_routing_instance && !data->captured) {
    data->found = true;
  }
}

static void vrf_name_end_element(void *userData, const XML_Char *name) {
  struct vrf_name_data *data = (struct vrf_name_data *)userData;

  if (strcmp(name, "routing-instance") == 0) {
    data->in_routing_instance = false;
  } else if (strcmp(name, "name") == 0) {
    data->found = false;
  }
}

static void vrf_name_character_data(void *userData, const XML_Char *s, int len) {
  struct vrf_name_data *data = (struct vrf_name_data *)userData;

  if (data->found && !data->captured && len < (int)sizeof(data->vrf_name)) {
    strncpy(data->vrf_name, s, len);
    data->vrf_name[len] = '\0';
    data->captured = true;
  }
}

/**
 * Extract VRF name from NETCONF request
 * @param request XML request string
 * @return VRF name string or NULL if not found
 */
char *extract_vrf_name_from_request(const char *request) {
  XML_Parser parser;
  struct vrf_name_data user_data = {"", false, false, false};
  char *result = NULL;

  if (!request) {
    return NULL;
  }

  parser = XML_ParserCreate(NULL);
  if (!parser) {
    return NULL;
  }

  XML_SetUserData(parser, &user_data);
  XML_SetStartElementHandler(parser, vrf_name_start_element);
  XML_SetEndElementHandler(parser, vrf_name_end_element);
  XML_SetCharacterDataHandler(parser, vrf_name_character_data);

  if (XML_Parse(parser, request, strlen(request), 1) != XML_STATUS_OK) {
    XML_ParserFree(parser);
    return NULL;
  }

  if (user_data.captured && strlen(user_data.vrf_name) > 0) {
    result = strdup(user_data.vrf_name);
  }

  XML_ParserFree(parser);
  return result;
} 