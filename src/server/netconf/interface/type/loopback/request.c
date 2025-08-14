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

#include <loopback.h>
#include <system/freebsd/loopback/loopback.h>
#include <netconf/netconf.h>
#include <netd.h>
#include <debug.h>
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Get Loopback interface data from FreeBSD system
 * @param iface_name interface name
 * @param loopback_data pointer to netd_loopback_t structure to populate
 * @return 0 on success, -1 on failure
 */
int get_loopback_data(const char *iface_name, netd_loopback_t *loopback_data) {
    if (!iface_name || !loopback_data) {
        debug_log(ERROR, "Invalid parameters for Loopback data acquisition");
        return -1;
    }
    
    /* Initialize the Loopback structure */
    memset(loopback_data, 0, sizeof(netd_loopback_t));
    strlcpy(loopback_data->base.name, iface_name, sizeof(loopback_data->base.name));
    loopback_data->base.type = NETD_IF_TYPE_LOOPBACK;
    
    /* Get Loopback data from system */
    uint32_t mtu;
    
    if (freebsd_loopback_get_mtu(iface_name, &mtu) == 0) {
        loopback_data->mtu = mtu;
    }
    
    debug_log(DEBUG, "Successfully acquired Loopback data for %s: mtu=%d", 
              iface_name, loopback_data->mtu);
    return 0;
}
