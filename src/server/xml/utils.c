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
#include <expat.h>
#include <netd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Callback data structures for general XML parsing */
struct message_id_data {
  char *message_id;
  bool found;
};

/* Callback for extracting message-id */
static void message_id_start_element(void *userData, const XML_Char *name,
                                     const XML_Char **atts) {
  struct message_id_data *data = (struct message_id_data *)userData;

  if (strcmp(name, "rpc") == 0 || strcmp(name, "rpc-reply") == 0) {
    for (int i = 0; atts[i] != NULL; i += 2) {
      if (strcmp(atts[i], "message-id") == 0) {
        data->message_id = strdup(atts[i + 1]);
        data->found = true;
        break;
      }
    }
  }
}

/**
 * Extract message-id from NETCONF request
 * @param request XML request string
 * @return message-id string or NULL if not found
 */
char *extract_message_id(const char *request) {
  XML_Parser parser;
  struct message_id_data user_data = {NULL, false};
  char *result = NULL;

  if (!request) {
    return NULL;
  }

  parser = XML_ParserCreate(NULL);
  if (!parser) {
    debug_log(ERROR, "Failed to create XML parser for message-id extraction");
    return NULL;
  }

  XML_SetUserData(parser, &user_data);
  XML_SetStartElementHandler(parser, message_id_start_element);

  if (XML_Parse(parser, request, strlen(request), 1) != XML_STATUS_OK) {
    debug_log(ERROR, "XML parsing failed for message-id extraction: %s",
              XML_ErrorString(XML_GetErrorCode(parser)));
  } else if (user_data.found) {
    result = user_data.message_id;
  }

  XML_ParserFree(parser);
  return result;
}

/* Callback data structures for type extraction */
struct type_data {
  char *type_value;
  bool found;
  bool in_type_element;
};

/* Callback for extracting type value */
static void type_start_element(void *userData, const XML_Char *name,
                               const XML_Char **atts) {
  struct type_data *data = (struct type_data *)userData;

  debug_log(DEBUG, "Type extraction - element: %s, attributes: %p", name, (void*)atts);
  
  if (strcmp(name, "type") == 0) {
    data->in_type_element = true;
    debug_log(DEBUG, "Found type element, setting in_type_element to true");
  }
}

/* Callback for extracting type value */
static void type_end_element(void *userData, const XML_Char *name) {
  struct type_data *data = (struct type_data *)userData;

  if (strcmp(name, "type") == 0) {
    data->in_type_element = false;
    debug_log(DEBUG, "End of type element, setting in_type_element to false");
  }
}

/* Callback for extracting type value */
static void type_char_data(void *userData, const XML_Char *s, int len) {
  struct type_data *data = (struct type_data *)userData;

  if (data->in_type_element && !data->found) {
    debug_log(DEBUG, "Type extraction - character data: '%.*s' (len: %d)", len, s, len);
    
    /* Allocate memory for the type value */
    data->type_value = malloc(len + 1);
    if (data->type_value) {
      strncpy(data->type_value, s, len);
      data->type_value[len] = '\0';
      data->found = true;
      debug_log(DEBUG, "Extracted type value: %s", data->type_value);
    } else {
      debug_log(ERROR, "Failed to allocate memory for type value");
    }
  }
}

/**
 * Extract interface type from XML request using proper XML parsing
 * @param request XML request string
 * @return Type value string or NULL if not found
 */
const char *extract_type_from_xml_request(const char *request) {
  XML_Parser parser;
  struct type_data user_data = {NULL, false, false};
  const char *result = NULL;

  if (!request) {
    debug_log(DEBUG, "extract_type_from_xml_request: NULL request");
    return NULL;
  }

  debug_log(DEBUG, "extract_type_from_xml_request: parsing request: %s", request);

  parser = XML_ParserCreate(NULL);
  if (!parser) {
    debug_log(ERROR, "Failed to create XML parser for type extraction");
    return NULL;
  }

  XML_SetUserData(parser, &user_data);
  XML_SetStartElementHandler(parser, type_start_element);
  XML_SetEndElementHandler(parser, type_end_element);
  XML_SetCharacterDataHandler(parser, type_char_data);

  if (XML_Parse(parser, request, strlen(request), 1) != XML_STATUS_OK) {
    debug_log(ERROR, "XML parsing failed for type extraction: %s",
              XML_ErrorString(XML_GetErrorCode(parser)));
  } else if (user_data.found) {
    result = user_data.type_value;
    debug_log(DEBUG, "extract_type_from_xml_request: successfully extracted type: %s", result);
  } else {
    debug_log(DEBUG, "extract_type_from_xml_request: no type found in request");
  }

  XML_ParserFree(parser);
  return result;
}

/* Callback data structures for general XML parsing */
struct xml_check_data {
  bool found_elements[MAX_XML_ELEMENTS];
  char *element_names[MAX_XML_ELEMENTS];
  int element_count;
};

/* Callback for checking XML elements */
static void xml_check_start_element(void *userData, const XML_Char *name,
                                   const XML_Char **atts) {
  struct xml_check_data *data = (struct xml_check_data *)userData;
  
  debug_log(DEBUG2, "XML element: %s, attributes: %p", name, (void*)atts);
  
  if (data->element_count < MAX_XML_ELEMENTS) {
    data->element_names[data->element_count] = strdup(name);
    data->found_elements[data->element_count] = true;
    data->element_count++;
  }
}

/**
 * Check if XML request contains specific elements
 * @param request XML request string
 * @param elements Array of element names to check for
 * @param element_count Number of elements to check
 * @return true if all specified elements are found, false otherwise
 */
bool xml_contains_elements(const char *request, const char **elements, int element_count) {
  XML_Parser parser;
  struct xml_check_data user_data = {0};
  bool result = false;

  if (!request || !elements || element_count <= 0 || element_count > MAX_XML_ELEMENTS) {
    return false;
  }

  parser = XML_ParserCreate(NULL);
  if (!parser) {
    debug_log(ERROR, "Failed to create XML parser for element checking");
    return false;
  }

  XML_SetUserData(parser, &user_data);
  XML_SetStartElementHandler(parser, xml_check_start_element);

  if (XML_Parse(parser, request, strlen(request), 1) != XML_STATUS_OK) {
    debug_log(ERROR, "XML parsing failed for element checking: %s",
              XML_ErrorString(XML_GetErrorCode(parser)));
  } else {
    /* Check if all required elements are found */
    result = true;
    for (int i = 0; i < element_count; i++) {
      bool found = false;
      for (int j = 0; j < user_data.element_count; j++) {
        if (strcmp(elements[i], user_data.element_names[j]) == 0) {
          found = true;
          break;
        }
      }
      if (!found) {
        result = false;
        break;
      }
    }
  }

  /* Clean up allocated memory */
  for (int i = 0; i < user_data.element_count; i++) {
    free(user_data.element_names[i]);
    user_data.element_names[i] = NULL;
  }

  XML_ParserFree(parser);
  return result;
}



 