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

#include <gre.h>
#include <system/freebsd/gre/gre.h>
#include <netconf/netconf.h>
#include <netd.h>
#include <debug.h>
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Get GRE interface data from FreeBSD system
 * @param iface_name interface name
 * @param gre_data pointer to netd_gre_t structure to populate
 * @return 0 on success, -1 on failure
 */
int get_gre_data(const char *iface_name, netd_gre_t *gre_data) {
    if (!iface_name || !gre_data) {
        debug_log(ERROR, "Invalid parameters for GRE data acquisition");
        return -1;
    }
    
    /* Initialize the GRE structure */
    memset(gre_data, 0, sizeof(netd_gre_t));
    strlcpy(gre_data->base.name, iface_name, sizeof(gre_data->base.name));
    gre_data->base.type = NETD_IF_TYPE_GRE;
    
    /* Get GRE data from system */
    char local_addr[64];
    char remote_addr[64];
    
    if (freebsd_gre_show(iface_name, local_addr, sizeof(local_addr), 
                        remote_addr, sizeof(remote_addr)) == 0) {
        strlcpy(gre_data->local_addr, local_addr, sizeof(gre_data->local_addr));
        strlcpy(gre_data->remote_addr, remote_addr, sizeof(gre_data->remote_addr));
        
        debug_log(DEBUG, "Successfully acquired GRE data for %s: local=%s, remote=%s", 
                  iface_name, local_addr, remote_addr);
        return 0;
    } else {
        debug_log(ERROR, "Failed to get GRE data for %s", iface_name);
        return -1;
    }
}
