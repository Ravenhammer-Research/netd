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

/* Callback data structures */
struct message_id_data {
    char *message_id;
    bool found;
};

struct request_check_data {
    bool is_get;
    bool has_target;
};

struct fib_data {
    uint32_t fib;
    bool found;
};

/* Callback for extracting message-id */
static void message_id_start_element(void *userData, const XML_Char *name, const XML_Char **atts)
{
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
static char *extract_message_id(const char *request)
{
    XML_Parser parser = XML_ParserCreate(NULL);
    char *msg_id = NULL;
    
    if (!parser) {
        return NULL;
    }
    
    /* Set up user data to capture message-id */
    struct message_id_data user_data = { NULL, false };
    
    XML_SetUserData(parser, &user_data);
    XML_SetElementHandler(parser, message_id_start_element, NULL);
    
    /* Parse the XML */
    if (XML_Parse(parser, request, strlen(request), 1) == XML_STATUS_OK) {
        if (user_data.found) {
            msg_id = user_data.message_id;
        }
    }
    
    XML_ParserFree(parser);
    return msg_id;
}

/* Callback for checking interfaces request */
static void interfaces_start_element(void *userData, const XML_Char *name, const XML_Char **atts)
{
    struct request_check_data *data = (struct request_check_data *)userData;
    
    if (strcmp(name, "get") == 0) {
        data->is_get = true;
    } else if (strcmp(name, "interfaces") == 0) {
        /* Check for the correct namespace */
        for (int i = 0; atts[i] != NULL; i += 2) {
            if (strcmp(atts[i], "xmlns") == 0 && 
                strcmp(atts[i + 1], "urn:ietf:params:xml:ns:yang:ietf-interfaces") == 0) {
                data->has_target = true;
                break;
            }
        }
    }
}

/**
 * Check if request is a get operation for interfaces
 * @param request XML request string
 * @return true if get interfaces request, false otherwise
 */
static bool is_get_interfaces_request(const char *request)
{
    XML_Parser parser = XML_ParserCreate(NULL);
    bool is_get = false;
    
    if (!parser) {
        return false;
    }
    
    /* Set up user data to track what we find */
    struct request_check_data user_data = { false, false };
    
    XML_SetUserData(parser, &user_data);
    XML_SetElementHandler(parser, interfaces_start_element, NULL);
    
    /* Parse the XML */
    if (XML_Parse(parser, request, strlen(request), 1) == XML_STATUS_OK) {
        is_get = user_data.is_get && user_data.has_target;
    }
    
    XML_ParserFree(parser);
    return is_get;
}

/* Callback for checking VRFs request */
static void vrfs_start_element(void *userData, const XML_Char *name, const XML_Char **atts)
{
    struct request_check_data *data = (struct request_check_data *)userData;
    
    if (strcmp(name, "get") == 0) {
        data->is_get = true;
    } else if (strcmp(name, "lib") == 0) {
        /* Check for the correct namespace */
        for (int i = 0; atts[i] != NULL; i += 2) {
            if (strcmp(atts[i], "xmlns") == 0 && 
                strcmp(atts[i + 1], "http://frrouting.org/yang/vrf") == 0) {
                data->has_target = true;
                break;
            }
        }
    }
}

/**
 * Check if request is a get operation for VRFs
 * @param request XML request string
 * @return true if get VRFs request, false otherwise
 */
static bool is_get_vrfs_request(const char *request)
{
    XML_Parser parser = XML_ParserCreate(NULL);
    bool is_get = false;
    
    if (!parser) {
        return false;
    }
    
    /* Set up user data to track what we find */
    struct request_check_data user_data = { false, false };
    
    XML_SetUserData(parser, &user_data);
    XML_SetElementHandler(parser, vrfs_start_element, NULL);
    
    /* Parse the XML */
    if (XML_Parse(parser, request, strlen(request), 1) == XML_STATUS_OK) {
        is_get = user_data.is_get && user_data.has_target;
    }
    
    XML_ParserFree(parser);
    return is_get;
}

/* Callback for checking routes request */
static void routes_start_element(void *userData, const XML_Char *name, const XML_Char **atts)
{
    struct request_check_data *data = (struct request_check_data *)userData;
    
    if (strcmp(name, "get") == 0) {
        data->is_get = true;
    } else if (strcmp(name, "routing") == 0) {
        /* Check for the correct namespace */
        for (int i = 0; atts[i] != NULL; i += 2) {
            if (strcmp(atts[i], "xmlns") == 0 && 
                strcmp(atts[i + 1], "urn:ietf:params:xml:ns:yang:ietf-routing") == 0) {
                data->has_target = true;
                break;
            }
        }
    }
}

/**
 * Check if request is a get operation for routes
 * @param request XML request string
 * @return true if get routes request, false otherwise
 */
static bool is_get_routes_request(const char *request)
{
    XML_Parser parser = XML_ParserCreate(NULL);
    bool is_get = false;
    
    if (!parser) {
        return false;
    }
    
    /* Set up user data to track what we find */
    struct request_check_data user_data = { false, false };
    
    XML_SetUserData(parser, &user_data);
    XML_SetElementHandler(parser, routes_start_element, NULL);
    
    /* Parse the XML */
    if (XML_Parse(parser, request, strlen(request), 1) == XML_STATUS_OK) {
        is_get = user_data.is_get && user_data.has_target;
    }
    
    XML_ParserFree(parser);
    return is_get;
}

/* Callback for extracting FIB value */
static void fib_start_element(void *userData, const XML_Char *name, const XML_Char **atts)
{
    struct fib_data *data = (struct fib_data *)userData;
    (void)atts; /* Suppress unused parameter warning */
    
    if (strcmp(name, "fib") == 0) {
        data->found = true;
    }
}

static void fib_end_element(void *userData, const XML_Char *name)
{
    struct fib_data *data = (struct fib_data *)userData;
    
    if (strcmp(name, "fib") == 0) {
        data->found = false;
    }
}

static void fib_character_data(void *userData, const XML_Char *s, int len)
{
    struct fib_data *data = (struct fib_data *)userData;
    
    if (data->found) {
        char fib_str[16];
        if (len < (int)(sizeof(fib_str) - 1)) {
            strncpy(fib_str, s, len);
            fib_str[len] = '\0';
            data->fib = (uint32_t)atoi(fib_str);
        }
    }
}

/**
 * Extract FIB number from routes request
 * @param request XML request string
 * @return FIB number, 0 if not found or invalid
 */
static uint32_t extract_fib_from_request(const char *request)
{
    XML_Parser parser = XML_ParserCreate(NULL);
    uint32_t fib = 0;
    
    if (!parser) {
        return 0;
    }
    
    /* Set up user data to capture FIB value */
    struct fib_data user_data = { 0, false };
    
    XML_SetUserData(parser, &user_data);
    XML_SetElementHandler(parser, fib_start_element, fib_end_element);
    XML_SetCharacterDataHandler(parser, fib_character_data);
    
    /* Parse the XML */
    if (XML_Parse(parser, request, strlen(request), 1) == XML_STATUS_OK) {
        fib = user_data.fib;
    }
    
    XML_ParserFree(parser);
    return fib;
}

/* Callback for checking save request */
static void save_start_element(void *userData, const XML_Char *name, const XML_Char **atts)
{
    struct request_check_data *data = (struct request_check_data *)userData;
    
    if (strcmp(name, "save") == 0) {
        /* Check for the correct namespace */
        for (int i = 0; atts[i] != NULL; i += 2) {
            if (strcmp(atts[i], "xmlns") == 0 && 
                strcmp(atts[i + 1], "urn:ietf:params:xml:ns:yang:netd") == 0) {
                data->has_target = true;
                break;
            }
        }
    }
}

/**
 * Check if request is a save operation
 * @param request XML request string
 * @return true if save request, false otherwise
 */
static bool is_save_request(const char *request)
{
    XML_Parser parser = XML_ParserCreate(NULL);
    bool is_save = false;
    
    if (!parser) {
        return false;
    }
    
    /* Set up user data to track what we find */
    struct request_check_data user_data = { false, false };
    
    XML_SetUserData(parser, &user_data);
    XML_SetElementHandler(parser, save_start_element, NULL);
    
    /* Parse the XML */
    if (XML_Parse(parser, request, strlen(request), 1) == XML_STATUS_OK) {
        is_save = user_data.has_target;
    }
    
    XML_ParserFree(parser);
    return is_save;
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
                "  <data xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
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
        debug_log(DEBUG_ERROR, "Invalid parameters for NETCONF request handling");
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Starting NETCONF request processing");

    /* Validate incoming request against YANG schema if YANG context is available */
    if (state->yang_ctx) {
        debug_log(DEBUG_DEBUG, "Validating NETCONF RPC against YANG schema");
        if (yang_validate_rpc(state, request) < 0) {
            debug_log(DEBUG_WARN, "NETCONF RPC failed YANG validation, but processing anyway");
            /* Don't fail the request, just log a warning for now */
        } else {
            debug_log(DEBUG_DEBUG, "NETCONF RPC validated successfully against YANG schema");
        }
    } else {
        debug_log(DEBUG_DEBUG, "No YANG context available, skipping RPC validation");
    }

    /* Extract message ID */
    debug_log(DEBUG_DEBUG, "Extracting message ID from request");
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
            debug_log(DEBUG_DEBUG, "Generated interfaces data (%zu bytes)", strlen(interfaces_data));
            *response = generate_success_response(message_id, interfaces_data);
            free(interfaces_data);
            debug_log(DEBUG_INFO, "Successfully processed get-interfaces request");
        } else {
            debug_log(DEBUG_ERROR, "Failed to generate interfaces data");
            *response = generate_error_response(message_id, "application", "operation-failed", "Failed to get interfaces");
        }
    } else if (is_get_vrfs_request(request)) {
        debug_log(DEBUG_DEBUG, "Handling get-vrfs request");
        char *vrfs_data = vrf_get_all(state);
        if (vrfs_data) {
            debug_log(DEBUG_DEBUG, "Generated VRFs data (%zu bytes)", strlen(vrfs_data));
            *response = generate_success_response(message_id, vrfs_data);
            free(vrfs_data);
            debug_log(DEBUG_INFO, "Successfully processed get-vrfs request");
        } else {
            debug_log(DEBUG_ERROR, "Failed to generate VRFs data");
            *response = generate_error_response(message_id, "application", "operation-failed", "Failed to get VRFs");
        }
    } else if (is_get_routes_request(request)) {
        debug_log(DEBUG_DEBUG, "Handling get-routes request");
        uint32_t fib = extract_fib_from_request(request);
        debug_log(DEBUG_DEBUG, "Extracted FIB: %u", fib);
        
        /* Clear existing routes and enumerate for the specific FIB */
        debug_log(DEBUG_DEBUG, "Clearing existing routes from state");
        route_clear_all(state);
        
        debug_log(DEBUG_DEBUG, "Enumerating system routes for FIB %u", fib);
        if (route_enumerate_system(state, fib) < 0) {
            debug_log(DEBUG_ERROR, "Failed to enumerate routes for FIB %u", fib);
            *response = generate_error_response(message_id, "application", "operation-failed", "Failed to get routes");
        } else {
            debug_log(DEBUG_DEBUG, "Successfully enumerated routes for FIB %u", fib);
            char *routes_data = route_get_all(state);
            if (routes_data) {
                debug_log(DEBUG_DEBUG, "Generated routes data (%zu bytes)", strlen(routes_data));
                *response = generate_success_response(message_id, routes_data);
                free(routes_data);
                debug_log(DEBUG_INFO, "Successfully processed get-routes request for FIB %u", fib);
            } else {
                debug_log(DEBUG_ERROR, "Failed to generate routes data");
                *response = generate_error_response(message_id, "application", "operation-failed", "Failed to get routes");
            }
        }
    } else if (is_save_request(request)) {
        debug_log(DEBUG_DEBUG, "Handling save request");
        if (config_save(state) == 0) {
            *response = generate_success_response(message_id, NULL);
            debug_log(DEBUG_INFO, "Successfully processed save request");
        } else {
            debug_log(DEBUG_ERROR, "Failed to save configuration");
            *response = generate_error_response(message_id, "application", "operation-failed", "Failed to save configuration");
        }
    } else {
        debug_log(DEBUG_WARN, "Unknown or unsupported request type");
        *response = generate_error_response(message_id, "application", "operation-not-supported", "Request not supported");
    }

    free(message_id);

    if (!*response) {
        debug_log(DEBUG_ERROR, "Failed to generate response for NETCONF request");
        return -1;
    }

    debug_log(DEBUG_DEBUG, "NETCONF request processing completed successfully");
    return 0;
} 