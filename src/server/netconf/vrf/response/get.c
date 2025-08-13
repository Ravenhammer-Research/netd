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

static const struct lys_module *get_ietf_routing_module(struct ly_ctx *ctx) {
    return ly_ctx_get_module(ctx, "ietf-routing", NULL);
}

struct lyd_node *create_vrf_get_response(struct ly_ctx *ctx, const char *vrf_name) {
    struct lyd_node *response = NULL;
    int ret;
    const struct lys_module *routing_module = get_ietf_routing_module(ctx);

    ret = lyd_new_inner(NULL, routing_module, "routing-instance", 0, &response);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create routing-instance response container");
        return NULL;
    }

    struct lyd_node *vrf_node = NULL;
    ret = lyd_new_inner(response, routing_module, "routing-instance", 0, &vrf_node);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create routing-instance node");
        return NULL;
    }

    lyd_new_term(vrf_node, routing_module, "name", vrf_name, 0, NULL);
    lyd_new_term(vrf_node, routing_module, "type", "l3vpn:l3vpn", 0, NULL);
    lyd_new_term(vrf_node, routing_module, "enabled", "true", 0, NULL);

    return response;
} 