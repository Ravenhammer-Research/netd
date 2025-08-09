/*
 * Copyright (c) 2024 The FreeBSD Foundation
 *
 * This software was developed by {your organization}.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "80211.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <unistd.h>
#include <net/if.h>

/**
 * Delete a wireless interface
 * @param name Wireless interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_wireless_delete(const char *name) {
    int sock;
    struct ifreq ifr;
    
    if (!name) {
        debug_log(DEBUG_ERROR, "Invalid parameters for wireless interface deletion");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Deleting wireless interface %s", name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for wireless interface deletion: %s", strerror(errno));
        return -1;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    
    if (ioctl(sock, SIOCIFDESTROY, &ifr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to delete wireless interface: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Deleted wireless interface %s", name);
    return 0;
}

/**
 * Delete a WLAN interface
 * @param name WLAN interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_delete(const char *name) {
    int sock;
    struct ifreq ifr;
    
    if (!name) {
        debug_log(DEBUG_ERROR, "Invalid parameters for WLAN interface deletion");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Deleting WLAN interface %s", name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for WLAN interface deletion: %s", strerror(errno));
        return -1;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    
    if (ioctl(sock, SIOCIFDESTROY, &ifr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to delete WLAN interface: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Deleted WLAN interface %s", name);
    return 0;
} 