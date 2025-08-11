#include <netd.h>
#include <xml/xml.h>
#include <netconf/netconf.h>
#include <interface.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
interface_type_t filter_type = IF_TYPE_UNKNOWN;

debug_log(INFO, "Handling get-interfaces request for request: %s", request ? request : "NULL");

/* Extract interface type filter from request if present */
if (request) {
/* Parse XML to extract type filter */
const char *type_value = extract_type_from_xml_request(request);
if (type_value) {
/* Parse the type string */
if (strstr(type_value, "ianaift:bridge") || strstr(type_value, "bridge")) {
filter_type = IF_TYPE_BRIDGE;
} else if (strstr(type_value, "ianaift:l2vlan") || strstr(type_value, "vlan")) {
filter_type = IF_TYPE_VLAN;
} else if (strstr(type_value, "ianaift:ethernetCsmacd") || strstr(type_value, "ethernet")) {
filter_type = IF_TYPE_ETHERNET;
} else if (strstr(type_value, "netd:tap") || strstr(type_value, "tap")) {
filter_type = IF_TYPE_TAP;
} else if (strstr(type_value, "netd:lagg") || strstr(type_value, "lagg")) {
filter_type = IF_TYPE_LAGG;
} else if (strstr(type_value, "ianaift:softwareLoopback") || strstr(type_value, "loopback")) {
filter_type = IF_TYPE_LOOPBACK;
} else if (strstr(type_value, "ianaift:ieee80211") || strstr(type_value, "wlan")) {
filter_type = IF_TYPE_WIRELESS;
} else if (strstr(type_value, "netd:epair") || strstr(type_value, "epair")) {
filter_type = IF_TYPE_EPAIR;
} else if (strstr(type_value, "netd:gif") || strstr(type_value, "gif")) {
filter_type = IF_TYPE_GIF;
} else if (strstr(type_value, "netd:vxlan") || strstr(type_value, "vxlan")) {
filter_type = IF_TYPE_VXLAN;
}

debug_log(INFO, "Extracted interface type filter: %s -> %d", type_value, filter_type);
}
}

ret = interface_list(state, IF_TYPE_UNKNOWN);
if (ret < 0) {
debug_log(ERROR, "Failed to list interfaces");
*response = create_error_response(message_id, "operation-failed",
"Failed to list interfaces");
return -1;
}

/* Filter interfaces before XML generation */
if (filter_type != IF_TYPE_UNKNOWN) {
interface_t *iface, *next;
TAILQ_FOREACH_SAFE(iface, &state->interfaces, entries, next) {
if (iface->type != filter_type) {
TAILQ_REMOVE(&state->interfaces, iface, entries);
free(iface);
}
}
}

/* Create XML response with interface data */
xml_response = create_interfaces_xml_response(state, message_id);
if (!xml_response) {
debug_log(ERROR, "Failed to create interfaces XML response");
return -1;
}

*response = xml_response;
return 0;
}
