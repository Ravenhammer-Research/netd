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

#include <lagg.h>
#include <system/freebsd/lagg/lagg.h>
#include <netconf/netconf.h>
#include <netd.h>
#include <debug.h>
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Get LAGG interface data from FreeBSD system
 * @param iface_name interface name
 * @param lagg_data pointer to netd_lagg_t structure to populate
 * @return 0 on success, -1 on failure
 */
int get_lagg_data(const char *iface_name, netd_lagg_t *lagg_data) {
    if (!iface_name || !lagg_data) {
        debug_log(ERROR, "Invalid parameters for LAGG data acquisition");
        return -1;
    }
    
    /* Initialize the LAGG structure */
    memset(lagg_data, 0, sizeof(netd_lagg_t));
    strlcpy(lagg_data->base.name, iface_name, sizeof(lagg_data->base.name));
    lagg_data->base.type = NETD_IF_TYPE_LAGG;
    netd_lagg_members_init(&lagg_data->members);
    
    /* Get LAGG data from system */
    char protocol[MAX_IFNAME_LEN];
    char members[MAX_IFNAME_LEN][10]; /* Assuming max 10 members */
    int member_count;
    
    if (freebsd_lagg_show(iface_name, protocol, sizeof(protocol), members, 10, &member_count) == 0) {
        /* Set LAGG protocol */
        strlcpy(lagg_data->protocol, protocol, sizeof(lagg_data->protocol));
        
        /* Add LAGG members */
        for (int i = 0; i < member_count; i++) {
            if (!netd_lagg_members_add(&lagg_data->members, NULL)) { /* Pass NULL for interface for now */
                debug_log(ERROR, "Failed to add LAGG member to list");
                continue;
            }
            debug_log(DEBUG2, "Found LAGG member: %s", members[i]);
        }
        
        debug_log(DEBUG, "Successfully acquired LAGG data for %s: protocol=%s, members=%d", 
                  iface_name, protocol, member_count);
        return 0;
    } else {
        debug_log(ERROR, "Failed to get LAGG data for %s", iface_name);
        return -1;
    }
}

/**
 * Add member to LAGG interface
 * @param lagg_name LAGG interface name
 * @param member_name member interface name
 * @return 0 on success, -1 on failure
 */
int lagg_add_member(const char *lagg_name, const char *member_name) {
    return freebsd_lagg_add_member(lagg_name, member_name);
}

/**
 * Remove member from LAGG interface
 * @param lagg_name LAGG interface name
 * @param member_name member interface name
 * @return 0 on success, -1 on failure
 */
int lagg_remove_member(const char *lagg_name, const char *member_name) {
    return freebsd_lagg_remove_member(lagg_name, member_name);
}

/**
 * Set LAGG protocol
 * @param lagg_name LAGG interface name
 * @param protocol protocol name (failover, lacp, roundrobin, etc.)
 * @return 0 on success, -1 on failure
 */
int lagg_set_protocol(const char *lagg_name, const char *protocol) {
    return freebsd_lagg_set_protocol(lagg_name, protocol);
} 