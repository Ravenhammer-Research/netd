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
#include <interface/interface.h>
#include <libnetconf2/netconf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Main NETCONF request handler
 * @param state Server state
 * @param request XML request string
 * @param response Response string (allocated and returned)
 * @return 0 on success, -1 on failure
 */
int netconf_handle_request(netd_state_t *state, const char *request,
                          char **response) {
  char *message_id;
  int ret = -1;

  if (!state || !request || !response) {
    debug_log(DEBUG_ERROR, "Invalid parameters for NETCONF request handling");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Handling NETCONF request: %s", request);

  /* Extract message ID */
  message_id = extract_message_id(request);
  if (!message_id) {
    debug_log(DEBUG_ERROR, "Failed to extract message ID from request");
    return -1;
  }

  /* Handle different request types */
  if (is_edit_config_request(request)) {
    ret = handle_edit_config(state, request, message_id, response);
  } else if (is_commit_request(request)) {
    ret = handle_commit_request(state, request, message_id, response);
  } else if (is_save_request(request)) {
    ret = handle_save_request(state, request, message_id, response);
  } else if (is_get_interfaces_request(request)) {
    ret = handle_get_interfaces_request(state, request, message_id, response);
  } else if (is_get_vrfs_request(request)) {
    ret = handle_get_vrfs_request(state, request, message_id, response);
  } else if (is_get_routes_request(request)) {
    ret = handle_get_routes_request(state, request, message_id, response);
  } else if (is_get_vrf_routes_request(request)) {
    ret = handle_get_vrf_routes_request(state, request, message_id, response);
  } else {
    debug_log(DEBUG_ERROR, "Unknown or unsupported NETCONF request type");
    *response = create_error_response(message_id, "operation-not-supported",
                               "Request type not supported");
    ret = -1;
  }

  free(message_id);
  return ret;
}

/**
 * Handle commit request
 * @param state Server state
 * @param request XML request string
 * @param message_id Message ID
 * @param response Response string
 * @return 0 on success, -1 on failure
 */
int handle_commit_request(netd_state_t *state, const char *request,
                                const char *message_id, char **response) {
  int ret;

  debug_log(DEBUG_INFO, "Handling commit request for request: %s", request ? request : "NULL");

  if (!state->transaction_active) {
    debug_log(DEBUG_ERROR, "No active transaction to commit");
    *response = create_error_response(message_id, "operation-failed",
                               "No active transaction to commit");
    return -1;
  }

  ret = transaction_commit(state);
  if (ret < 0) {
    debug_log(DEBUG_ERROR, "Failed to commit transaction");
    *response = create_error_response(message_id, "operation-failed",
                               "Failed to commit transaction");
    return -1;
  }

  /* Create success response */
  *response = create_success_response(message_id);
  return 0;
}

/**
 * Handle save request
 * @param state Server state
 * @param request XML request string
 * @param message_id Message ID
 * @param response Response string
 * @return 0 on success, -1 on failure
 */
int handle_save_request(netd_state_t *state, const char *request,
                               const char *message_id, char **response) {
  int ret;

  debug_log(DEBUG_INFO, "Handling save request for request: %s", request ? request : "NULL");

  ret = config_save(state);
  if (ret < 0) {
    debug_log(DEBUG_ERROR, "Failed to save configuration");
    *response = create_error_response(message_id, "operation-failed",
                               "Failed to save configuration");
    return -1;
  }

  /* Create success response */
  *response = create_success_response(message_id);
  return 0;
}

/**
 * Handle get-interfaces request
 * @param state Server state
 * @param request XML request string
 * @param message_id Message ID
 * @param response Response string
 * @return 0 on success, -1 on failure
 */
int handle_get_interfaces_request(netd_state_t *state, const char *request,
                                         const char *message_id, char **response) {
  char *xml_response;
  int ret;

  debug_log(DEBUG_INFO, "Handling get-interfaces request for request: %s", request ? request : "NULL");

  ret = interface_list(state, IF_TYPE_UNKNOWN);
  if (ret < 0) {
    debug_log(DEBUG_ERROR, "Failed to list interfaces");
    *response = create_error_response(message_id, "operation-failed",
                               "Failed to list interfaces");
    return -1;
  }

  /* Create XML response with interface data */
  xml_response = create_interfaces_xml_response(state, message_id);
  if (!xml_response) {
    debug_log(DEBUG_ERROR, "Failed to create interfaces XML response");
    return -1;
  }

  *response = xml_response;
  return 0;
}

/**
 * Handle get-vrfs request
 * @param state Server state
 * @param request XML request string
 * @param message_id Message ID
 * @param response Response string
 * @return 0 on success, -1 on failure
 */
int handle_get_vrfs_request(netd_state_t *state, const char *request,
                                   const char *message_id, char **response) {
  char *xml_response;
  int ret;

  debug_log(DEBUG_INFO, "Handling get-vrfs request for request: %s", request ? request : "NULL");

  ret = vrf_list(state);
  if (ret < 0) {
    debug_log(DEBUG_ERROR, "Failed to list VRFs");
    *response = create_error_response(message_id, "operation-failed",
                               "Failed to list VRFs");
    return -1;
  }

  /* Create XML response with VRF data */
  xml_response = create_vrfs_xml_response(state, message_id);
  if (!xml_response) {
    debug_log(DEBUG_ERROR, "Failed to create VRFs XML response");
    return -1;
  }

  *response = xml_response;
  return 0;
}

/**
 * Handle get-routes request
 * @param state Server state
 * @param request XML request string
 * @param message_id Message ID
 * @param response Response string
 * @return 0 on success, -1 on failure
 */
int handle_get_routes_request(netd_state_t *state, const char *request,
                                     const char *message_id, char **response) {
  char *xml_response;
  int ret;
  uint32_t fib = 0;

  debug_log(DEBUG_INFO, "Handling get-routes request");

  /* Extract FIB from request if specified */
  fib = extract_fib_from_request(request);

  ret = route_list(state, fib, AF_UNSPEC);
  if (ret < 0) {
    debug_log(DEBUG_ERROR, "Failed to list routes");
    *response = create_error_response(message_id, "operation-failed",
                               "Failed to list routes");
    return -1;
  }

  /* Create XML response with route data */
  xml_response = create_routes_xml_response(state, message_id, fib);
  if (!xml_response) {
    debug_log(DEBUG_ERROR, "Failed to create routes XML response");
    return -1;
  }

  *response = xml_response;
  return 0;
}

/**
 * Handle get-vrf-routes request
 * @param state Server state
 * @param request XML request string
 * @param message_id Message ID
 * @param response Response string
 * @return 0 on success, -1 on failure
 */
int handle_get_vrf_routes_request(netd_state_t *state, const char *request,
                                         const char *message_id, char **response) {
  char *xml_response;
  char *vrf_name;
  int ret;

  debug_log(DEBUG_INFO, "Handling get-vrf-routes request");

  /* Extract VRF name from request */
  vrf_name = extract_vrf_name_from_request(request);
  if (!vrf_name) {
    debug_log(DEBUG_ERROR, "Failed to extract VRF name from request");
    *response = create_error_response(message_id, "operation-failed",
                               "Failed to extract VRF name");
    return -1;
  }

  /* Find VRF and get its FIB */
  vrf_t *vrf = vrf_find_by_name(state, vrf_name);
  if (!vrf) {
    debug_log(DEBUG_ERROR, "VRF %s not found", vrf_name);
    free(vrf_name);
    *response = create_error_response(message_id, "operation-failed",
                               "VRF not found");
    return -1;
  }

  ret = route_list(state, vrf->fib_number, AF_UNSPEC);
  free(vrf_name);

  if (ret < 0) {
    debug_log(DEBUG_ERROR, "Failed to list VRF routes");
    *response = create_error_response(message_id, "operation-failed",
                               "Failed to list VRF routes");
    return -1;
  }

  /* Create XML response with VRF route data */
  xml_response = create_vrf_routes_xml_response(state, message_id, vrf);
  if (!xml_response) {
    debug_log(DEBUG_ERROR, "Failed to create VRF routes XML response");
    return -1;
  }

  *response = xml_response;
  return 0;
}

 