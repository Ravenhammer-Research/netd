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

#ifndef FREEBSD_INTERFACE_H
#define FREEBSD_INTERFACE_H

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <netd.h>

/* System interface functions */
int freebsd_interface_create(const char *name, netd_interface_type_t type);
bool freebsd_interface_exists(const char *name);
int freebsd_interface_delete(const char *name);
int freebsd_interface_set_fib(const char *name, uint32_t fib);
int freebsd_interface_get_fib(const char *name, uint32_t *fib);
int freebsd_interface_set_address(const char *name, const char *address,
                                  int family);
int freebsd_interface_delete_address(const char *name, int family);
int freebsd_interface_set_mtu(const char *name, int mtu);
int freebsd_interface_get_mtu(const char *name, uint32_t *mtu);
int freebsd_interface_get_groups(const char *name, netd_interface_groups_t *groups);
int freebsd_enumerate_interfaces(netd_system_query_t *system_query);
bool freebsd_is_hardware_interface(netd_interface_type_t type);
const char *freebsd_get_interface_oper_status(int flags);



#endif /* FREEBSD_INTERFACE_H */ 