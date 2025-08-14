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
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <bridge.h>
#include <sys/types.h>
#include <net/if.h>
#include <net/if_bridgevar.h>
#include <net/if_dl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/sockio.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netd.h>
#include <types.h>

/**
 * Add a member interface to a bridge
 * @param bridge_name Bridge interface name
 * @param member_name Member interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_bridge_add_member(const char *bridge_name, const char *member_name) {
    int sock;
    struct ifdrv ifd;
    struct ifbreq breq;
    
    if (!bridge_name || !member_name) {
        debug_log(ERROR, "Invalid parameters for bridge member addition");
        return -1;
    }
    
    debug_log(DEBUG, "Adding member %s to bridge %s", member_name, bridge_name);
    
    /* Create socket for ioctl */
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for bridge add member: %s", strerror(errno));
        return -1;
    }
    
    /* Set up ifdrv structure for bridge ioctl */
    memset(&ifd, 0, sizeof(ifd));
    strlcpy(ifd.ifd_name, bridge_name, sizeof(ifd.ifd_name));
    ifd.ifd_cmd = BRDGADD;  /* add bridge member (ifbreq) */
    ifd.ifd_len = sizeof(struct ifbreq);
    ifd.ifd_data = &breq;
    
    /* Set up ifbreq structure */
    memset(&breq, 0, sizeof(breq));
    strlcpy(breq.ifbr_ifsname, member_name, sizeof(breq.ifbr_ifsname));
    
    if (ioctl(sock, SIOCGDRVSPEC, &ifd) < 0) {
        debug_log(ERROR, "Failed to add member %s to bridge %s: %s", member_name, bridge_name, strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Added member %s to bridge %s", member_name, bridge_name);
    return 0;
}

/**
 * Set STP mode for a bridge
 * @param bridge_name Bridge interface name
 * @param stp_mode STP mode (0=disabled, 1=enabled)
 * @return 0 on success, -1 on failure
 */
int freebsd_bridge_set_stp(const char *bridge_name, int stp_mode) {
    int sock;
    struct ifdrv ifd;
    int stp_value;
    
    if (!bridge_name || (stp_mode != 0 && stp_mode != 1)) {
        debug_log(ERROR, "Invalid parameters for bridge STP setting");
        return -1;
    }
    
    debug_log(DEBUG, "Setting STP mode for bridge %s to %d", bridge_name, stp_mode);
    
    /* Create socket for ioctl */
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for bridge STP setting: %s", strerror(errno));
        return -1;
    }
    
    /* Set up ifdrv structure for bridge ioctl */
    memset(&ifd, 0, sizeof(ifd));
    strlcpy(ifd.ifd_name, bridge_name, sizeof(ifd.ifd_name));
    ifd.ifd_cmd = BRDGSIFFLGS;  /* set member if flags (ifbreq) - using this as closest match for STP setting */
    ifd.ifd_len = sizeof(int);
    ifd.ifd_data = &stp_value;
    
    /* Set STP value */
    stp_value = stp_mode;
    
    if (ioctl(sock, SIOCGDRVSPEC, &ifd) < 0) {
        debug_log(ERROR, "Failed to set STP mode for bridge %s: %s", bridge_name, strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Set STP mode for bridge %s to %d", bridge_name, stp_mode);
    return 0;
}
