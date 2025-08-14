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

#include <ethernet.h>
#include <system/freebsd/interface/interface.h>
#include <netconf/netconf.h>
#include <netd.h>
#include <debug.h>
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Get Ethernet interface data from FreeBSD system
 * @param iface_name interface name
 * @param ethernet_data pointer to netd_ethernet_t structure to populate
 * @return 0 on success, -1 on failure
 */
int get_ethernet_data(const char *iface_name, netd_ethernet_t *ethernet_data) {
    if (!iface_name || !ethernet_data) {
        debug_log(ERROR, "Invalid parameters for Ethernet data acquisition");
        return -1;
    }
    
    /* Initialize the Ethernet structure */
    memset(ethernet_data, 0, sizeof(netd_ethernet_t));
    strlcpy(ethernet_data->base.name, iface_name, sizeof(ethernet_data->base.name));
    ethernet_data->base.type = NETD_IF_TYPE_ETHERNET;
    
    /* Get Ethernet data from system */
    uint32_t mtu;
    netd_interface_groups_t groups;
    
    if (freebsd_interface_get_mtu(iface_name, &mtu) == 0) {
        ethernet_data->mtu = mtu;
    }
    
    if (freebsd_interface_get_groups(iface_name, &groups) == 0) {
        /* Copy groups data */
        netd_interface_group_t *group;
        TAILQ_FOREACH(group, &groups.head, entries) {
            if (!netd_interface_groups_add(&ethernet_data->groups, group->name)) {
                debug_log(ERROR, "Failed to add interface group to Ethernet data");
            }
        }
    }
    
    debug_log(DEBUG, "Successfully acquired Ethernet data for %s: mtu=%d, groups=%d", 
              iface_name, ethernet_data->mtu, ethernet_data->groups.count);
    return 0;
}
