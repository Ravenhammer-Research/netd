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

#include <netconf.h>
#include <xml.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Handle edit-config request
 * @param state Server state
 * @param request XML request string
 * @param message_id Message ID
 * @param response Response string
 * @return 0 on success, -1 on failure
 */
int handle_edit_config(netd_state_t *state, const char *request,
                       const char *message_id, char **response) {
  int ret = 0;

  debug_log(INFO, "Handling edit-config request");

  if (!state || !request || !message_id || !response) {
    debug_log(ERROR, "Invalid parameters for edit-config handling");
    return -1;
  }

  /* Validate configuration data against YANG schema */
  if (yang_validate_config(state, request) < 0) {
    debug_log(ERROR, "Configuration validation failed");
    *response = create_error_response(message_id, "invalid-value", "Configuration failed YANG validation");
    return -1;
  }

  /* Use XML utilities to parse and process the edit-config request */
  ret = process_edit_config_request(state, request);
  if (ret < 0) {
    debug_log(ERROR, "Failed to process edit-config request");
  }

  if (ret == 0) {
    /* Create success response */
    *response = create_success_response(message_id);
  } else {
    /* Create error response */
    *response = create_error_response(message_id, "operation-failed",
                                     "Failed to process edit-config");
  }

  return ret;
} 