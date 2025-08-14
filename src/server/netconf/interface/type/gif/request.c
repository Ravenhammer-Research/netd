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

#include <gif.h>
#include <system/freebsd/gif/gif.h>
#include <netconf/netconf.h>
#include <netd.h>
#include <debug.h>
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Get GIF interface data from FreeBSD system
 * @param iface_name interface name
 * @param gif_data pointer to netd_gif_t structure to populate
 * @return 0 on success, -1 on failure
 */
int get_gif_data(const char *iface_name, netd_gif_t *gif_data) {
    if (!iface_name || !gif_data) {
        debug_log(ERROR, "Invalid parameters for GIF data acquisition");
        return -1;
    }
    
    /* Initialize the GIF structure */
    memset(gif_data, 0, sizeof(netd_gif_t));
    strlcpy(gif_data->base.name, iface_name, sizeof(gif_data->base.name));
    gif_data->base.type = NETD_IF_TYPE_GIF;
    
    /* Get GIF data from system */
    char local_addr[64];
    char remote_addr[64];
    
    if (freebsd_gif_show(iface_name, local_addr, sizeof(local_addr), 
                        remote_addr, sizeof(remote_addr)) == 0) {
        strlcpy(gif_data->local_addr, local_addr, sizeof(gif_data->local_addr));
        strlcpy(gif_data->remote_addr, remote_addr, sizeof(gif_data->remote_addr));
        
        debug_log(DEBUG, "Successfully acquired GIF data for %s: local=%s, remote=%s", 
                  iface_name, local_addr, remote_addr);
        return 0;
    } else {
        debug_log(ERROR, "Failed to get GIF data for %s", iface_name);
        return -1;
    }
}

/**
 * Set GIF tunnel endpoints
 * @param gif_name GIF interface name
 * @param local_addr local tunnel endpoint address
 * @param remote_addr remote tunnel endpoint address
 * @return 0 on success, -1 on failure
 */
int gif_set_endpoints(const char *gif_name, const char *local_addr, const char *remote_addr) {
    return freebsd_gif_set_tunnel(gif_name, local_addr, remote_addr);
} 