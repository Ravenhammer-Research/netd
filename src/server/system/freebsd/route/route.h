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

#ifndef ROUTE_H
#define ROUTE_H

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

struct netd_state;

/**
 * Add a route to the routing table
 * @param fib FIB number
 * @param destination Destination address
 * @param gateway Gateway address
 * @param interface Interface name
 * @param flags Route flags
 * @return 0 on success, -1 on failure
 */
int freebsd_route_add(uint32_t fib, const char *destination,
                      const char *gateway, const char *interface, int flags);

/**
 * Delete a route from the routing table
 * @param fib FIB number
 * @param destination Destination address
 * @return 0 on success, -1 on failure
 */
int freebsd_route_delete(uint32_t fib, const char *destination);

/**
 * List routes from the routing table and populate state
 * @param state Server state
 * @param fib FIB number
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int freebsd_route_list(netd_state_t *state, uint32_t fib, int family);

/**
 * Get the number of FIBs configured in the system
 * @return Number of FIBs, or 1 if unable to determine
 */
uint32_t get_system_fib_count(void);

/**
 * Validate FIB number
 * @param fib FIB number
 * @return true if valid, false otherwise
 */
bool is_valid_fib_number(uint32_t fib);

/**
 * Clean up interface map resources
 */
void freebsd_route_cleanup(void);

#endif /* ROUTE_H */ 