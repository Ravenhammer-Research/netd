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

#include <netd.h>
#include <debug.h>
#include <libyang/context.h>
#include <libyang/tree_schema.h>
#include <libyang/parser_schema.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Load required YANG modules into the context
 * @param ctx libyang context
 * @return 0 on success, -1 on failure
 */
int yang_load_modules(struct ly_ctx *ctx) {
    const char *modules[] = {
        "ietf-netconf",
        "ietf-netconf-monitoring", 
        "ietf-interfaces",
        "ietf-ip",
        "netd"
    };
    
    /* Set up YANG search paths */
    if (ly_ctx_set_searchdir(ctx, YANG_GLOBAL_PATH) != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set global YANG search path: %s", YANG_GLOBAL_PATH);
        return -1;
    }
    
    if (ly_ctx_set_searchdir(ctx, YANG_LOCAL_PATH) != LY_SUCCESS) {
        debug_log(ERROR, "Failed to set local YANG search path: %s", YANG_LOCAL_PATH);
        return -1;
    }
    
    for (size_t i = 0; i < sizeof(modules) / sizeof(modules[0]); i++) {
        struct lys_module *module = ly_ctx_load_module(ctx, modules[i], NULL, NULL);
        if (module == NULL) {
            debug_log(ERROR, "Failed to load YANG module: %s", modules[i]);
            return -1;
        }
        debug_log(DEBUG, "Loaded YANG module: %s", modules[i]);
    }
    
    return 0;
}
