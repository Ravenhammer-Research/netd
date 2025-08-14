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

#ifndef FREEBSD_BRIDGE_H
#define FREEBSD_BRIDGE_H

#include <sys/types.h>
#include <stddef.h>
#include <types.h>

/**
 * Get bridge members and populate netd_bridge_members_t structure
 * @param ifname Bridge interface name
 * @param members netd_bridge_members_t structure to populate
 * @return 0 on success, -1 on failure
 */
int freebsd_get_bridge_members(const char *ifname, netd_bridge_members_t *members);

/**
 * Get complete bridge data including interface base and members
 * @param ifname Bridge interface name
 * @param bridge netd_bridge_t structure to populate
 * @return 0 on success, -1 on failure
 */
int freebsd_get_bridge_data(const char *ifname, netd_bridge_t *bridge);

/**
 * Add a member interface to a bridge
 * @param bridge_name Bridge interface name
 * @param member_name Member interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_bridge_add_member(const char *bridge_name, const char *member_name);

/**
 * Remove a member interface from a bridge
 * @param bridge_name Bridge interface name
 * @param member_name Member interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_bridge_remove_member(const char *bridge_name, const char *member_name);

/**
 * Set STP mode for a bridge
 * @param bridge_name Bridge interface name
 * @param stp_mode STP mode (0=disabled, 1=enabled)
 * @return 0 on success, -1 on failure
 */
int freebsd_bridge_set_stp(const char *bridge_name, int stp_mode);

#endif /* FREEBSD_BRIDGE_H */ 