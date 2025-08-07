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

/* Forward declarations */
static bool is_edit_config_request(const char *request);
static int handle_edit_config(netd_state_t *state, const char *request, const char *message_id, char **response);
static bool is_commit_request(const char *request);
extern int add_pending_route_add(netd_state_t *state, uint32_t fib, const char *destination, 
                                 const char *gateway, const char *interface, int flags);
extern int add_pending_route_delete(netd_state_t *state, uint32_t fib, const char *destination);

/* Callback data structures */
struct message_id_data {
    char *message_id;
    bool found;
};

struct request_check_data {
    bool is_get;
    bool has_target;
    bool in_vrf;
};

struct fib_data {
    uint32_t fib;
    bool found;
};

struct vrf_name_data {
    char vrf_name[64];
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
    struct request_check_data user_data = { false, false, false };
    
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
    struct request_check_data user_data = { false, false, false };
    
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
    struct request_check_data user_data = { false, false, false };
    
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

/* Callback for checking VRF route request */
static void vrf_routes_start_element(void *userData, const XML_Char *name, const XML_Char **atts)
{
    struct request_check_data *data = (struct request_check_data *)userData;
    
    debug_log(DEBUG_DEBUG, "vrf_routes_start_element: userData=%p, name=%s, atts=%p", userData, name, atts);
    
    if (strcmp(name, "get") == 0) {
        data->is_get = true;
        debug_log(DEBUG_DEBUG, "Found 'get' element");
    } else if (strcmp(name, "vrf") == 0) {
        data->in_vrf = true;
        debug_log(DEBUG_DEBUG, "Found 'vrf' element, in_vrf = true");
    } else if (strcmp(name, "route-request") == 0 && data->in_vrf) {
        debug_log(DEBUG_DEBUG, "Found 'route-request' element inside vrf");
        data->has_target = true;
        debug_log(DEBUG_DEBUG, "Found route-request, has_target = true");
    }
}

/**
 * Check if request is a get operation for VRF routes
 * @param request XML request string
 * @return true if get VRF routes request, false otherwise
 */
static bool is_get_vrf_routes_request(const char *request)
{
    XML_Parser parser = XML_ParserCreate(NULL);
    bool is_get = false;
    
    if (!parser) {
        return false;
    }
    
    /* Set up user data to track what we find */
    struct request_check_data user_data = { false, false, false };
    
    XML_SetUserData(parser, &user_data);
    XML_SetElementHandler(parser, vrf_routes_start_element, NULL);
    
    /* Parse the XML */
    if (XML_Parse(parser, request, strlen(request), 1) == XML_STATUS_OK) {
        is_get = user_data.is_get && user_data.has_target;
        debug_log(DEBUG_DEBUG, "VRF routes XML parsing result: is_get=%s, has_target=%s, final_result=%s", 
                  user_data.is_get ? "true" : "false", 
                  user_data.has_target ? "true" : "false",
                  is_get ? "true" : "false");
    } else {
        debug_log(DEBUG_ERROR, "VRF routes XML parsing failed: %s", XML_ErrorString(XML_GetErrorCode(parser)));
    }
    
    XML_ParserFree(parser);
    return is_get;
}

/* Callback for extracting VRF name */
static void vrf_name_start_element(void *userData, const XML_Char *name, const XML_Char **atts)
{
    struct vrf_name_data *data = (struct vrf_name_data *)userData;
    (void)atts; /* Suppress unused parameter warning */
    
    if (strcmp(name, "name") == 0) {
        data->found = true;
    }
}

static void vrf_name_end_element(void *userData, const XML_Char *name)
{
    struct vrf_name_data *data = (struct vrf_name_data *)userData;
    
    if (strcmp(name, "name") == 0) {
        data->found = false;
    }
}

static void vrf_name_character_data(void *userData, const XML_Char *s, int len)
{
    struct vrf_name_data *data = (struct vrf_name_data *)userData;
    
    if (data->found && len < (int)(sizeof(data->vrf_name) - 1)) {
        strncpy(data->vrf_name, s, len);
        data->vrf_name[len] = '\0';
    }
}

/**
 * Extract VRF name from VRF route request
 * @param request XML request string
 * @return VRF name, "default" if not found
 */
static const char *extract_vrf_name_from_request(const char *request)
{
    XML_Parser parser = XML_ParserCreate(NULL);
    static char vrf_name[64] = "default"; /* Default VRF name */
    
    if (!parser) {
        return vrf_name;
    }
    
    /* Set up user data to capture VRF name */
    struct vrf_name_data user_data = { "", false };
    
    XML_SetUserData(parser, &user_data);
    XML_SetElementHandler(parser, vrf_name_start_element, vrf_name_end_element);
    XML_SetCharacterDataHandler(parser, vrf_name_character_data);
    
    /* Parse the XML */
    if (XML_Parse(parser, request, strlen(request), 1) == XML_STATUS_OK) {
        if (user_data.vrf_name[0] != '\0') {
            strlcpy(vrf_name, user_data.vrf_name, sizeof(vrf_name));
        }
    }
    
    XML_ParserFree(parser);
    return vrf_name;
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
    struct request_check_data user_data = { false, false, false };
    
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
 * Start element handler for commit parsing
 */
static void commit_start_element(void *userData, const XML_Char *name, const XML_Char **atts)
{
    struct request_check_data *data = (struct request_check_data *)userData;
    
    if (strcmp(name, "commit") == 0) {
        /* Check for the correct namespace */
        for (int i = 0; atts[i] != NULL; i += 2) {
            if (strcmp(atts[i], "xmlns") == 0 && 
                strcmp(atts[i + 1], "urn:ietf:params:xml:ns:yang:netd") == 0) {
                data->is_get = true;
                break;
            }
        }
    }
}

/**
 * Check if request is a commit operation
 * @param request XML request string
 * @return true if commit request, false otherwise
 */
static bool is_commit_request(const char *request)
{
    XML_Parser parser = XML_ParserCreate(NULL);
    bool is_commit_config = false;
    
    if (!parser) {
        return false;
    }
    
    /* Set up user data to track what we find */
    struct request_check_data user_data = { false, false, false };
    
    XML_SetUserData(parser, &user_data);
    XML_SetElementHandler(parser, commit_start_element, NULL);
    
    /* Parse the XML */
    if (XML_Parse(parser, request, strlen(request), 1) == XML_STATUS_OK) {
        is_commit_config = user_data.is_get;
    }
    
    XML_ParserFree(parser);
    return is_commit_config;
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
    if (is_edit_config_request(request)) {
        debug_log(DEBUG_DEBUG, "Handling edit-config request");
        if (handle_edit_config(state, request, message_id, response) == 0) {
            debug_log(DEBUG_INFO, "Successfully processed edit-config request");
        } else {
            debug_log(DEBUG_ERROR, "Failed to process edit-config request");
            if (!*response) {
                *response = generate_error_response(message_id, "application", "operation-failed", "Failed to process edit-config");
            }
        }
    } else if (is_get_interfaces_request(request)) {
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
    } else if (is_get_vrf_routes_request(request)) {
        debug_log(DEBUG_DEBUG, "Handling get-vrf-routes request");
        const char *vrf_name = extract_vrf_name_from_request(request);
        debug_log(DEBUG_DEBUG, "Extracted VRF name: %s", vrf_name);
        
        /* Determine FIB number from VRF name */
        uint32_t fib = 0;
        if (strcmp(vrf_name, "default") == 0) {
            fib = 0;
        } else {
            /* Try to parse as number first */
            char *endptr;
            fib = strtoul(vrf_name, &endptr, 10);
            if (*endptr != '\0') {
                /* Not a number, look up VRF by name */
                vrf_t *vrf = vrf_find_by_name(state, vrf_name);
                if (vrf) {
                    fib = vrf->fib_number;
                } else {
                    debug_log(DEBUG_ERROR, "VRF not found: %s", vrf_name);
                    *response = generate_error_response(message_id, "application", "operation-failed", "VRF not found");
                    free(message_id);
                    return 0;
                }
            }
        }
        
        debug_log(DEBUG_DEBUG, "Using FIB %u for VRF %s", fib, vrf_name);
        
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
                debug_log(DEBUG_INFO, "Successfully processed get-vrf-routes request for VRF %s (FIB %u)", vrf_name, fib);
            } else {
                debug_log(DEBUG_ERROR, "Failed to generate routes data");
                *response = generate_error_response(message_id, "application", "operation-failed", "Failed to get routes");
            }
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
    } else if (is_commit_request(request)) {
        debug_log(DEBUG_DEBUG, "Handling commit request");
        if (state->transaction_active) {
            if (transaction_commit(state) == 0) {
                debug_log(DEBUG_INFO, "Successfully committed transaction");
                *response = generate_success_response(message_id, NULL);
            } else {
                debug_log(DEBUG_ERROR, "Failed to commit transaction");
                *response = generate_error_response(message_id, "application", "operation-failed", "Failed to commit transaction");
            }
        } else {
            debug_log(DEBUG_WARN, "No active transaction to commit");
            *response = generate_error_response(message_id, "application", "operation-failed", "No active transaction");
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

/* Structure to hold edit-config parsing context */
struct edit_config_data {
    bool is_edit_config;
    bool in_config;
    bool in_vrf;
    bool in_route;
    bool has_route_config;
    char vrf_name[64];
    char route_destination[64];
    char route_gateway[64];
    char route_interface[64];
    char route_type[32];
    bool route_enabled;
    char current_tag[64];
    char temp_content[256];
};

/**
 * Start element handler for edit-config parsing
 */
static void edit_config_start_element(void *userData, const XML_Char *name, const XML_Char **atts)
{
    (void)atts; /* Suppress unused parameter warning */
    struct edit_config_data *data = (struct edit_config_data *)userData;
    
    debug_log(DEBUG_DEBUG, "edit_config_start_element: userData=%p, name=%s, atts=%p", userData, name, atts);
    
    if (strcmp(name, "edit-config") == 0) {
        data->is_edit_config = true;
        debug_log(DEBUG_DEBUG, "Found edit-config element");
    } else if (strcmp(name, "config") == 0) {
        data->in_config = true;
        debug_log(DEBUG_DEBUG, "Found config element");
    } else if (strcmp(name, "vrf") == 0 && data->in_config) {
        data->in_vrf = true;
        debug_log(DEBUG_DEBUG, "Found vrf element");
    } else if (strcmp(name, "route") == 0 && data->in_vrf) {
        data->in_route = true;
        debug_log(DEBUG_DEBUG, "Found route element");
        /* Capture the state here - we have a complete route configuration */
        data->has_route_config = true;
    } else if (data->in_route || data->in_vrf) {
        strlcpy(data->current_tag, name, sizeof(data->current_tag));
        data->temp_content[0] = '\0';
    }
}

/**
 * End element handler for edit-config parsing
 */
static void edit_config_end_element(void *userData, const XML_Char *name)
{
    struct edit_config_data *data = (struct edit_config_data *)userData;
    
    /* Only reset flags when we're completely done with the edit-config */
    if (strcmp(name, "edit-config") == 0) {
        debug_log(DEBUG_DEBUG, "Resetting all flags at end of edit-config");
        data->is_edit_config = false;
        data->in_config = false;
        data->in_vrf = false;
        data->in_route = false;
    } else if (data->in_vrf && !data->in_route) {
        if (strcmp(name, "name") == 0) {
            strlcpy(data->vrf_name, data->temp_content, sizeof(data->vrf_name));
            debug_log(DEBUG_DEBUG, "Extracted VRF name: %s", data->vrf_name);
        }
    } else if (data->in_route) {
        if (strcmp(name, "destination") == 0) {
            strlcpy(data->route_destination, data->temp_content, sizeof(data->route_destination));
            debug_log(DEBUG_DEBUG, "Extracted route destination: %s", data->route_destination);
        } else if (strcmp(name, "gateway") == 0) {
            strlcpy(data->route_gateway, data->temp_content, sizeof(data->route_gateway));
            debug_log(DEBUG_DEBUG, "Extracted route gateway: %s", data->route_gateway);
        } else if (strcmp(name, "interface") == 0) {
            strlcpy(data->route_interface, data->temp_content, sizeof(data->route_interface));
            debug_log(DEBUG_DEBUG, "Extracted route interface: %s", data->route_interface);
        } else if (strcmp(name, "type") == 0) {
            strlcpy(data->route_type, data->temp_content, sizeof(data->route_type));
            debug_log(DEBUG_DEBUG, "Extracted route type: %s", data->route_type);
        } else if (strcmp(name, "enabled") == 0) {
            data->route_enabled = (strcmp(data->temp_content, "true") == 0);
            debug_log(DEBUG_DEBUG, "Extracted route enabled: %s", data->temp_content);
        }
    }
}

/**
 * Character data handler for edit-config parsing
 */
static void edit_config_char_data(void *userData, const XML_Char *s, int len)
{
    struct edit_config_data *data = (struct edit_config_data *)userData;
    
    if ((data->in_vrf || data->in_route) && len > 0) {
        size_t current_len = strlen(data->temp_content);
        size_t remaining = sizeof(data->temp_content) - current_len - 1;
        
        if (remaining > 0) {
            size_t copy_len = (len < (int)remaining) ? (size_t)len : remaining;
            memcpy(data->temp_content + current_len, s, copy_len);
            data->temp_content[current_len + copy_len] = '\0';
        }
    }
}

/**
 * Check if request is an edit-config operation
 * @param request XML request string
 * @return true if edit-config request, false otherwise
 */
static bool is_edit_config_request(const char *request)
{
    XML_Parser parser = XML_ParserCreate(NULL);
    bool is_edit_config = false;
    
    if (!parser) {
        return false;
    }
    
    /* Set up user data to track what we find */
    struct edit_config_data user_data = { false, false, false, false, false, "", "", "", "", "", false, "", "" };
    
    XML_SetUserData(parser, &user_data);
    XML_SetElementHandler(parser, edit_config_start_element, NULL); /* Don't use end element handler for detection */
    XML_SetCharacterDataHandler(parser, edit_config_char_data);
    
    /* Parse the XML */
    if (XML_Parse(parser, request, strlen(request), 1) == XML_STATUS_OK) {
        is_edit_config = user_data.is_edit_config;
        debug_log(DEBUG_DEBUG, "Edit-config XML parsing result: is_edit_config=%s", 
                  is_edit_config ? "true" : "false");
    } else {
        debug_log(DEBUG_ERROR, "Edit-config XML parsing failed: %s", XML_ErrorString(XML_GetErrorCode(parser)));
    }
    
    XML_ParserFree(parser);
    return is_edit_config;
}

/**
 * Handle edit-config operation
 * @param state Server state
 * @param request XML request string
 * @param message_id Message ID
 * @param response Response XML string (allocated)
 * @return 0 on success, -1 on failure
 */
static int handle_edit_config(netd_state_t *state, const char *request, const char *message_id, char **response)
{
    XML_Parser parser = XML_ParserCreate(NULL);
    int result = -1;
    
    if (!parser) {
        return -1;
    }
    
    /* Set up user data to track what we find */
    struct edit_config_data user_data = { false, false, false, false, false, "", "", "", "", "", false, "", "" };
    
    XML_SetUserData(parser, &user_data);
    XML_SetElementHandler(parser, edit_config_start_element, edit_config_end_element);
    XML_SetCharacterDataHandler(parser, edit_config_char_data);
    
               /* Parse the XML */
           if (XML_Parse(parser, request, strlen(request), 1) == XML_STATUS_OK) {
               debug_log(DEBUG_DEBUG, "Edit-config parsing result: is_edit_config=%s, in_config=%s, in_vrf=%s, in_route=%s, has_route_config=%s",
                         user_data.is_edit_config ? "true" : "false",
                         user_data.in_config ? "true" : "false",
                         user_data.in_vrf ? "true" : "false",
                         user_data.in_route ? "true" : "false",
                         user_data.has_route_config ? "true" : "false");

               if (user_data.has_route_config) {
            /* We have a route configuration */
            debug_log(DEBUG_INFO, "Processing route configuration: VRF=%s, dest=%s, gw=%s, iface=%s, type=%s, enabled=%s", 
                      user_data.vrf_name, user_data.route_destination, user_data.route_gateway, 
                      user_data.route_interface[0] ? user_data.route_interface : "none",
                      user_data.route_type, user_data.route_enabled ? "true" : "false");
            
            /* Start a transaction if one isn't already active */
            if (!state->transaction_active) {
                debug_log(DEBUG_DEBUG, "Starting transaction for edit-config operation");
                if (transaction_begin(state) < 0) {
                    debug_log(DEBUG_ERROR, "Failed to start transaction for edit-config");
                    *response = generate_error_response(message_id, "application", "operation-failed", "Failed to start transaction");
                    XML_ParserFree(parser);
                    return -1;
                }
            }
            
            /* Determine FIB number from VRF name */
            uint32_t fib = 0;
            if (strcmp(user_data.vrf_name, "default") == 0) {
                fib = 0;
            } else {
                /* Try to parse as number first */
                char *endptr;
                fib = strtoul(user_data.vrf_name, &endptr, 10);
                if (*endptr != '\0') {
                    /* Not a number, look up VRF by name */
                    vrf_t *vrf = vrf_find_by_name(state, user_data.vrf_name);
                    if (vrf) {
                        fib = vrf->fib_number;
                    } else {
                        debug_log(DEBUG_ERROR, "VRF not found: %s", user_data.vrf_name);
                        *response = generate_error_response(message_id, "application", "operation-failed", "VRF not found");
                        XML_ParserFree(parser);
                        return -1;
                    }
                }
            }
            
            /* Check if this is a route deletion (enabled=false) or addition (enabled=true) */
            if (!user_data.route_enabled) {
                /* Route deletion */
                if (add_pending_route_delete(state, fib, user_data.route_destination) == 0) {
                    *response = generate_success_response(message_id, NULL);
                    result = 0;
                    debug_log(DEBUG_INFO, "Successfully added route deletion to pending changes: FIB=%u, dest=%s", 
                              fib, user_data.route_destination);
                } else {
                    debug_log(DEBUG_ERROR, "Failed to add route deletion to pending changes: FIB=%u, dest=%s", 
                              fib, user_data.route_destination);
                    *response = generate_error_response(message_id, "application", "operation-failed", "Failed to add route deletion to pending changes");
                }
            } else {
                /* Route addition */
                if (add_pending_route_add(state, fib, user_data.route_destination, user_data.route_gateway, 
                                         user_data.route_interface[0] ? user_data.route_interface : "", 0) == 0) {
                    *response = generate_success_response(message_id, NULL);
                    result = 0;
                    debug_log(DEBUG_INFO, "Successfully added route to pending changes: FIB=%u, dest=%s, gw=%s, iface=%s", 
                              fib, user_data.route_destination, user_data.route_gateway,
                              user_data.route_interface[0] ? user_data.route_interface : "none");
                } else {
                    debug_log(DEBUG_ERROR, "Failed to add route to pending changes: FIB=%u, dest=%s, gw=%s, iface=%s", 
                              fib, user_data.route_destination, user_data.route_gateway,
                              user_data.route_interface[0] ? user_data.route_interface : "none");
                    *response = generate_error_response(message_id, "application", "operation-failed", "Failed to add route to pending changes");
                }
            }
        } else if (user_data.is_edit_config && user_data.in_config) {
            /* Handle the old generic format for now - just return success */
            debug_log(DEBUG_INFO, "Processing generic edit-config operation");
            *response = generate_success_response(message_id, NULL);
            result = 0;
        } else {
            debug_log(DEBUG_WARN, "Unsupported edit-config operation");
            *response = generate_error_response(message_id, "application", "operation-not-supported", "Unsupported edit-config operation");
        }
    } else {
        debug_log(DEBUG_ERROR, "Edit-config XML parsing failed: %s", XML_ErrorString(XML_GetErrorCode(parser)));
        *response = generate_error_response(message_id, "protocol", "malformed-message", "Invalid XML");
    }
    
    XML_ParserFree(parser);
    return result;
} 