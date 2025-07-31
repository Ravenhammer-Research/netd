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

/**
 * Initialize YANG context
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int yang_init(netd_state_t *state)
{
    if (!state) {
        return -1;
    }

    /* Create YANG context with search paths */
    if (ly_ctx_new("./yang", 0, &state->yang_ctx) != LY_SUCCESS) {
        debug_log(DEBUG_ERROR, "Failed to create YANG context");
        return -1;
    }
    
    if (!state->yang_ctx) {
        debug_log(DEBUG_ERROR, "Failed to create YANG context");
        return -1;
    }

    /* Load standard IETF modules */
    struct lys_module *mod;
    
    mod = ly_ctx_load_module(state->yang_ctx, "ietf-interfaces", NULL, NULL);
    if (!mod) {
        debug_log(DEBUG_ERROR, "Failed to load ietf-interfaces module");
        goto error;
    }

    mod = ly_ctx_load_module(state->yang_ctx, "ietf-routing", NULL, NULL);
    if (!mod) {
        debug_log(DEBUG_ERROR, "Failed to load ietf-routing module");
        goto error;
    }

    mod = ly_ctx_load_module(state->yang_ctx, "ietf-routing-types", NULL, NULL);
    if (!mod) {
        debug_log(DEBUG_ERROR, "Failed to load ietf-routing-types module");
        goto error;
    }

    mod = ly_ctx_load_module(state->yang_ctx, "ietf-inet-types", NULL, NULL);
    if (!mod) {
        debug_log(DEBUG_ERROR, "Failed to load ietf-inet-types module");
        goto error;
    }

    /* Load our custom netd module */
    mod = ly_ctx_load_module(state->yang_ctx, "netd", NULL, NULL);
    if (!mod) {
        debug_log(DEBUG_ERROR, "Failed to load netd module");
        goto error;
    }

    debug_log(DEBUG_INFO, "YANG context initialized successfully");
    return 0;

error:
    yang_cleanup(state);
    return -1;
}

/**
 * Cleanup YANG context
 * @param state Server state
 */
void yang_cleanup(netd_state_t *state)
{
    if (state && state->yang_ctx) {
        ly_ctx_destroy(state->yang_ctx);
        state->yang_ctx = NULL;
        debug_log(DEBUG_INFO, "YANG context cleaned up");
    }
} 