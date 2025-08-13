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

struct lyd_node *create_route_get_response(struct ly_ctx *ctx, const char *route_dest) {
    struct lyd_node *response = NULL;
    int ret;
    const struct lys_module *routing_module = get_ietf_routing_module(ctx);

    ret = lyd_new_inner(NULL, routing_module, "routing", 0, &response);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create routing response container");
        return NULL;
    }

    struct lyd_node *route_node = NULL;
    ret = lyd_new_inner(response, routing_module, "route", 0, &route_node);
    if (ret != LY_SUCCESS) {
        debug_log(ERROR, "Failed to create route node");
        return NULL;
    }

    lyd_new_term(route_node, routing_module, "destination-prefix", route_dest, 0, NULL);
    lyd_new_term(route_node, routing_module, "route-preference", "0", 0, NULL);

    return response;
} 