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

#ifndef NETCONF_ROUTE_H
#define NETCONF_ROUTE_H

#include <netd.h>
#include <stdint.h>

/* Route management */
char *create_routes_xml_response(netd_state_t *state, const char *message_id, uint32_t fib);
int route_add(netd_state_t *state, uint32_t fib, const char *destination,
    const char *gateway, const char *interface, int flags);
int route_delete(netd_state_t *state, uint32_t fib, const char *destination);
int route_list(netd_state_t *state, uint32_t fib, int family);
int route_flush_fib(netd_state_t *state, uint32_t fib);
int route_clear_all(netd_state_t *state);
char *route_table_query(netd_state_t *state, uint32_t fib);
extern int add_pending_route_add(netd_state_t *state, uint32_t fib,
    const char *destination, const char *gateway,
    const char *interface, int flags);
extern int add_pending_route_delete(netd_state_t *state, uint32_t fib,
       const char *destination);

#endif /* NETCONF_ROUTE_H */