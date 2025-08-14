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

#include <vlan.h>
#include <system/freebsd/vlan/vlan.h>
#include <netconf/netconf.h>
#include <netd.h>
#include <debug.h>
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Get VLAN interface data from FreeBSD system
 * @param iface_name interface name
 * @param vlan_data pointer to netd_vlan_t structure to populate
 * @return 0 on success, -1 on failure
 */
int get_vlan_data(const char *iface_name, netd_vlan_t *vlan_data) {
    if (!iface_name || !vlan_data) {
        debug_log(ERROR, "Invalid parameters for VLAN data acquisition");
        return -1;
    }
    
    /* Initialize the VLAN structure */
    memset(vlan_data, 0, sizeof(netd_vlan_t));
    strlcpy(vlan_data->base.name, iface_name, sizeof(vlan_data->base.name));
    vlan_data->base.type = NETD_IF_TYPE_VLAN;
    
    /* Get VLAN data from system */
    int vlan_id;
    char vlan_proto[32];
    int vlan_pcp;
    char vlan_parent[MAX_IFNAME_LEN];
    
    if (freebsd_vlan_show(iface_name, &vlan_id, vlan_proto, sizeof(vlan_proto), 
                         &vlan_pcp, vlan_parent, sizeof(vlan_parent)) == 0) {
        vlan_data->vlan_id = vlan_id;
        strlcpy(vlan_data->protocol, vlan_proto, sizeof(vlan_data->protocol));
        vlan_data->priority = vlan_pcp;
        strlcpy(vlan_data->parent, vlan_parent, sizeof(vlan_data->parent));
        
        debug_log(DEBUG, "Successfully acquired VLAN data for %s: id=%d, protocol=%s, priority=%d, parent=%s", 
                  iface_name, vlan_id, vlan_proto, vlan_pcp, vlan_parent);
        return 0;
    } else {
        debug_log(ERROR, "Failed to get VLAN data for %s", iface_name);
        return -1;
    }
}

/**
 * Create VLAN interface
 * @param vlan_name VLAN interface name
 * @param parent_name parent interface name
 * @param vlan_id VLAN ID
 * @return 0 on success, -1 on failure
 */
int vlan_create(const char *vlan_name, const char *parent_name, int vlan_id) {
    return freebsd_vlan_create(vlan_name, parent_name, vlan_id);
}

/**
 * Delete VLAN interface
 * @param vlan_name VLAN interface name
 * @return 0 on success, -1 on failure
 */
int vlan_delete(const char *vlan_name) {
    return freebsd_vlan_delete(vlan_name);
} 