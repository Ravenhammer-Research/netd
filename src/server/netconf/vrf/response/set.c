#include <response.h>
#include <netconf/netconf.h>
#include <netd.h>
#include <debug.h>
#include <libyang/tree_data.h>
#include <libyang/tree_schema.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern struct ly_ctx *yang_ctx;

static const struct lys_module *get_ietf_netconf_module(struct ly_ctx *ctx) {
    return ly_ctx_get_module(ctx, "ietf-netconf", NULL);
}

struct lyd_node *create_vrf_set_response(struct ly_ctx *ctx, const char *vrf_name, bool success) {
    struct lyd_node *response = NULL;
    int ret;
    const struct lys_module *netconf_module = get_ietf_netconf_module(ctx);
    char error_msg[256];

    ret = lyd_new_inner(NULL, netconf_module, "rpc-reply", 0, &response);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create rpc-reply container");
        return NULL;
    }

    if (success) {
        lyd_new_term(response, netconf_module, "ok", "", 0, NULL);
    } else {
        struct lyd_node *error = NULL;
        ret = lyd_new_inner(response, netconf_module, "rpc-error", 0, &error);
        if (ret == LY_SUCCESS) {
            lyd_new_term(error, netconf_module, "error-type", "application", 0, NULL);
            lyd_new_term(error, netconf_module, "error-tag", "operation-failed", 0, NULL);
            lyd_new_term(error, netconf_module, "error-severity", "error", 0, NULL);
            snprintf(error_msg, sizeof(error_msg), "VRF configuration failed for VRF: %s", vrf_name);
            lyd_new_term(error, netconf_module, "error-message", error_msg, 0, NULL);
        }
    }

    return response;
} 