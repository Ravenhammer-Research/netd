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

#include <netd.h>
#include <netconf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Create a success response
 * @param message_id Message ID for the response
 * @return Success response string (allocated)
 */
char *create_success_response(const char *message_id) {
  char *response;

  if (!message_id) {
    return NULL;
  }

  response = malloc(NETCONF_RESPONSE_BUFFER_SIZE);
  if (!response) {
    debug_log(ERROR, "Failed to allocate memory for success response");
    return NULL;
  }

  int result = prepare_response(response,
                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                 "message-id=\"%s\">\n"
                 "  <ok/>\n"
                 "</rpc-reply>",
                 message_id);

  if (result == -1) {
    debug_log(ERROR, "Response too big");
    free(response);
    return NULL;
  }

  return response;
}

/**
 * Create an error response
 * @param message_id Message ID for the response
 * @param error_type Type of error
 * @param error_message Error message
 * @return Error response string (allocated)
 */
char *create_error_response(const char *message_id, const char *error_type,
                            const char *error_message) {
  char *response;

  if (!message_id || !error_type || !error_message) {
    return NULL;
  }

  response = malloc(NETCONF_RESPONSE_BUFFER_SIZE);
  if (!response) {
    debug_log(ERROR, "Failed to allocate memory for error response");
    return NULL;
  }

  int result = prepare_response(response,
                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                 "message-id=\"%s\">\n"
                 "  <rpc-error>\n"
                 "    <error-type>%s</error-type>\n"
                 "    <error-tag>operation-failed</error-tag>\n"
                 "    <error-severity>error</error-severity>\n"
                 "    <error-message>%s</error-message>\n"
                 "  </rpc-error>\n"
                 "</rpc-reply>",
                 message_id, error_type, error_message);

  if (result == -1) {
    debug_log(ERROR, "Response too big");
    free(response);
    return NULL;
  }

  return response;
}







 