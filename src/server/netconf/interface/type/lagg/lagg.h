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

#ifndef LAGG_H
#define LAGG_H

#include <libyang/tree_data.h>

/* Forward declarations */
struct ly_ctx;

/**
 * Get LAGG interface data from FreeBSD system
 * @param ctx libyang context
 * @param iface_name interface name
 * @param iface_node interface node to add LAGG data to
 * @return 0 on success, -1 on failure
 */
int get_lagg_data(struct ly_ctx *ctx, const char *iface_name, struct lyd_node *iface_node);

/**
 * Add member to LAGG interface
 * @param lagg_name LAGG interface name
 * @param member_name member interface name
 * @return 0 on success, -1 on failure
 */
int lagg_add_member(const char *lagg_name, const char *member_name);

/**
 * Remove member from LAGG interface
 * @param lagg_name LAGG interface name
 * @param member_name member interface name
 * @return 0 on success, -1 on failure
 */
int lagg_remove_member(const char *lagg_name, const char *member_name);

/**
 * Set LAGG protocol
 * @param lagg_name LAGG interface name
 * @param protocol protocol name (failover, lacp, roundrobin, etc.)
 * @return 0 on success, -1 on failure
 */
int lagg_set_protocol(const char *lagg_name, const char *protocol);

#endif /* LAGG_H */ 