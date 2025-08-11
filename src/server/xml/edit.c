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
#include <netd.h>
#include <bsdxml.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* Callback for edit-config start element */
static void edit_config_start_element(void *userData, const XML_Char *name,
                                      const XML_Char **atts) {
  struct edit_config_data *data = (struct edit_config_data *)userData;

  if (strcmp(name, "edit-config") == 0) {
    data->is_edit_config = true;
    debug_log(DEBUG, "Found edit-config element");
  } else if (data->is_edit_config) {
    /* Only log elements when we're in an edit-config context */
    debug_log(DEBUG, "Edit config start element: %s with %d attributes", name, atts ? 0 : 0);
    
    if (strcmp(name, "config") == 0) {
    data->in_config = true;
  } else if (strcmp(name, "vrf") == 0) {
    data->in_vrf = true;
    data->has_vrf_config = true;
  } else if (strcmp(name, "route") == 0) {
    data->in_route = true;
    data->has_route_config = true;
  } else if (strcmp(name, "set") == 0) {
    data->in_set = true;
    data->has_set_config = true;
    }
  }

  strlcpy(data->current_tag, name, sizeof(data->current_tag));
  data->temp_content[0] = '\0';
}

/* Callback for edit-config end element */
static void edit_config_end_element(void *userData, const XML_Char *name) {
  struct edit_config_data *data = (struct edit_config_data *)userData;

  if (strcmp(name, "edit-config") == 0) {
    data->is_edit_config = false;
  } else if (strcmp(name, "config") == 0) {
    data->in_config = false;
  } else if (strcmp(name, "vrf") == 0) {
    data->in_vrf = false;
  } else if (strcmp(name, "route") == 0) {
    data->in_route = false;
  } else if (strcmp(name, "set") == 0) {
    data->in_set = false;
  }
}

/* Callback for edit-config character data */
static void edit_config_char_data(void *userData, const XML_Char *s, int len) {
  struct edit_config_data *data = (struct edit_config_data *)userData;

  if (len < (int)sizeof(data->temp_content)) {
    strncpy(data->temp_content, s, len);
    data->temp_content[len] = '\0';

    if (data->in_vrf) {
      if (strcmp(data->current_tag, "name") == 0) {
        strlcpy(data->vrf_name, data->temp_content, sizeof(data->vrf_name));
      } else if (strcmp(data->current_tag, "table") == 0) {
        strlcpy(data->vrf_table, data->temp_content, sizeof(data->vrf_table));
      }
    } else if (data->in_route) {
      if (strcmp(data->current_tag, "destination") == 0) {
        strlcpy(data->route_destination, data->temp_content, sizeof(data->route_destination));
      } else if (strcmp(data->current_tag, "gateway") == 0) {
        strlcpy(data->route_gateway, data->temp_content, sizeof(data->route_gateway));
      } else if (strcmp(data->current_tag, "interface") == 0) {
        strlcpy(data->route_interface, data->temp_content, sizeof(data->route_interface));
      } else if (strcmp(data->current_tag, "type") == 0) {
        strlcpy(data->route_type, data->temp_content, sizeof(data->route_type));
      } else if (strcmp(data->current_tag, "enabled") == 0) {
        data->route_enabled = (strcmp(data->temp_content, "true") == 0);
      }
    } else if (data->in_set) {
      if (strcmp(data->current_tag, "object") == 0) {
        strlcpy(data->set_object, data->temp_content, sizeof(data->set_object));
      } else if (strcmp(data->current_tag, "type") == 0) {
        strlcpy(data->set_type, data->temp_content, sizeof(data->set_type));
      } else if (strcmp(data->current_tag, "name") == 0) {
        strlcpy(data->set_name, data->temp_content, sizeof(data->set_name));
      } else if (strcmp(data->current_tag, "property") == 0) {
        strlcpy(data->set_property, data->temp_content, sizeof(data->set_property));
      } else if (strcmp(data->current_tag, "value") == 0) {
        strlcpy(data->set_value, data->temp_content, sizeof(data->set_value));
      } else if (strcmp(data->current_tag, "sub-property") == 0) {
        strlcpy(data->set_sub_property, data->temp_content, sizeof(data->set_sub_property));
      } else if (strcmp(data->current_tag, "sub-value") == 0) {
        strlcpy(data->set_sub_value, data->temp_content, sizeof(data->set_sub_value));
      }
    }
  }
}

/**
 * Check if request is an edit-config request
 * @param request XML request string
 * @return true if edit-config request, false otherwise
 */
bool is_edit_config_request(const char *request) {
  XML_Parser parser;
  struct edit_config_data user_data = {0};

  if (!request) {
    return false;
  }

  parser = XML_ParserCreate(NULL);
  if (!parser) {
    debug_log(ERROR, "Failed to create XML parser for edit-config check");
    return false;
  }

  XML_SetUserData(parser, &user_data);
  XML_SetStartElementHandler(parser, edit_config_start_element);
  XML_SetEndElementHandler(parser, edit_config_end_element);
  XML_SetCharacterDataHandler(parser, edit_config_char_data);

  if (XML_Parse(parser, request, strlen(request), 1) != XML_STATUS_OK) {
    debug_log(ERROR, "XML parsing failed for edit-config check: %s",
              XML_ErrorString(XML_GetErrorCode(parser)));
  }

  XML_ParserFree(parser);
  return user_data.is_edit_config;
}

/**
 * Process parsed edit-config data
 * @param state Server state
 * @param data Parsed edit-config data
 * @return 0 on success, -1 on failure
 */
static int process_edit_config_data(netd_state_t *state,
                                    struct edit_config_data *data) {
  int ret = 0;

  if (!state || !data) {
    return -1;
  }

  /* Process VRF configuration */
  if (data->has_vrf_config && strlen(data->vrf_name) > 0) {
    uint32_t fib = 0;
    if (strlen(data->vrf_table) > 0) {
      fib = (uint32_t)strtoul(data->vrf_table, NULL, 10);
    }
    
    ret = add_pending_vrf_create(state, data->vrf_name, fib);
    if (ret < 0) {
      debug_log(ERROR, "Failed to add pending VRF creation");
      return ret;
    }
  }

  /* Process route configuration */
  if (data->has_route_config && strlen(data->route_destination) > 0) {
    int flags = 0;
    if (strlen(data->route_type) > 0) {
      if (strcmp(data->route_type, "reject") == 0) {
        flags |= ROUTE_FLAG_REJECT;
      } else if (strcmp(data->route_type, "blackhole") == 0) {
        flags |= ROUTE_FLAG_BLACKHOLE;
      }
    }

    ret = add_pending_route_add(state, 0, data->route_destination,
                                data->route_gateway, data->route_interface, flags);
    if (ret < 0) {
      debug_log(ERROR, "Failed to add pending route addition");
      return ret;
    }
  }

  /* Process set configuration */
  if (data->has_set_config && strlen(data->set_object) > 0) {
    if (strcmp(data->set_object, "interface") == 0) {
      if (strcmp(data->set_property, "fib") == 0) {
        uint32_t fib = (uint32_t)strtoul(data->set_value, NULL, 10);
        ret = add_pending_interface_set_fib(state, data->set_name, fib);
        if (ret < 0) {
          debug_log(ERROR, "Failed to add pending interface FIB change");
          return ret;
        }
      }
    }
  }

  return 0;
}

/**
 * Process edit-config request using XML parsing
 * @param state Server state
 * @param request XML request string
 * @return 0 on success, -1 on failure
 */
int process_edit_config_request(netd_state_t *state, const char *request) {
  XML_Parser parser;
  struct edit_config_data user_data = {0};
  int ret = 0;

  if (!state || !request) {
    return -1;
  }

  parser = XML_ParserCreate(NULL);
  if (!parser) {
    debug_log(ERROR, "Failed to create XML parser for edit-config");
    return -1;
  }

  XML_SetUserData(parser, &user_data);
  XML_SetStartElementHandler(parser, edit_config_start_element);
  XML_SetEndElementHandler(parser, edit_config_end_element);
  XML_SetCharacterDataHandler(parser, edit_config_char_data);

  if (XML_Parse(parser, request, strlen(request), 1) != XML_STATUS_OK) {
    debug_log(ERROR, "XML parsing failed for edit-config: %s",
              XML_ErrorString(XML_GetErrorCode(parser)));
    ret = -1;
  } else {
    /* Process the parsed configuration */
    ret = process_edit_config_data(state, &user_data);
  }

  XML_ParserFree(parser);
  return ret;
} 