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

#include "netd.h"
#include <string.h>
#include <bsdxml.h>
#include <libnetconf2/netconf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Extract message-id from NETCONF request
 * @param request XML request string
 * @return message-id string or NULL if not found
 */
static char *extract_message_id(const char *request)
{
    const char *start = strstr(request, "message-id=\"");
    if (!start) {
        return NULL;
    }
    start += 12; // Skip "message-id=\""
    
    const char *end = strchr(start, '"');
    if (!end) {
        return NULL;
    }
    
    size_t len = end - start;
    char *msg_id = malloc(len + 1);
    if (!msg_id) {
        return NULL;
    }
    
    strncpy(msg_id, start, len);
    msg_id[len] = '\0';
    return msg_id;
}

/**
 * Check if request is a get operation for interfaces
 * @param request XML request string
 * @return true if get interfaces request, false otherwise
 */
static bool is_get_interfaces_request(const char *request)
{
    return strstr(request, "<get>") && strstr(request, "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\"/>");
}

/**
 * Check if request is a get operation for VRFs
 * @param request XML request string
 * @return true if get VRFs request, false otherwise
 */
static bool is_get_vrfs_request(const char *request)
{
    return strstr(request, "<get>") && strstr(request, "<network-instances xmlns=\"urn:ietf:params:xml:ns:yang:ietf-network-instance\"/>");
}

/**
 * Check if request is a get operation for routes
 * @param request XML request string
 * @return true if get routes request, false otherwise
 */
static bool is_get_routes_request(const char *request)
{
    return strstr(request, "<get>") && strstr(request, "<network-instances xmlns=\"urn:ietf:params:xml:ns:yang:ietf-network-instance\"/>");
}

/**
 * Extract FIB number from routes request
 * @param request XML request string
 * @return FIB number, 0 if not found or invalid
 */
static uint32_t extract_fib_from_request(const char *request)
{
    const char *fib_start = strstr(request, "<fib>");
    if (fib_start) {
        fib_start += 5; /* Skip "<fib>" */
        const char *fib_end = strstr(fib_start, "</fib>");
        if (fib_end) {
            char fib_str[16];
            size_t len = fib_end - fib_start;
            if (len < sizeof(fib_str)) {
                strncpy(fib_str, fib_start, len);
                fib_str[len] = '\0';
                return (uint32_t)atoi(fib_str);
            }
        }
    }
    return 0; /* Default FIB */
}

/**
 * Check if request is a save operation
 * @param request XML request string
 * @return true if save request, false otherwise
 */
static bool is_save_request(const char *request)
{
    return strstr(request, "<save xmlns=\"urn:ietf:params:xml:ns:yang:netd\">");
}

/**
 * Generate NETCONF success response
 * @param message_id Message ID from request
 * @param data Response data XML
 * @return Allocated response string
 */
static char *generate_success_response(const char *message_id, const char *data)
{
    char *response;
    if (data) {
        asprintf(&response, 
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" message-id=\"%s\">\n"
                "  <data>\n"
                "%s\n"
                "  </data>\n"
                "</rpc-reply>", 
                message_id, data);
    } else {
        asprintf(&response,
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" message-id=\"%s\">\n"
                "  <ok/>\n"
                "</rpc-reply>",
                message_id);
    }
    return response;
}

/**
 * Generate NETCONF error response
 * @param message_id Message ID from request
 * @param error_type Error type
 * @param error_tag Error tag
 * @param error_message Error message
 * @return Allocated response string
 */
static char *generate_error_response(const char *message_id, const char *error_type, 
                                   const char *error_tag, const char *error_message)
{
    char *response;
    asprintf(&response,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" message-id=\"%s\">\n"
            "  <rpc-error>\n"
            "    <error-type>%s</error-type>\n"
            "    <error-tag>%s</error-tag>\n"
            "    <error-severity>error</error-severity>\n"
            "    <error-message>%s</error-message>\n"
            "  </rpc-error>\n"
            "</rpc-reply>",
            message_id, error_type, error_tag, error_message);
    return response;
}

/**
 * Handle netconf request
 * @param state Server state
 * @param request Request XML string
 * @param response Response XML string (allocated)
 * @return 0 on success, -1 on failure
 */
int netconf_handle_request(netd_state_t *state, const char *request, char **response)
{
    if (!state || !request || !response) {
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Handling NETCONF request");

    /* Validate incoming request against YANG schema if YANG context is available */
    if (state->yang_ctx) {
        if (yang_validate_rpc(state, request) < 0) {
            debug_log(DEBUG_WARN, "NETCONF RPC failed YANG validation, but processing anyway");
            /* Don't fail the request, just log a warning for now */
        } else {
            debug_log(DEBUG_DEBUG, "NETCONF RPC validated successfully against YANG schema");
        }
    }

    /* Extract message ID */
    char *message_id = extract_message_id(request);
    if (!message_id) {
        debug_log(DEBUG_ERROR, "Failed to extract message ID from request");
        *response = generate_error_response("1", "protocol", "malformed-message", "Missing message-id");
        return 0;
    }

    debug_log(DEBUG_DEBUG, "Request message-id: %s", message_id);

    /* Handle different request types */
    if (is_get_interfaces_request(request)) {
        debug_log(DEBUG_DEBUG, "Handling get-interfaces request");
        char *interfaces_data = interface_get_all(state);
        if (interfaces_data) {
            *response = generate_success_response(message_id, interfaces_data);
            free(interfaces_data);
        } else {
            *response = generate_error_response(message_id, "application", "operation-failed", "Failed to get interfaces");
        }
    } else if (is_get_vrfs_request(request)) {
        debug_log(DEBUG_DEBUG, "Handling get-vrfs request");
        char *vrfs_data = vrf_get_all(state);
        if (vrfs_data) {
            *response = generate_success_response(message_id, vrfs_data);
            free(vrfs_data);
        } else {
            *response = generate_error_response(message_id, "application", "operation-failed", "Failed to get VRFs");
        }
    } else if (is_get_routes_request(request)) {
        debug_log(DEBUG_DEBUG, "Handling get-routes request");
        uint32_t fib = extract_fib_from_request(request);
        debug_log(DEBUG_DEBUG, "Extracted FIB: %u", fib);
        
        /* Clear existing routes and enumerate for the specific FIB */
        route_clear_all(state);
        if (route_enumerate_system(state, fib) < 0) {
            debug_log(DEBUG_ERROR, "Failed to enumerate routes for FIB %u", fib);
            *response = generate_error_response(message_id, "application", "operation-failed", "Failed to get routes");
        } else {
            char *routes_data = route_get_all(state);
            if (routes_data) {
                *response = generate_success_response(message_id, routes_data);
                free(routes_data);
            } else {
                *response = generate_error_response(message_id, "application", "operation-failed", "Failed to get routes");
            }
        }
    } else if (is_save_request(request)) {
        debug_log(DEBUG_DEBUG, "Handling save request");
        if (config_save(state) == 0) {
            *response = generate_success_response(message_id, NULL);
        } else {
            *response = generate_error_response(message_id, "application", "operation-failed", "Failed to save configuration");
        }
    } else {
        debug_log(DEBUG_DEBUG, "Unknown request type");
        *response = generate_error_response(message_id, "application", "operation-not-supported", "Request not supported");
    }

    free(message_id);

    if (!*response) {
        debug_log(DEBUG_ERROR, "Failed to generate response");
        return -1;
    }

    return 0;
} 