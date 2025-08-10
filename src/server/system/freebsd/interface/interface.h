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

#ifndef INTERFACE_H
#define INTERFACE_H

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <netd.h>

/**
 * Get interface operational status based on flags
 * @param flags Interface flags
 * @return "up" if IFF_RUNNING is set, "down" otherwise
 */
const char *freebsd_get_interface_oper_status(int flags);

/**
 * Create a network interface
 * @param name Interface name
 * @param type Interface type
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_create(const char *name, interface_type_t type);

/**
 * Check if interface exists in system
 * @param name Interface name
 * @return true if exists, false otherwise
 */
bool freebsd_interface_exists(const char *name);

/**
 * Delete a network interface
 * @param name Interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_delete(const char *name);

/**
 * Set interface FIB assignment
 * @param name Interface name
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_set_fib(const char *name, uint32_t fib);

/**
 * Set interface address
 * @param name Interface name
 * @param address Address string
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_set_address(const char *name, const char *address,
                                  int family);

/**
 * Delete interface address
 * @param name Interface name
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_delete_address(const char *name, int family);

/**
 * Set interface MTU
 * @param name Interface name
 * @param mtu MTU value
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_set_mtu(const char *name, int mtu);

/**
 * Get interface FIB
 * @param name Interface name
 * @param fib Pointer to store FIB number
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_get_fib(const char *name, uint32_t *fib);

/**
 * Get interface MTU
 * @param name Interface name
 * @param mtu Pointer to store MTU value
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_get_mtu(const char *name, int *mtu);

/**
 * Get interface groups
 * @param name Interface name
 * @param groups Array to store group names
 * @param max_groups Maximum number of groups to store
 * @param group_count Pointer to store actual number of groups
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_get_groups(const char *name,
                                 char (*groups)[MAX_GROUP_NAME_LEN],
                                 int max_groups, int *group_count);

#endif /* INTERFACE_H */ 