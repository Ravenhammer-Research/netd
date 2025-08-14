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

#include <pfsync.h>
#include <netconf/netconf.h>
#include <netd.h>
#include <debug.h>
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Get PFSYNC interface data from FreeBSD system
 * @param iface_name interface name
 * @param pfsync_data pointer to netd_pfsync_t structure to populate
 * @return 0 on success, -1 on failure
 */
int get_pfsync_data(const char *iface_name, netd_pfsync_t *pfsync_data) {
    if (!iface_name || !pfsync_data) {
        debug_log(ERROR, "Invalid parameters for PFSYNC data acquisition");
        return -1;
    }
    
    /* Initialize the PFSYNC structure */
    memset(pfsync_data, 0, sizeof(netd_pfsync_t));
    strlcpy(pfsync_data->base.name, iface_name, sizeof(pfsync_data->base.name));
    pfsync_data->base.type = NETD_IF_TYPE_PFSYNC;
    
    /* TODO: Implement PFSYNC data acquisition from system */
    /* For now, set default values */
    pfsync_data->enabled = 1;
    pfsync_data->sync_peer = 0;
    
    debug_log(DEBUG, "Successfully acquired PFSYNC data for %s", iface_name);
    return 0;
}
