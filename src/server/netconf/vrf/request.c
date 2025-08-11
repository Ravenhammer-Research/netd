#include <netd.h>
#include <vrf.h>
#include <netconf/netconf.h>
#include <netconf/route/route.h>
#include <xml/xml.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

debug_log(INFO, "Handling get-vrfs request for request: %s", request ? request : "NULL");

ret = vrf_list(state);
if (ret < 0) {
debug_log(ERROR, "Failed to list VRFs");
*response = create_error_response(message_id, "operation-failed",
"Failed to list VRFs");
return -1;
}

/* Create XML response with VRF data */
xml_response = create_vrfs_xml_response(state, message_id);
if (!xml_response) {
debug_log(ERROR, "Failed to create VRFs XML response");
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

debug_log(INFO, "Handling get-vrf-routes request");

/* Extract VRF identifier from request */
vrf_name = extract_vrf_name_from_request(request);

if (!vrf_name) {
debug_log(ERROR, "Failed to extract VRF identifier from request");
*response = create_error_response(message_id, "operation-failed",
"Failed to extract VRF identifier");
return -1;
}

/* Determine if it's a name or ID and find VRF accordingly */
vrf_t *vrf = NULL;
uint32_t fib_number = 0;

if (isdigit(vrf_name[0])) {
/* It's a numeric ID/FIB number */
fib_number = (uint32_t)strtoul(vrf_name, NULL, 10);
vrf = vrf_find_by_fib(state, fib_number);
} else {
/* It's a name */
vrf = vrf_find_by_name(state, vrf_name);
if (vrf) {
fib_number = vrf->fib_number;
}
}

if (!vrf) {
debug_log(ERROR, "VRF %s not found", vrf_name);
free(vrf_name);
*response = create_error_response(message_id, "operation-failed",
"VRF not found");
return -1;
}

free(vrf_name);

/* Get all routes for this VRF */
char *routes_data = route_table_query(state, fib_number);
if (!routes_data) {
debug_log(ERROR, "Failed to get VRF routes for FIB %u", fib_number);
*response = create_error_response(message_id, "operation-failed",
"Failed to get VRF routes");
return -1;
}

/* Wrap the routes data in a complete NETCONF response */
asprintf(&xml_response,
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
"message-id=\"%s\">\n"
"  <data>\n"
"%s"
"  </data>\n"
"</rpc-reply>",
message_id, routes_data);

free(routes_data);
if (!xml_response) {
debug_log(ERROR, "Failed to create VRF routes XML response");
return -1;
}

*response = xml_response;
return 0;
}
