/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistribributions of source code must retain the above copyright notice,
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

#ifndef NETCONF_H
#define NETCONF_H

#include "../netd.h"
#include "../xml/xml.h"

/* Forward declarations */
struct netd_state;

/* Main NETCONF request handler */
int netconf_handle_request(netd_state_t *state, const char *request,
                          char **response);



/* Request type checking functions */
bool is_get_interfaces_request(const char *request);
bool is_get_vrfs_request(const char *request);
bool is_get_routes_request(const char *request);
bool is_get_vrf_routes_request(const char *request);
bool is_save_request(const char *request);
bool is_commit_request(const char *request);
bool is_edit_config_request(const char *request);

/* Edit config handling */
int handle_edit_config(netd_state_t *state, const char *request,
                       const char *message_id, char **response);

/* Request handling functions */
int handle_commit_request(netd_state_t *state, const char *request,
                          const char *message_id, char **response);
int handle_save_request(netd_state_t *state, const char *request,
                         const char *message_id, char **response);
int handle_get_interfaces_request(netd_state_t *state, const char *request,
                                  const char *message_id, char **response);
int handle_get_vrfs_request(netd_state_t *state, const char *request,
                             const char *message_id, char **response);
int handle_get_routes_request(netd_state_t *state, const char *request,
                               const char *message_id, char **response);
int handle_get_vrf_routes_request(netd_state_t *state, const char *request,
                                   const char *message_id, char **response);

/* Response creation functions */
char *create_success_response(const char *message_id);
char *create_error_response(const char *message_id, const char *error_type,
                            const char *error_message);

/* External function declarations for pending changes */
extern int add_pending_vrf_create(netd_state_t *state, const char *name,
                                  uint32_t fib);
extern int add_pending_route_add(netd_state_t *state, uint32_t fib,
                                 const char *destination, const char *gateway,
                                 const char *interface, int flags);
extern int add_pending_route_delete(netd_state_t *state, uint32_t fib,
                                    const char *destination);
extern int add_pending_interface_set_fib(netd_state_t *state, const char *name,
                                         uint32_t fib);

#endif /* NETCONF_H */ 