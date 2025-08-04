/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "netd.h"
#include <fcntl.h>
#include <sys/un.h>
#include <net/if_dl.h> // For struct sockaddr_dl
#include <sys/sockio.h> // For SIOCSIFFIB

/**
 * Create a network interface
 * @param name Interface name
 * @param type Interface type
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_create(const char *name, interface_type_t type)
{
    int sock;
    struct ifreq ifr;
    const char *type_str;

    if (!name) {
        return -1;
    }

    /* Get interface type string */
    type_str = interface_type_to_string(type);
    if (!type_str) {
        debug_log(DEBUG_ERROR, "NULL type string for interface %s", name);
        return -1;
    }
    if (strcmp(type_str, "unknown") == 0) {
        debug_log(DEBUG_ERROR, "Unknown interface type for %s", name);
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Creating interface %s of type %s", name, type_str);

    /* Create socket for ioctl */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for interface creation: %s", strerror(errno));
        return -1;
    }

    /* Set up interface request */
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

    /* Create interface */
    if (ioctl(sock, SIOCIFCREATE2, &ifr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to create interface %s: %s", name, strerror(errno));
        close(sock);
        return -1;
    }

    debug_log(DEBUG_INFO, "Created interface %s of type %s", name, type_str);
    close(sock);
    return 0;
}

/**
 * Check if interface exists in system
 * @param name Interface name
 * @return true if exists, false otherwise
 */
bool freebsd_interface_exists(const char *name)
{
    int sock;
    struct ifreq ifr;

    if (!name) {
        return false;
    }

    /* Create socket for ioctl */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return false;
    }

    /* Set up interface request */
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

    /* Try to get interface flags - if it exists, this will succeed */
    bool exists = (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0);
    
    close(sock);
    return exists;
}

/**
 * Delete a network interface
 * @param name Interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_delete(const char *name)
{
    int sock;
    struct ifreq ifr;

    if (!name) {
        return -1;
    }

    /* Create socket for ioctl */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for interface deletion: %s", strerror(errno));
        return -1;
    }

    /* Set up interface request */
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

    /* Delete interface */
    if (ioctl(sock, SIOCIFDESTROY, &ifr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to delete interface %s: %s", name, strerror(errno));
        close(sock);
        return -1;
    }

    debug_log(DEBUG_INFO, "Deleted interface %s", name);
    close(sock);
    return 0;
}

/**
 * Set interface FIB assignment
 * @param name Interface name
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_set_fib(const char *name, uint32_t fib)
{
    int sock;
    struct ifreq ifr;

    if (!name) {
        return -1;
    }

    /* Create socket for ioctl */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for FIB assignment: %s", strerror(errno));
        return -1;
    }

    /* Set up interface request */
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    ifr.ifr_fib = fib;

    /* Set FIB */
    if (ioctl(sock, SIOCSIFFIB, &ifr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set FIB %u for interface %s: %s", fib, name, strerror(errno));
        close(sock);
        return -1;
    }

    debug_log(DEBUG_INFO, "Set FIB %u for interface %s", fib, name);
    close(sock);
    return 0;
}

/**
 * Set interface address
 * @param name Interface name
 * @param address Address string
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_set_address(const char *name, const char *address, int family)
{
    int sock;
    struct ifreq ifr;
    struct sockaddr_storage addr;

    if (!name || !address) {
        return -1;
    }

    /* Parse address */
    if (parse_address(address, &addr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to parse address %s", address);
        return -1;
    }

    /* Create socket for ioctl */
    sock = socket(family, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for address assignment: %s", strerror(errno));
        return -1;
    }

    /* Set up interface request */
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    memcpy(&ifr.ifr_addr, &addr, sizeof(ifr.ifr_addr));

    /* Set address */
    if (ioctl(sock, SIOCSIFADDR, &ifr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set address %s for interface %s: %s", address, name, strerror(errno));
        close(sock);
        return -1;
    }

    debug_log(DEBUG_INFO, "Set address %s for interface %s", address, name);
    close(sock);
    return 0;
}

/**
 * Delete interface address
 * @param name Interface name
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_delete_address(const char *name, int family)
{
    int sock;
    struct ifreq ifr;
    struct sockaddr_storage addr;

    if (!name) {
        return -1;
    }

    /* Create socket for ioctl */
    sock = socket(family, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for address deletion: %s", strerror(errno));
        return -1;
    }

    /* Set up interface request */
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    memset(&addr, 0, sizeof(addr));
    addr.ss_family = family;
    memcpy(&ifr.ifr_addr, &addr, sizeof(ifr.ifr_addr));

    /* Delete address */
    if (ioctl(sock, SIOCDIFADDR, &ifr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to delete address from interface %s: %s", name, strerror(errno));
        close(sock);
        return -1;
    }

    debug_log(DEBUG_INFO, "Deleted address from interface %s", name);
    close(sock);
    return 0;
}

/**
 * Set interface MTU
 * @param name Interface name
 * @param mtu MTU value
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_set_mtu(const char *name, int mtu)
{
    int sock;
    struct ifreq ifr;

    if (!name || mtu <= 0) {
        return -1;
    }

    /* Create socket for ioctl */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for MTU setting: %s", strerror(errno));
        return -1;
    }

    /* Set up interface request */
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    ifr.ifr_mtu = mtu;

    /* Set MTU */
    if (ioctl(sock, SIOCSIFMTU, &ifr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set MTU %d for interface %s: %s", mtu, name, strerror(errno));
        close(sock);
        return -1;
    }

    debug_log(DEBUG_INFO, "Set MTU %d for interface %s", mtu, name);
    close(sock);
    return 0;
}

/**
 * Add a route to the routing table
 * @param fib FIB number
 * @param destination Destination address
 * @param gateway Gateway address
 * @param interface Interface name
 * @param flags Route flags
 * @return 0 on success, -1 on failure
 */
int freebsd_route_add(uint32_t fib, const char *destination, const char *gateway, 
                      const char *interface, int flags)
{
    int sock;
    struct rt_msghdr *rtm;
    char *cp;
    int len;
    struct sockaddr_storage dest_addr, gw_addr;

    if (!destination) {
        return -1;
    }

    /* Parse addresses */
    if (parse_address(destination, &dest_addr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to parse destination address %s", destination);
        return -1;
    }

    if (gateway && parse_address(gateway, &gw_addr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to parse gateway address %s", gateway);
        return -1;
    }

    /* Create PF_ROUTE socket */
    sock = socket(PF_ROUTE, SOCK_RAW, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create PF_ROUTE socket: %s", strerror(errno));
        return -1;
    }

    /* Calculate message length */
    len = sizeof(struct rt_msghdr) + sizeof(struct sockaddr_storage);
    if (gateway) {
        len += sizeof(struct sockaddr_storage);
    }
    if (interface) {
        len += sizeof(struct sockaddr_dl);
    }

    /* Allocate message buffer */
    rtm = malloc(len);
    if (!rtm) {
        debug_log(DEBUG_ERROR, "Failed to allocate route message buffer");
        close(sock);
        return -1;
    }

    /* Set up route message */
    memset(rtm, 0, len);
    rtm->rtm_type = RTM_ADD;
    rtm->rtm_flags = flags | RTF_UP;
    rtm->rtm_version = RTM_VERSION;
    rtm->rtm_seq = 1;
    rtm->rtm_addrs = RTA_DST;
    rtm->rtm_fmask = 0;
    rtm->rtm_pid = getpid();
    rtm->rtm_msglen = len;

    /* Set destination address */
    cp = (char *)(rtm + 1);
    memcpy(cp, &dest_addr, sizeof(struct sockaddr_storage));
    cp += sizeof(struct sockaddr_storage);

    /* Set gateway address if provided */
    if (gateway) {
        rtm->rtm_addrs |= RTA_GATEWAY;
        memcpy(cp, &gw_addr, sizeof(struct sockaddr_storage));
        cp += sizeof(struct sockaddr_storage);
    }

    /* Set interface if provided */
    if (interface) {
        struct sockaddr_dl *sdl = (struct sockaddr_dl *)cp;
        rtm->rtm_addrs |= RTA_IFP;
        sdl->sdl_len = sizeof(struct sockaddr_dl);
        sdl->sdl_family = AF_LINK;
        sdl->sdl_index = if_nametoindex(interface);
        strlcpy(sdl->sdl_data, interface, sizeof(sdl->sdl_data));
    }

    /* Send route message */
    if (write(sock, rtm, len) < 0) {
        debug_log(DEBUG_ERROR, "Failed to add route: %s", strerror(errno));
        free(rtm);
        close(sock);
        return -1;
    }

    debug_log(DEBUG_INFO, "Added route to %s via %s (FIB %u)", destination, gateway ? gateway : "direct", fib);
    free(rtm);
    close(sock);
    return 0;
}

/**
 * Delete a route from the routing table
 * @param fib FIB number
 * @param destination Destination address
 * @return 0 on success, -1 on failure
 */
int freebsd_route_delete(uint32_t fib, const char *destination)
{
    int sock;
    struct rt_msghdr *rtm;
    char *cp;
    int len;
    struct sockaddr_storage dest_addr;

    if (!destination) {
        return -1;
    }

    /* Parse destination address */
    if (parse_address(destination, &dest_addr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to parse destination address %s", destination);
        return -1;
    }

    /* Create PF_ROUTE socket */
    sock = socket(PF_ROUTE, SOCK_RAW, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create PF_ROUTE socket: %s", strerror(errno));
        return -1;
    }

    /* Calculate message length */
    len = sizeof(struct rt_msghdr) + sizeof(struct sockaddr_storage);

    /* Allocate message buffer */
    rtm = malloc(len);
    if (!rtm) {
        debug_log(DEBUG_ERROR, "Failed to allocate route message buffer");
        close(sock);
        return -1;
    }

    /* Set up route message */
    memset(rtm, 0, len);
    rtm->rtm_type = RTM_DELETE;
    rtm->rtm_flags = 0;
    rtm->rtm_version = RTM_VERSION;
    rtm->rtm_seq = 1;
    rtm->rtm_addrs = RTA_DST;
    rtm->rtm_fmask = 0;
    rtm->rtm_pid = getpid();
    rtm->rtm_msglen = len;

    /* Set destination address */
    cp = (char *)(rtm + 1);
    memcpy(cp, &dest_addr, sizeof(struct sockaddr_storage));

    /* Send route message */
    if (write(sock, rtm, len) < 0) {
        debug_log(DEBUG_ERROR, "Failed to delete route: %s", strerror(errno));
        free(rtm);
        close(sock);
        return -1;
    }

    debug_log(DEBUG_INFO, "Deleted route to %s (FIB %u)", destination, fib);
    free(rtm);
    close(sock);
    return 0;
}

/**
 * List routes from the routing table
 * @param fib FIB number
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int freebsd_route_list(uint32_t fib, int family)
{
    size_t needed;
    int mib[6];
    char *buf, *lim, *next;
    struct rt_msghdr *rtm;
    int retry_count = 0;
    const int max_retries = 3;

    /* Use sysctl to get all routing information */
    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;        /* protocol */
    mib[3] = family;   /* address family */
    mib[4] = NET_RT_DUMP; /* get all routes */
    mib[5] = fib;      /* FIB number */

retry:
    /* First, get the size needed */
    if (sysctl(mib, nitems(mib), NULL, &needed, NULL, 0) < 0) {
        debug_log(DEBUG_ERROR, "Failed to get route table size: %s", strerror(errno));
        return -1;
    }

    /* Allocate buffer */
    buf = malloc(needed);
    if (!buf) {
        debug_log(DEBUG_ERROR, "Failed to allocate route table buffer");
        return -1;
    }

    /* Get the actual route data */
    if (sysctl(mib, nitems(mib), buf, &needed, NULL, 0) < 0) {
        if (errno == ENOMEM && retry_count++ < max_retries) {
            debug_log(DEBUG_DEBUG, "Route table grew, retrying (attempt %d)", retry_count);
            free(buf);
            sleep(1);
            goto retry;
        }
        debug_log(DEBUG_ERROR, "Failed to get route table: %s", strerror(errno));
        free(buf);
        return -1;
    }

    /* Process all route messages */
    lim = buf + needed;
    for (next = buf; next < lim; next += rtm->rtm_msglen) {
        rtm = (struct rt_msghdr *)(void *)next;
        
        if (rtm->rtm_type == RTM_ADD || rtm->rtm_type == RTM_CHANGE || rtm->rtm_type == RTM_GET) {
            /* Process route entry */
            debug_log(DEBUG_DEBUG, "Received route entry (type: %d)", rtm->rtm_type);
            
            /* Parse route information */
            char *cp = (char *)(rtm + 1);
            struct sockaddr *sa;
            struct sockaddr_storage dest_addr, gw_addr;
            char dest_str[INET6_ADDRSTRLEN];
            char gw_str[INET6_ADDRSTRLEN];
            char ifname[IFNAMSIZ];
            
            memset(&dest_addr, 0, sizeof(dest_addr));
            memset(&gw_addr, 0, sizeof(gw_addr));
            memset(ifname, 0, sizeof(ifname));
            
            /* Extract addresses from the message */
            for (int i = 0; i < RTAX_MAX; i++) {
                if (rtm->rtm_addrs & (1 << i)) {
                    sa = (struct sockaddr *)cp;
                    if (sa->sa_len == 0) {
                        cp += sizeof(struct sockaddr);
                        continue;
                    }
                    
                    switch (i) {
                        case RTAX_DST:
                            memcpy(&dest_addr, sa, sa->sa_len);
                            break;
                        case RTAX_GATEWAY:
                            memcpy(&gw_addr, sa, sa->sa_len);
                            break;
                        case RTAX_IFP:
                            if (sa->sa_family == AF_LINK) {
                                struct sockaddr_dl *sdl = (struct sockaddr_dl *)sa;
                                if (sdl->sdl_nlen > 0) {
                                    strlcpy(ifname, sdl->sdl_data, sizeof(ifname));
                                }
                            }
                            break;
                    }
                    cp += sa->sa_len;
                }
            }
            
            /* Format addresses for debug output */
            if (format_address(&dest_addr, dest_str, sizeof(dest_str)) < 0) {
                strlcpy(dest_str, "unknown", sizeof(dest_str));
            }
            
            if (gw_addr.ss_family != AF_UNSPEC) {
                if (format_address(&gw_addr, gw_str, sizeof(gw_str)) < 0) {
                    strlcpy(gw_str, "unknown", sizeof(gw_str));
                }
            } else {
                strlcpy(gw_str, "direct", sizeof(gw_str));
            }
            
            debug_log(DEBUG_DEBUG, "Route: %s via %s on %s", dest_str, gw_str, ifname);
        }
    }

    debug_log(DEBUG_INFO, "Listed routes for FIB %u", fib);
    free(buf);
    return 0;
} 