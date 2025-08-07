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
#include <sys/types.h>
#include <fcntl.h>
#include <sys/un.h>
#include <net/if.h>
#include <net/if_dl.h> // For struct sockaddr_dl
#include <sys/sockio.h> // For SIOCSIFFIB
#include <net/if_mib.h> // For struct ifmibdata
#include <ifaddrs.h> // For getifaddrs, freeifaddrs, struct ifaddrs
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/queue.h>
#include <sys/sysctl.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if_types.h>
#include <net/if_var.h>
#include <time.h>
#include <sys/module.h>
#include <sys/linker.h>

/**
 * Get interface operational status based on flags
 * @param flags Interface flags
 * @return "up" if IFF_RUNNING is set, "down" otherwise
 */
const char *freebsd_get_interface_oper_status(int flags)
{
    return (flags & IFF_RUNNING) ? "up" : "down";
}

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

    /* For epair interfaces, we need to use the cloning mechanism */
    if (type == IF_TYPE_EPAIR) {
        /* For epair interfaces, we create the base name and the system creates both a and b interfaces */
        /* First, try to load the epair module if it's not already loaded */
        int kld_id = kldload("if_epair");
        if (kld_id < 0 && errno != EEXIST) {
            debug_log(DEBUG_WARN, "Failed to load if_epair module: %s (continuing anyway)", strerror(errno));
        } else if (kld_id >= 0) {
            debug_log(DEBUG_INFO, "Loaded if_epair module (kld_id: %d)", kld_id);
        } else {
            debug_log(DEBUG_DEBUG, "if_epair module already loaded");
        }
        
        /* Check if the epair module is loaded */
        if (ioctl(sock, SIOCIFCREATE2, &ifr) < 0) {
            debug_log(DEBUG_ERROR, "Failed to create epair interface %s: %s", name, strerror(errno));
            close(sock);
            return -1;
        }
        debug_log(DEBUG_INFO, "Created epair interface %s (which creates %sa and %sb)", name, name, name);
    } else {
        /* Create interface for other types */
        if (ioctl(sock, SIOCIFCREATE2, &ifr) < 0) {
            debug_log(DEBUG_ERROR, "Failed to create interface %s: %s", name, strerror(errno));
            close(sock);
            return -1;
        }
        debug_log(DEBUG_INFO, "Created interface %s of type %s", name, type_str);
    }
    
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

    /* Set FIB for the socket - only for non-default FIB */
    if (fib > 0) {
        if (setsockopt(sock, SOL_SOCKET, SO_SETFIB, &fib, sizeof(fib)) < 0) {
            debug_log(DEBUG_ERROR, "Failed to set FIB %u for socket: %s", fib, strerror(errno));
            close(sock);
            return -1;
        }
        debug_log(DEBUG_DEBUG, "Set FIB %u for route socket", fib);
    }

    /* Calculate message length */
    len = sizeof(struct rt_msghdr) + SA_SIZE((struct sockaddr *)&dest_addr);
    if (gateway) {
        len += SA_SIZE((struct sockaddr *)&gw_addr);
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
    rtm->rtm_flags = flags | RTF_UP | RTF_STATIC;
    if (gateway) {
        rtm->rtm_flags |= RTF_GATEWAY;
    }
    rtm->rtm_version = RTM_VERSION;
    rtm->rtm_seq = 1;
    rtm->rtm_addrs = RTA_DST;
    rtm->rtm_fmask = 0;
    rtm->rtm_pid = getpid();
    rtm->rtm_msglen = len;

    /* Set destination address */
    cp = (char *)(rtm + 1);
    memmove(cp, (char *)&dest_addr, SA_SIZE((struct sockaddr *)&dest_addr));
    cp += SA_SIZE((struct sockaddr *)&dest_addr);

    /* Set gateway address if provided */
    if (gateway) {
        rtm->rtm_addrs |= RTA_GATEWAY;
        memmove(cp, (char *)&gw_addr, SA_SIZE((struct sockaddr *)&gw_addr));
        cp += SA_SIZE((struct sockaddr *)&gw_addr);
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
    debug_log(DEBUG_DEBUG, "Sending route message: type=%d, flags=0x%x, addrs=0x%x, len=%d", 
              rtm->rtm_type, rtm->rtm_flags, rtm->rtm_addrs, len);
    if (write(sock, rtm, len) < 0) {
        debug_log(DEBUG_ERROR, "Failed to add route: %s (errno=%d)", strerror(errno), errno);
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

/**
 * Get bridge member information for a bridge interface
 * @param ifname Interface name
 * @param members Buffer to store member information
 * @param members_size Size of the members buffer
 * @return 0 on success, -1 on failure
 */
int freebsd_get_bridge_members(const char *ifname, char *members, size_t members_size)
{
    struct ifaddrs *ifap, *ifa;
    char members_list[256] = "";
    int member_count = 0;
    uint32_t bridge_fib = 0;
    
    if (!ifname || !members) {
        return -1;
    }

    /* First get the FIB number for this bridge interface */
    int sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock >= 0) {
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
        
        if (ioctl(sock, SIOCGIFFIB, &ifr) == 0) {
            bridge_fib = ifr.ifr_fib;
            debug_log(DEBUG_TRACE, "Bridge %s is in FIB %u", ifname, bridge_fib);
        }
        close(sock);
    }

    /* Get all interfaces and find those in the same FIB as the bridge */
    if (getifaddrs(&ifap) != 0) {
        debug_log(DEBUG_ERROR, "Failed to get interface list: %s", strerror(errno));
        strlcpy(members, "none", members_size);
        return -1;
    }

    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_LINK) {
            continue;
        }

        /* Skip the bridge interface itself */
        if (strcmp(ifa->ifa_name, ifname) == 0) {
            continue;
        }

        /* Get FIB for this interface */
        sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
        if (sock >= 0) {
            struct ifreq ifr;
            memset(&ifr, 0, sizeof(ifr));
            strlcpy(ifr.ifr_name, ifa->ifa_name, sizeof(ifr.ifr_name));
            
            if (ioctl(sock, SIOCGIFFIB, &ifr) == 0) {
                if (ifr.ifr_fib == bridge_fib) {
                    /* This interface is in the same FIB as the bridge - likely a member */
                    if (member_count > 0) {
                        strlcat(members_list, ",", sizeof(members_list));
                    }
                    strlcat(members_list, ifa->ifa_name, sizeof(members_list));
                    member_count++;
                    debug_log(DEBUG_TRACE, "Found potential bridge member: %s (FIB %u)", ifa->ifa_name, ifr.ifr_fib);
                }
            }
            close(sock);
        }
    }

    freeifaddrs(ifap);

    if (strlen(members_list) > 0) {
        strlcpy(members, members_list, members_size);
        debug_log(DEBUG_TRACE, "Found bridge members for %s: '%s'", ifname, members);
    } else {
        strlcpy(members, "none", members_size);
        debug_log(DEBUG_INFO, "No bridge members found for %s", ifname);
    }
    
    return 0;
}

/**
 * Get VLAN information for an interface
 * @param ifname Interface name
 * @param vlan_id VLAN ID (output)
 * @param vlan_proto VLAN protocol (output)
 * @param proto_size Size of vlan_proto buffer
 * @param vlan_pcp VLAN Priority Code Point (output)
 * @param vlan_parent Parent interface name (output)
 * @param parent_size Size of vlan_parent buffer
 * @return 0 on success, -1 on failure
 */
int freebsd_get_vlan_info(const char *ifname, int *vlan_id, char *vlan_proto, size_t proto_size, 
                         int *vlan_pcp, char *vlan_parent, size_t parent_size)
{
    char path[256];
    char line[512];
    FILE *fp;
    char *token;
    int found = 0;
    
    if (!ifname || !vlan_id || !vlan_proto || !vlan_pcp || !vlan_parent) {
        return -1;
    }
    
    /* Initialize output parameters */
    *vlan_id = -1;
    *vlan_pcp = 0;
    vlan_proto[0] = '\0';
    vlan_parent[0] = '\0';
    
    /* Check if this is a VLAN interface by looking for dot in name */
    char *dot = strchr(ifname, '.');
    if (dot) {
        /* This is a VLAN interface like em0.18 */
        strlcpy(vlan_parent, ifname, dot - ifname + 1);
        *vlan_id = atoi(dot + 1);
        strlcpy(vlan_proto, "802.1q", proto_size);
        found = 1;
    } else if (strncmp(ifname, "vlan", 4) == 0) {
        /* This is a VLAN interface like vlan28 */
        *vlan_id = atoi(ifname + 4);
        strlcpy(vlan_proto, "802.1q", proto_size);
        
        /* Try to find parent interface using sysctl */
        snprintf(path, sizeof(path), "/proc/net/vlan/config");
        fp = fopen(path, "r");
        if (fp) {
            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, ifname)) {
                    /* Parse parent interface from line */
                    token = strtok(line, " \t");
                    if (token) {
                        token = strtok(NULL, " \t"); /* Skip VLAN name */
                        if (token) {
                            strlcpy(vlan_parent, token, parent_size);
                            found = 1;
                            break;
                        }
                    }
                }
            }
            fclose(fp);
        }
        
        if (!found) {
            /* Fallback: try to infer parent from interface name */
            snprintf(path, sizeof(path), "/proc/net/dev");
            fp = fopen(path, "r");
            if (fp) {
                while (fgets(line, sizeof(line), fp)) {
                    if (strstr(line, "em") || strstr(line, "igb") || strstr(line, "ix")) {
                        token = strtok(line, ":");
                        if (token) {
                            /* Remove leading/trailing whitespace */
                            while (*token == ' ') token++;
                            char *end = token + strlen(token) - 1;
                            while (end > token && *end == ' ') end--;
                            *(end + 1) = '\0';
                            
                            strlcpy(vlan_parent, token, parent_size);
                            found = 1;
                            break;
                        }
                    }
                }
                fclose(fp);
            }
        }
    }
    
    return found ? 0 : -1;
}

/**
 * Get WiFi information for an interface
 * @param ifname Interface name
 * @param regdomain Regulatory domain (output)
 * @param regdomain_size Size of regdomain buffer
 * @param country Country code (output)
 * @param country_size Size of country buffer
 * @param authmode Authentication mode (output)
 * @param authmode_size Size of authmode buffer
 * @param privacy Privacy setting (output)
 * @param privacy_size Size of privacy buffer
 * @param txpower Transmit power (output)
 * @param bmiss Beacon miss threshold (output)
 * @param scanvalid Scan validity period (output)
 * @param features WiFi features (output)
 * @param features_size Size of features buffer
 * @param bintval Beacon interval (output)
 * @param parent Parent interface name (output)
 * @param parent_size Size of parent buffer
 * @return 0 on success, -1 on failure
 */
int freebsd_get_wifi_info(const char *ifname, char *regdomain, size_t regdomain_size,
                         char *country, size_t country_size, char *authmode, size_t authmode_size,
                         char *privacy, size_t privacy_size, int *txpower, int *bmiss, int *scanvalid,
                         char *features, size_t features_size, int *bintval, char *parent, size_t parent_size)
{
    char path[256];
    char line[512];
    FILE *fp;
    char *token;
    int found = 0;
    
    if (!ifname || !regdomain || !country || !authmode || !privacy || !txpower || 
        !bmiss || !scanvalid || !features || !bintval || !parent) {
        return -1;
    }
    
    /* Initialize output parameters */
    regdomain[0] = '\0';
    country[0] = '\0';
    authmode[0] = '\0';
    privacy[0] = '\0';
    *txpower = 0;
    *bmiss = 0;
    *scanvalid = 0;
    features[0] = '\0';
    *bintval = 0;
    parent[0] = '\0';
    
    /* Check if this is a WiFi interface */
    if (strncmp(ifname, "wlan", 4) == 0 || 
        strncmp(ifname, "ath", 3) == 0 ||
        strncmp(ifname, "iwn", 3) == 0 ||
        strncmp(ifname, "iwm", 3) == 0 ||
        strncmp(ifname, "iwl", 3) == 0 ||
        strncmp(ifname, "bwi", 3) == 0 ||
        strncmp(ifname, "rum", 3) == 0 ||
        strncmp(ifname, "run", 3) == 0 ||
        strncmp(ifname, "ural", 4) == 0 ||
        strncmp(ifname, "urtw", 4) == 0 ||
        strncmp(ifname, "zyd", 3) == 0) {
        
        /* Try to get WiFi information using ifconfig */
        snprintf(path, sizeof(path), "/tmp/ifconfig_%s.txt", ifname);
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "ifconfig %s > %s 2>/dev/null", ifname, path);
        system(cmd);
        
        fp = fopen(path, "r");
        if (fp) {
            while (fgets(line, sizeof(line), fp)) {
                /* Parse regdomain and country */
                if (strstr(line, "regdomain")) {
                    token = strstr(line, "regdomain");
                    if (token) {
                        token += 9; /* Skip "regdomain " */
                        while (*token == ' ') token++;
                        char *end = strchr(token, ' ');
                        if (end) {
                            size_t len = end - token;
                            if (len < regdomain_size) {
                                strncpy(regdomain, token, len);
                                regdomain[len] = '\0';
                            }
                        }
                    }
                    
                    /* Extract country from same line */
                    token = strstr(line, "country");
                    if (token) {
                        token += 8; /* Skip "country " */
                        while (*token == ' ') token++;
                        char *end = strchr(token, ' ');
                        if (end) {
                            size_t len = end - token;
                            if (len < country_size) {
                                strncpy(country, token, len);
                                country[len] = '\0';
                            }
                        }
                    }
                }
                
                /* Parse authmode and privacy */
                if (strstr(line, "authmode")) {
                    token = strstr(line, "authmode");
                    if (token) {
                        token += 9; /* Skip "authmode " */
                        while (*token == ' ') token++;
                        char *end = strchr(token, ' ');
                        if (end) {
                            size_t len = end - token;
                            if (len < authmode_size) {
                                strncpy(authmode, token, len);
                                authmode[len] = '\0';
                            }
                        }
                    }
                }
                
                if (strstr(line, "privacy")) {
                    token = strstr(line, "privacy");
                    if (token) {
                        token += 8; /* Skip "privacy " */
                        while (*token == ' ') token++;
                        char *end = strchr(token, ' ');
                        if (end) {
                            size_t len = end - token;
                            if (len < privacy_size) {
                                strncpy(privacy, token, len);
                                privacy[len] = '\0';
                            }
                        }
                    }
                }
                
                /* Parse txpower */
                if (strstr(line, "txpower")) {
                    token = strstr(line, "txpower");
                    if (token) {
                        token += 8; /* Skip "txpower " */
                        while (*token == ' ') token++;
                        *txpower = atoi(token);
                    }
                }
                
                /* Parse bmiss */
                if (strstr(line, "bmiss")) {
                    token = strstr(line, "bmiss");
                    if (token) {
                        token += 6; /* Skip "bmiss " */
                        while (*token == ' ') token++;
                        *bmiss = atoi(token);
                    }
                }
                
                /* Parse scanvalid */
                if (strstr(line, "scanvalid")) {
                    token = strstr(line, "scanvalid");
                    if (token) {
                        token += 10; /* Skip "scanvalid " */
                        while (*token == ' ') token++;
                        *scanvalid = atoi(token);
                    }
                }
                
                /* Parse features */
                if (strstr(line, "wme") || strstr(line, "bintval")) {
                    char feature_list[128] = "";
                    if (strstr(line, "wme")) {
                        strcat(feature_list, "wme ");
                    }
                    if (strstr(line, "bintval")) {
                        strcat(feature_list, "bintval ");
                    }
                    if (strlen(feature_list) > 0) {
                        strlcpy(features, feature_list, features_size);
                    }
                }
                
                /* Parse bintval */
                if (strstr(line, "bintval")) {
                    token = strstr(line, "bintval");
                    if (token) {
                        token += 8; /* Skip "bintval " */
                        while (*token == ' ') token++;
                        *bintval = atoi(token);
                    }
                }
                
                /* Parse parent interface */
                if (strstr(line, "parent interface:")) {
                    token = strstr(line, "parent interface:");
                    if (token) {
                        token += 16; /* Skip "parent interface: " */
                        while (*token == ' ') token++;
                        char *end = strchr(token, '\n');
                        if (end) {
                            size_t len = end - token;
                            if (len < parent_size) {
                                strncpy(parent, token, len);
                                parent[len] = '\0';
                            }
                        }
                    }
                }
            }
            fclose(fp);
            unlink(path); /* Clean up temp file */
            found = 1;
        }
        
        /* Fallback: try to infer parent from interface name */
        if (parent[0] == '\0') {
            if (strncmp(ifname, "wlan", 4) == 0) {
                /* For wlan interfaces, try to find the parent */
                snprintf(path, sizeof(path), "/proc/net/dev");
                fp = fopen(path, "r");
                if (fp) {
                    while (fgets(line, sizeof(line), fp)) {
                        if (strstr(line, "iwm") || strstr(line, "iwn") || strstr(line, "ath")) {
                            token = strtok(line, ":");
                            if (token) {
                                /* Remove leading/trailing whitespace */
                                while (*token == ' ') token++;
                                char *end = token + strlen(token) - 1;
                                while (end > token && *end == ' ') end--;
                                *(end + 1) = '\0';
                                
                                strlcpy(parent, token, parent_size);
                                break;
                            }
                        }
                    }
                    fclose(fp);
                }
            }
        }
    }
    
    return found ? 0 : -1;
}

/**
 * Get interface FIB number
 * @param name Interface name
 * @param fib Pointer to store FIB number
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_get_fib(const char *name, uint32_t *fib)
{
    int sock;
    struct ifreq ifr;

    if (!name || !fib) {
        return -1;
    }

    /* Create socket for ioctl */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for FIB query: %s", strerror(errno));
        return -1;
    }

    /* Set up interface request */
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

    /* Get FIB */
    if (ioctl(sock, SIOCGIFFIB, &ifr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to get FIB for interface %s: %s", name, strerror(errno));
        close(sock);
        return -1;
    }

    *fib = ifr.ifr_fib;
    close(sock);
    return 0;
}

/**
 * Get interface MTU
 * @param name Interface name
 * @param mtu Pointer to store MTU value
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_get_mtu(const char *name, int *mtu)
{
    int sock;
    struct ifreq ifr;

    if (!name || !mtu) {
        return -1;
    }

    /* Create socket for ioctl */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for MTU query: %s", strerror(errno));
        return -1;
    }

    /* Set up interface request */
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

    /* Get MTU */
    if (ioctl(sock, SIOCGIFMTU, &ifr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to get MTU for interface %s: %s", name, strerror(errno));
        close(sock);
        return -1;
    }

    *mtu = ifr.ifr_mtu;
    close(sock);
    return 0;
}

/**
 * Get interface groups
 * @param name Interface name
 * @param groups Array to store group names
 * @param max_groups Maximum number of groups to store
 * @param group_count Pointer to store actual number of groups
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_get_groups(const char *name, char (*groups)[MAX_GROUP_NAME_LEN], int max_groups, int *group_count)
{
    int sock;
    struct ifgroupreq ifgr;
    struct ifg_req *ifg;
    size_t len;

    if (!name || !groups || !group_count) {
        return -1;
    }

    *group_count = 0;

    /* Create socket for ioctl */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for group query: %s", strerror(errno));
        return -1;
    }

    /* Set up interface group request */
    memset(&ifgr, 0, sizeof(ifgr));
    strlcpy(ifgr.ifgr_name, name, IFNAMSIZ);

    /* First call to get the required buffer size */
    if (ioctl(sock, SIOCGIFGROUP, (caddr_t)&ifgr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to get group info for %s: %s", name, strerror(errno));
        close(sock);
        return -1;
    }

    len = ifgr.ifgr_len;
    if (len > 0) {
        /* Allocate memory for group list */
        ifgr.ifgr_groups = (struct ifg_req *)calloc(len / sizeof(struct ifg_req), sizeof(struct ifg_req));
        if (ifgr.ifgr_groups != NULL) {
            /* Second call to get the actual group data */
            if (ioctl(sock, SIOCGIFGROUP, (caddr_t)&ifgr) == 0) {
                /* Process each group */
                for (ifg = ifgr.ifgr_groups; ifg && len >= sizeof(*ifg) && *group_count < max_groups; ifg++) {
                    len -= sizeof(*ifg);
                    strlcpy(groups[*group_count], ifg->ifgrq_group, MAX_GROUP_NAME_LEN);
                    (*group_count)++;
                }
            } else {
                debug_log(DEBUG_ERROR, "Failed to get groups for %s: %s", name, strerror(errno));
            }
            free(ifgr.ifgr_groups);
        }
    }

    close(sock);
    return 0;
} 

/**
 * Enumerate system interfaces and populate interface list
 * @param interfaces Interface list to populate
 * @return 0 on success, -1 on failure
 */
int freebsd_enumerate_interfaces(netd_state_t *state)
{
    struct ifaddrs *ifap, *ifa;
    interface_t *iface;
    char *last_name = NULL;
    
    if (!state) {
        return -1;
    }

    /* Get interface addresses */
    if (getifaddrs(&ifap) != 0) {
        debug_log(DEBUG_ERROR, "Failed to get interface addresses: %s", strerror(errno));
        return -1;
    }
    
    /* Iterate through interfaces */
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        /* Skip if we've already processed this interface name */
        if (last_name && strcmp(last_name, ifa->ifa_name) == 0) {
            continue;
        }
        
        /* Skip interfaces without addresses, except for certain virtual interface types */
        if (!ifa->ifa_addr) {
            /* For virtual interfaces like tap, tun, bridge, etc., we want to include them even without addresses */
            if (strncmp(ifa->ifa_name, "tap", 3) == 0 ||
                strncmp(ifa->ifa_name, "tun", 3) == 0 ||
                strncmp(ifa->ifa_name, "bridge", 6) == 0 ||
                strncmp(ifa->ifa_name, "vlan", 4) == 0 ||
                strncmp(ifa->ifa_name, "vxlan", 5) == 0 ||
                strncmp(ifa->ifa_name, "gif", 3) == 0 ||
                strncmp(ifa->ifa_name, "gre", 3) == 0 ||
                strncmp(ifa->ifa_name, "lagg", 4) == 0 ||
                strncmp(ifa->ifa_name, "epair", 5) == 0) {
                /* Continue processing these virtual interfaces even without addresses */
                debug_log(DEBUG_DEBUG, "Including virtual interface %s without address", ifa->ifa_name);
            } else {
                debug_log(DEBUG_DEBUG, "Skipping interface %s without address", ifa->ifa_name);
                continue;
            }
        }

        /* Skip loopback interface if it's not the main loopback */
        if (strcmp(ifa->ifa_name, "lo0") != 0 && 
            strncmp(ifa->ifa_name, "lo", 2) == 0) {
            continue;
        }

        /* Determine interface type based on name */
        interface_type_t type = IF_TYPE_UNKNOWN;
        if (strncmp(ifa->ifa_name, "em", 2) == 0 || 
            strncmp(ifa->ifa_name, "igb", 3) == 0 ||
            strncmp(ifa->ifa_name, "ix", 2) == 0 ||
            strncmp(ifa->ifa_name, "bge", 3) == 0 ||
            strncmp(ifa->ifa_name, "fxp", 3) == 0 ||
            strncmp(ifa->ifa_name, "re", 2) == 0 ||
            strncmp(ifa->ifa_name, "rl", 2) == 0 ||
            strncmp(ifa->ifa_name, "sk", 2) == 0 ||
            strncmp(ifa->ifa_name, "ti", 2) == 0 ||
            strncmp(ifa->ifa_name, "tx", 2) == 0 ||
            strncmp(ifa->ifa_name, "vr", 2) == 0 ||
            strncmp(ifa->ifa_name, "xl", 2) == 0) {
            type = IF_TYPE_ETHERNET;
        } else if (strncmp(ifa->ifa_name, "wlan", 4) == 0 ||
                   strncmp(ifa->ifa_name, "ath", 3) == 0 ||
                   strncmp(ifa->ifa_name, "iwn", 3) == 0 ||
                   strncmp(ifa->ifa_name, "iwm", 3) == 0 ||
                   strncmp(ifa->ifa_name, "iwl", 3) == 0 ||
                   strncmp(ifa->ifa_name, "bwi", 3) == 0 ||
                   strncmp(ifa->ifa_name, "rum", 3) == 0 ||
                   strncmp(ifa->ifa_name, "run", 3) == 0 ||
                   strncmp(ifa->ifa_name, "ural", 4) == 0 ||
                   strncmp(ifa->ifa_name, "urtw", 4) == 0 ||
                   strncmp(ifa->ifa_name, "zyd", 3) == 0) {
            type = IF_TYPE_WIRELESS;
        } else if (strncmp(ifa->ifa_name, "epair", 5) == 0) {
            type = IF_TYPE_EPAIR;
        } else if (strncmp(ifa->ifa_name, "gif", 3) == 0) {
            type = IF_TYPE_GIF;
        } else if (strncmp(ifa->ifa_name, "gre", 3) == 0) {
            type = IF_TYPE_GRE;
        } else if (strncmp(ifa->ifa_name, "lagg", 4) == 0) {
            type = IF_TYPE_LAGG;
        } else if (strcmp(ifa->ifa_name, "lo0") == 0) {
            type = IF_TYPE_LOOPBACK;
        } else if (strncmp(ifa->ifa_name, "ovpn", 4) == 0) {
            type = IF_TYPE_OVPN;
        } else if (strncmp(ifa->ifa_name, "tun", 3) == 0) {
            type = IF_TYPE_TUN;
        } else if (strncmp(ifa->ifa_name, "tap", 3) == 0) {
            type = IF_TYPE_TAP;
        } else if (strncmp(ifa->ifa_name, "vlan", 4) == 0) {
            type = IF_TYPE_VLAN;
        } else if (strncmp(ifa->ifa_name, "vxlan", 5) == 0) {
            type = IF_TYPE_VXLAN;
        } else if (strncmp(ifa->ifa_name, "bridge", 6) == 0) {
            type = IF_TYPE_BRIDGE;
            debug_log(DEBUG_TRACE, "Processing bridge interface: %s", ifa->ifa_name);
        }

        /* Allocate new interface */
        iface = malloc(sizeof(*iface));
        if (!iface) {
            debug_log(DEBUG_ERROR, "Failed to allocate memory for interface %s", ifa->ifa_name);
            continue;
        }

        /* Initialize interface */
        memset(iface, 0, sizeof(*iface));
        strlcpy(iface->name, ifa->ifa_name, sizeof(iface->name));
        iface->type = type;
        iface->fib = 0; /* Default FIB */
        iface->group_count = 0;
        iface->enabled = (ifa->ifa_flags & IFF_UP) != 0;
        iface->flags = ifa->ifa_flags;
        iface->mtu = 0; /* Will be set below */
        iface->bridge_members[0] = '\0'; /* Initialize bridge members to empty */

        /* Get additional interface information using system.c functions */
        
        /* Get FIB */
        if (freebsd_interface_get_fib(ifa->ifa_name, &iface->fib) == 0) {
            debug_log(DEBUG_TRACE, "Found FIB for %s: %u", ifa->ifa_name, iface->fib);
        } else {
            debug_log(DEBUG_DEBUG, "Failed to get FIB for %s", ifa->ifa_name);
        }
        
        /* Get MTU */
        if (freebsd_interface_get_mtu(ifa->ifa_name, &iface->mtu) == 0) {
            debug_log(DEBUG_TRACE, "Found MTU for %s: %d", ifa->ifa_name, iface->mtu);
        } else {
            debug_log(DEBUG_DEBUG, "Failed to get MTU for %s", ifa->ifa_name);
        }
        
        /* Get group information */
        if (freebsd_interface_get_groups(ifa->ifa_name, iface->groups, MAX_GROUPS_PER_IF, &iface->group_count) == 0) {
            debug_log(DEBUG_TRACE, "Found %d groups for %s", iface->group_count, ifa->ifa_name);
        } else {
            debug_log(DEBUG_DEBUG, "Failed to get groups for %s", ifa->ifa_name);
        }
        
        /* Get bridge member information for bridge interfaces */
        if (type == IF_TYPE_BRIDGE) {
            freebsd_get_bridge_members(ifa->ifa_name, iface->bridge_members, sizeof(iface->bridge_members));
        }
        
        /* Get VLAN information for VLAN interfaces */
        if (type == IF_TYPE_VLAN || strchr(ifa->ifa_name, '.') != NULL) {
            freebsd_get_vlan_info(ifa->ifa_name, &iface->vlan_id, iface->vlan_proto, sizeof(iface->vlan_proto),
                                 &iface->vlan_pcp, iface->vlan_parent, sizeof(iface->vlan_parent));
            debug_log(DEBUG_TRACE, "Found VLAN info for %s: id=%d, proto=%s, pcp=%d, parent=%s", 
                      ifa->ifa_name, iface->vlan_id, iface->vlan_proto, iface->vlan_pcp, iface->vlan_parent);
        }
        
        /* Get WiFi information for wireless interfaces */
        if (type == IF_TYPE_WIRELESS) {
            freebsd_get_wifi_info(ifa->ifa_name, iface->wifi_regdomain, sizeof(iface->wifi_regdomain),
                                 iface->wifi_country, sizeof(iface->wifi_country),
                                 iface->wifi_authmode, sizeof(iface->wifi_authmode),
                                 iface->wifi_privacy, sizeof(iface->wifi_privacy),
                                 &iface->wifi_txpower, &iface->wifi_bmiss, &iface->wifi_scanvalid,
                                 iface->wifi_features, sizeof(iface->wifi_features),
                                 &iface->wifi_bintval, iface->wifi_parent, sizeof(iface->wifi_parent));
            debug_log(DEBUG_TRACE, "Found WiFi info for %s: regdomain=%s, country=%s, authmode=%s, privacy=%s, txpower=%d, bmiss=%d, scanvalid=%d, features=%s, bintval=%d, parent=%s", 
                      ifa->ifa_name, iface->wifi_regdomain, iface->wifi_country, iface->wifi_authmode, 
                      iface->wifi_privacy, iface->wifi_txpower, iface->wifi_bmiss, iface->wifi_scanvalid,
                      iface->wifi_features, iface->wifi_bintval, iface->wifi_parent);
        }
        
        /* Get all addresses */
        struct ifaddrs *ifa_addr;
        bool primary_ipv4_found = false;
        bool primary_ipv6_found = false;
        
        for (ifa_addr = ifap; ifa_addr; ifa_addr = ifa_addr->ifa_next) {
            if (strcmp(ifa_addr->ifa_name, ifa->ifa_name) == 0 && ifa_addr->ifa_addr) {
                if (ifa_addr->ifa_addr->sa_family == AF_INET) {
                    struct sockaddr_in *sin = (struct sockaddr_in *)ifa_addr->ifa_addr;
                    char addr_str[64];
                    inet_ntop(AF_INET, &sin->sin_addr, addr_str, sizeof(addr_str));
                    
                    if (!primary_ipv4_found) {
                        /* First IPv4 address is primary */
                        strncpy(iface->primary_address, addr_str, sizeof(iface->primary_address) - 1);
                        iface->primary_address[sizeof(iface->primary_address) - 1] = '\0';
                        primary_ipv4_found = true;
                        debug_log(DEBUG_TRACE, "Found primary IPv4 address for %s: %s", ifa->ifa_name, iface->primary_address);
                    } else if (iface->alias_count < 10) {
                        /* Additional IPv4 addresses are aliases */
                        strncpy(iface->alias_addresses[iface->alias_count], addr_str, sizeof(iface->alias_addresses[0]) - 1);
                        iface->alias_addresses[iface->alias_count][sizeof(iface->alias_addresses[0]) - 1] = '\0';
                        iface->alias_count++;
                        debug_log(DEBUG_TRACE, "Found IPv4 alias for %s: %s", ifa->ifa_name, addr_str);
                    }
                } else if (ifa_addr->ifa_addr->sa_family == AF_INET6) {
                    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ifa_addr->ifa_addr;
                    char addr_str[64];
                    inet_ntop(AF_INET6, &sin6->sin6_addr, addr_str, sizeof(addr_str));
                    
                    if (!primary_ipv6_found) {
                        /* First IPv6 address is primary */
                        strncpy(iface->primary_address6, addr_str, sizeof(iface->primary_address6) - 1);
                        iface->primary_address6[sizeof(iface->primary_address6) - 1] = '\0';
                        primary_ipv6_found = true;
                        debug_log(DEBUG_TRACE, "Found primary IPv6 address for %s: %s", ifa->ifa_name, iface->primary_address6);
                    } else if (iface->alias_count6 < 10) {
                        /* Additional IPv6 addresses are aliases */
                        strncpy(iface->alias_addresses6[iface->alias_count6], addr_str, sizeof(iface->alias_addresses6[0]) - 1);
                        iface->alias_addresses6[iface->alias_count6][sizeof(iface->alias_addresses6[0]) - 1] = '\0';
                        iface->alias_count6++;
                        debug_log(DEBUG_TRACE, "Found IPv6 alias for %s: %s", ifa->ifa_name, addr_str);
                    }
                }
            }
        }

        /* Add to interface list */
        TAILQ_INSERT_TAIL(&state->interfaces, iface, entries);
        
        if (strncmp(ifa->ifa_name, "bridge", 6) == 0) {
            debug_log(DEBUG_TRACE, "Added bridge interface to list: %s", ifa->ifa_name);
        }
        
        debug_log(DEBUG_TRACE, "Added system interface %s (type: %s, enabled: %s, fib: %u, mtu: %d, addr: %s, addr6: %s, groups: %d)", 
                  ifa->ifa_name, 
                  interface_type_to_string(type),
                  iface->enabled ? "yes" : "no",
                  iface->fib,
                  iface->mtu,
                  iface->primary_address[0] ? iface->primary_address : "none",
                  iface->primary_address6[0] ? iface->primary_address6 : "none",
                  iface->group_count);

        last_name = strdup(ifa->ifa_name);
    }

    freeifaddrs(ifap);
    if (last_name) {
        free(last_name);
    }

    return 0;
} 

/**
 * Check if an interface name represents a FreeBSD hardware interface
 * @param name Interface name to check
 * @return true if it's a hardware interface, false otherwise
 */
bool freebsd_is_hardware_interface(const char *name)
{
    if (!name) {
        return false;
    }

    /* Check for FreeBSD hardware interface names */
    if (strncmp(name, "em", 2) == 0 || 
        strncmp(name, "igb", 3) == 0 ||
        strncmp(name, "ix", 2) == 0 ||
        strncmp(name, "bge", 3) == 0 ||
        strncmp(name, "fxp", 3) == 0 ||
        strncmp(name, "re", 2) == 0 ||
        strncmp(name, "rl", 2) == 0 ||
        strncmp(name, "sk", 2) == 0 ||
        strncmp(name, "ti", 2) == 0 ||
        strncmp(name, "tx", 2) == 0 ||
        strncmp(name, "vr", 2) == 0 ||
        strncmp(name, "xl", 2) == 0 ||
        strncmp(name, "wlan", 4) == 0 ||
        strncmp(name, "ath", 3) == 0 ||
        strncmp(name, "iwn", 3) == 0 ||
        strncmp(name, "iwm", 3) == 0 ||
        strncmp(name, "iwl", 3) == 0 ||
        strncmp(name, "bwi", 3) == 0 ||
        strncmp(name, "rum", 3) == 0 ||
        strncmp(name, "run", 3) == 0 ||
        strncmp(name, "ural", 4) == 0 ||
        strncmp(name, "urtw", 4) == 0 ||
        strncmp(name, "zyd", 3) == 0 ||
        strcmp(name, "lo0") == 0) {
        return true;
    }

    return false;
}

/**
 * Parse a FreeBSD route message and add it to the state
 * @param state Server state
 * @param rtm Route message header
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
static int parse_route_message(netd_state_t *state, struct rt_msghdr *rtm, uint32_t fib)
{
    char *cp = (char *)(rtm + 1);
    struct sockaddr *sp[RTAX_MAX];
    struct sockaddr_storage dest_addr, gw_addr;
    char ifname[IFNAMSIZ];
    netd_route_t *route;
    int i;
    
    memset(sp, 0, sizeof(sp));
    memset(&dest_addr, 0, sizeof(dest_addr));
    memset(&gw_addr, 0, sizeof(gw_addr));
    memset(ifname, 0, sizeof(ifname));
    
    /* Parse all sockaddr structures into array first (BSD approach) */
    for (i = 0; i < RTAX_MAX; i++) {
        if (rtm->rtm_addrs & (1 << i)) {
            sp[i] = (struct sockaddr *)cp;
            debug_log(DEBUG_TRACE, "Found sockaddr at index %d, family %d, len %d, addr: %p", i, sp[i]->sa_family, sp[i]->sa_len, (void*)sp[i]);
            cp += SA_SIZE(sp[i]);
        }
    }
    
    /* Extract destination address */
    if (sp[RTAX_DST]) {
        memcpy(&dest_addr, sp[RTAX_DST], sp[RTAX_DST]->sa_len);
        debug_log(DEBUG_TRACE, "Extracted destination, family %d", dest_addr.ss_family);
    }
    
    /* Extract gateway address - only if RTF_GATEWAY flag is set */
    if (sp[RTAX_GATEWAY] && (rtm->rtm_flags & RTF_GATEWAY)) {
        memcpy(&gw_addr, sp[RTAX_GATEWAY], sp[RTAX_GATEWAY]->sa_len);
        debug_log(DEBUG_TRACE, "Extracted gateway (RTF_GATEWAY), family %d", gw_addr.ss_family);
    } else if (sp[RTAX_GATEWAY]) {
        /* For direct routes, the gateway field contains the link-layer address */
        memcpy(&gw_addr, sp[RTAX_GATEWAY], sp[RTAX_GATEWAY]->sa_len);
        debug_log(DEBUG_TRACE, "Extracted gateway (direct), family %d", gw_addr.ss_family);
    } else {
        debug_log(DEBUG_DEBUG, "No gateway found in route message");
    }
    
    /* Extract interface name */
    if (sp[RTAX_IFP] && sp[RTAX_IFP]->sa_family == AF_LINK) {
        struct sockaddr_dl *sdl = (struct sockaddr_dl *)sp[RTAX_IFP];
        if (sdl->sdl_nlen > 0 && sdl->sdl_nlen < IFNAMSIZ) {
            memcpy(ifname, sdl->sdl_data, sdl->sdl_nlen);
            ifname[sdl->sdl_nlen] = '\0';
            debug_log(DEBUG_TRACE, "Extracted interface name: %s", ifname);
        } else {
            debug_log(DEBUG_DEBUG, "Interface name length invalid: %d", sdl->sdl_nlen);
        }
    } else if (sp[RTAX_IFP]) {
        debug_log(DEBUG_DEBUG, "Interface sockaddr found but not AF_LINK, family %d", sp[RTAX_IFP]->sa_family);
    } else {
        debug_log(DEBUG_DEBUG, "No interface sockaddr found");
    }
    
    /* Validate that we have a destination */
    if (dest_addr.ss_family == AF_UNSPEC) {
        debug_log(DEBUG_DEBUG, "Skipping route with no destination");
        return -1;
    }
    
    /* Allocate and populate route entry */
    route = malloc(sizeof(*route));
    if (route) {
        memset(route, 0, sizeof(*route));
        memcpy(&route->destination, &dest_addr, sizeof(dest_addr));
        memcpy(&route->gateway, &gw_addr, sizeof(gw_addr));
        
        /* Only copy interface name if it's valid */
        if (ifname[0] != '\0') {
            strlcpy(route->interface, ifname, sizeof(route->interface));
        }
        
        route->fib = fib; /* Set the FIB number */
        route->flags = rtm->rtm_flags;
        
        TAILQ_INSERT_TAIL(&state->routes, route, entries);
        
        char dest_str[INET6_ADDRSTRLEN];
        char gw_str[INET6_ADDRSTRLEN];
        format_address(&dest_addr, dest_str, sizeof(dest_str));
        if (gw_addr.ss_family != AF_UNSPEC) {
            if (gw_addr.ss_family == AF_LINK) {
                /* Format link-layer address as "link#X" */
                struct sockaddr_dl *sdl = (struct sockaddr_dl *)&gw_addr;
                snprintf(gw_str, sizeof(gw_str), "link#%d", sdl->sdl_index);
            } else {
                format_address(&gw_addr, gw_str, sizeof(gw_str));
            }
        } else {
            strlcpy(gw_str, "direct", sizeof(gw_str));
        }
        
        /* Only log if interface name is valid */
        if (ifname[0] != '\0') {
            debug_log(DEBUG_TRACE, "Added route: %s via %s on %s", dest_str, gw_str, ifname);
        } else {
            debug_log(DEBUG_TRACE, "Added route: %s via %s", dest_str, gw_str);
        }
        return 0;
    }
    
    return -1;
}

/**
 * Enumerate system routes and add them to state
 * @param state Server state
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
int freebsd_route_enumerate_system(netd_state_t *state, uint32_t fib)
{
    size_t needed;
    int mib[7];
    char *buf, *lim, *next;
    struct rt_msghdr *rtm;
    int retry_count = 0;
    const int max_retries = 3;
    int count = 0;

    if (!state) {
        return -1;
    }

    /* Enumerate IPv4 routes */
    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;        /* protocol */
    mib[3] = AF_INET;  /* IPv4 */
    mib[4] = NET_RT_DUMP; /* get all routes */
    mib[5] = 0;        /* no flags */
    mib[6] = fib;      /* FIB number */

retry_ipv4:
    /* First, get the size needed */
    if (sysctl(mib, nitems(mib), NULL, &needed, NULL, 0) < 0) {
        debug_log(DEBUG_ERROR, "Failed to get IPv4 route table size: %s", strerror(errno));
        return -1;
    }

    /* Allocate buffer */
    buf = malloc(needed);
    if (!buf) {
        debug_log(DEBUG_ERROR, "Failed to allocate IPv4 route table buffer");
        return -1;
    }

    /* Get the actual route data */
    if (sysctl(mib, nitems(mib), buf, &needed, NULL, 0) < 0) {
        if (errno == ENOMEM && retry_count++ < max_retries) {
            debug_log(DEBUG_DEBUG, "IPv4 route table grew, retrying (attempt %d)", retry_count);
            free(buf);
            sleep(1);
            goto retry_ipv4;
        }
        debug_log(DEBUG_ERROR, "Failed to get IPv4 route table: %s", strerror(errno));
        free(buf);
        return -1;
    }

    /* Process all IPv4 route messages */
    lim = buf + needed;
    for (next = buf; next < lim; next += rtm->rtm_msglen) {
        rtm = (struct rt_msghdr *)(void *)next;
        
        if (rtm->rtm_type == RTM_ADD || rtm->rtm_type == RTM_CHANGE || rtm->rtm_type == RTM_GET) {
            if (parse_route_message(state, rtm, fib) == 0) {
                count++;
            }
        }
    }

    free(buf);

    /* Enumerate IPv6 routes */
    retry_count = 0;
    mib[3] = AF_INET6; /* IPv6 */
    mib[5] = 0;        /* no flags */
    mib[6] = fib;      /* FIB number */

retry_ipv6:
    /* First, get the size needed */
    if (sysctl(mib, nitems(mib), NULL, &needed, NULL, 0) < 0) {
        debug_log(DEBUG_ERROR, "Failed to get IPv6 route table size: %s", strerror(errno));
        return -1;
    }

    /* Allocate buffer */
    buf = malloc(needed);
    if (!buf) {
        debug_log(DEBUG_ERROR, "Failed to allocate IPv6 route table buffer");
        return -1;
    }

    /* Get the actual route data */
    if (sysctl(mib, nitems(mib), buf, &needed, NULL, 0) < 0) {
        if (errno == ENOMEM && retry_count++ < max_retries) {
            debug_log(DEBUG_DEBUG, "IPv6 route table grew, retrying (attempt %d)", retry_count);
            free(buf);
            sleep(1);
            goto retry_ipv6;
        }
        debug_log(DEBUG_ERROR, "Failed to get IPv6 route table: %s", strerror(errno));
        free(buf);
        return -1;
    }

    /* Process all IPv6 route messages */
    lim = buf + needed;
    for (next = buf; next < lim; next += rtm->rtm_msglen) {
        rtm = (struct rt_msghdr *)(void *)next;
        
        if (rtm->rtm_type == RTM_ADD || rtm->rtm_type == RTM_CHANGE || rtm->rtm_type == RTM_GET) {
            if (parse_route_message(state, rtm, fib) == 0) {
                count++;
            }
        }
    }

    free(buf);

    debug_log(DEBUG_DEBUG, "Route enumeration completed successfully, added %d routes", count);
    return 0;
} 