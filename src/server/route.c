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

/**
 * Add a route to the routing table
 * @param state Server state
 * @param fib FIB number
 * @param destination Destination address
 * @param gateway Gateway address (can be NULL for reject/blackhole)
 * @param interface Interface name (can be NULL)
 * @param flags Route flags
 * @return 0 on success, -1 on failure
 */
int route_add(netd_state_t *state, uint32_t fib, const char *destination, 
              const char *gateway, const char *interface, int flags)
{
    netd_route_t *route;
    int ret;

    if (!state || !destination || !is_valid_fib_number(fib)) {
        debug_log(DEBUG_ERROR, "Invalid parameters for route addition");
        return -1;
    }

    /* Allocate new route entry */
    route = malloc(sizeof(*route));
    if (!route) {
        debug_log(DEBUG_ERROR, "Failed to allocate memory for route");
        return -1;
    }

    /* Initialize route */
    memset(route, 0, sizeof(*route));
    if (parse_address(destination, &route->destination) < 0) {
        debug_log(DEBUG_ERROR, "Failed to parse destination address %s", destination);
        free(route);
        return -1;
    }
    if (gateway && parse_address(gateway, &route->gateway) < 0) {
        debug_log(DEBUG_ERROR, "Failed to parse gateway address %s", gateway);
        free(route);
        return -1;
    }
    if (interface) {
        strlcpy(route->interface, interface, sizeof(route->interface));
    }
    route->fib = fib;
    route->flags = flags;

    /* Add route in FreeBSD */
    ret = freebsd_route_add(fib, destination, gateway, interface, flags);
    if (ret < 0) {
        debug_log(DEBUG_ERROR, "Failed to add route in FreeBSD");
        free(route);
        return -1;
    }

    /* Add to route list */
    TAILQ_INSERT_TAIL(&state->routes, route, entries);

    debug_log(DEBUG_INFO, "Added route to %s via %s in FIB %u", destination, 
              gateway ? gateway : "direct", fib);
    return 0;
}

/**
 * Delete a route from the routing table
 * @param state Server state
 * @param fib FIB number
 * @param destination Destination address
 * @return 0 on success, -1 on failure
 */
int route_delete(netd_state_t *state, uint32_t fib, const char *destination)
{
    netd_route_t *route;
    struct sockaddr_storage dest_addr;

    if (!state || !destination || !is_valid_fib_number(fib)) {
        debug_log(DEBUG_ERROR, "Invalid parameters for route deletion");
        return -1;
    }

    /* Parse destination address */
    if (parse_address(destination, &dest_addr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to parse destination address %s", destination);
        return -1;
    }

    /* Find and remove route from list */
    TAILQ_FOREACH(route, &state->routes, entries) {
        if (route->fib == fib && 
            memcmp(&route->destination, &dest_addr, sizeof(dest_addr)) == 0) {
            TAILQ_REMOVE(&state->routes, route, entries);
            free(route);
            break;
        }
    }

    /* Delete route in FreeBSD */
    if (freebsd_route_delete(fib, destination) < 0) {
        debug_log(DEBUG_ERROR, "Failed to delete route in FreeBSD");
        return -1;
    }

    debug_log(DEBUG_INFO, "Deleted route to %s in FIB %u", destination, fib);
    return 0;
}

/**
 * List routes from the routing table
 * @param state Server state
 * @param fib FIB number
 * @param family Address family (AF_INET, AF_INET6, or AF_UNSPEC for all)
 * @return 0 on success, -1 on failure
 */
int route_list(netd_state_t *state, uint32_t fib, int family)
{
    netd_route_t *route;
    char dest_str[INET6_ADDRSTRLEN];
    char gw_str[INET6_ADDRSTRLEN];
    int count = 0;

    if (!state || !is_valid_fib_number(fib)) {
        debug_log(DEBUG_ERROR, "Invalid parameters for route listing");
        return -1;
    }

    debug_log(DEBUG_INFO, "Listing routes for FIB %u", fib);

    TAILQ_FOREACH(route, &state->routes, entries) {
        if (route->fib == fib && 
            (family == AF_UNSPEC || get_address_family(&route->destination) == family)) {
            
            /* Format destination address */
            if (format_address(&route->destination, dest_str, sizeof(dest_str)) < 0) {
                strlcpy(dest_str, "invalid", sizeof(dest_str));
            }

            /* Format gateway address */
            if (route->gateway.ss_family != AF_UNSPEC) {
                if (format_address(&route->gateway, gw_str, sizeof(gw_str)) < 0) {
                    strlcpy(gw_str, "invalid", sizeof(gw_str));
                }
            } else {
                strlcpy(gw_str, "-", sizeof(gw_str));
            }

            debug_log(DEBUG_INFO, "  %s via %s", dest_str, gw_str);
            if (route->interface[0] != '\0') {
                debug_log(DEBUG_INFO, "    Interface: %s", route->interface);
            }
            debug_log(DEBUG_INFO, "    Flags: 0x%x", route->flags);
            count++;
        }
    }

    if (count == 0) {
        debug_log(DEBUG_INFO, "  No routes found");
    } else {
        debug_log(DEBUG_INFO, "Total routes: %d", count);
    }

    return 0;
}

/**
 * Flush all routes for a FIB
 * @param state Server state
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
int route_flush_fib(netd_state_t *state, uint32_t fib)
{
    netd_route_t *route, *temp;

    if (!state || !is_valid_fib_number(fib)) {
        debug_log(DEBUG_ERROR, "Invalid parameters for route flush");
        return -1;
    }

    debug_log(DEBUG_INFO, "Flushing all routes for FIB %u", fib);

    /* Remove all routes for this FIB from the list */
    TAILQ_FOREACH_SAFE(route, &state->routes, entries, temp) {
        if (route->fib == fib) {
            TAILQ_REMOVE(&state->routes, route, entries);
            free(route);
        }
    }

    /* Note: FreeBSD doesn't have a direct "flush all routes for FIB" operation */
    /* We would need to iterate through all routes and delete them individually */
    /* For now, we'll just clear our internal state */
    debug_log(DEBUG_INFO, "Flushed routes for FIB %u", fib);
    return 0;
} 

/**
 * Helper function to parse route message and add to state
 * @param state Server state
 * @param rtm Route message header
 * @return 0 on success, -1 on failure
 */
static int parse_route_message(netd_state_t *state, struct rt_msghdr *rtm)
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
            debug_log(DEBUG_DEBUG, "Found sockaddr at index %d, family %d, len %d", i, sp[i]->sa_family, sp[i]->sa_len);
            cp += SA_SIZE(sp[i]);
        }
    }
    
    /* Extract destination address */
    if (sp[RTAX_DST]) {
        memcpy(&dest_addr, sp[RTAX_DST], sp[RTAX_DST]->sa_len);
        debug_log(DEBUG_DEBUG, "Extracted destination, family %d", dest_addr.ss_family);
    }
    
    /* Extract gateway address - only if RTF_GATEWAY flag is set */
    if (sp[RTAX_GATEWAY] && (rtm->rtm_flags & RTF_GATEWAY)) {
        memcpy(&gw_addr, sp[RTAX_GATEWAY], sp[RTAX_GATEWAY]->sa_len);
        debug_log(DEBUG_DEBUG, "Extracted gateway (RTF_GATEWAY), family %d", gw_addr.ss_family);
    } else if (sp[RTAX_GATEWAY]) {
        /* For direct routes, the gateway field contains the link-layer address */
        memcpy(&gw_addr, sp[RTAX_GATEWAY], sp[RTAX_GATEWAY]->sa_len);
        debug_log(DEBUG_DEBUG, "Extracted gateway (direct), family %d", gw_addr.ss_family);
    } else {
        debug_log(DEBUG_DEBUG, "No gateway found in route message");
    }
    
    /* Extract interface name */
    if (sp[RTAX_IFP] && sp[RTAX_IFP]->sa_family == AF_LINK) {
        struct sockaddr_dl *sdl = (struct sockaddr_dl *)sp[RTAX_IFP];
        if (sdl->sdl_nlen > 0 && sdl->sdl_nlen < IFNAMSIZ) {
            memcpy(ifname, sdl->sdl_data, sdl->sdl_nlen);
            ifname[sdl->sdl_nlen] = '\0';
            debug_log(DEBUG_DEBUG, "Extracted interface name: %s", ifname);
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
        
        route->fib = 0; /* Default FIB for now */
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
            debug_log(DEBUG_DEBUG, "Added route: %s via %s on %s", dest_str, gw_str, ifname);
        } else {
            debug_log(DEBUG_DEBUG, "Added route: %s via %s", dest_str, gw_str);
        }
        return 0;
    }
    
    return -1;
}

/**
 * Enumerate system routes and add them to state
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int route_enumerate_system(netd_state_t *state)
{
    size_t needed;
    int mib[6];
    char *buf, *lim, *next;
    struct rt_msghdr *rtm;
    int retry_count = 0;
    const int max_retries = 3;
    int count = 0;

    if (!state) {
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Enumerating system routes");

    /* Enumerate IPv4 routes */
    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;        /* protocol */
    mib[3] = AF_INET;  /* IPv4 */
    mib[4] = NET_RT_DUMP; /* get all routes */
    mib[5] = 0;        /* FIB 0 */

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
            if (parse_route_message(state, rtm) == 0) {
                count++;
            }
        }
    }

    free(buf);

    /* Enumerate IPv6 routes */
    retry_count = 0;
    mib[3] = AF_INET6; /* IPv6 */

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
            if (parse_route_message(state, rtm) == 0) {
                count++;
            }
        }
    }

    free(buf);

    debug_log(DEBUG_DEBUG, "Route enumeration complete, added %d routes", count);
    return 0;
}

/**
 * Get all routes as XML for NETCONF response
 * @param state Server state
 * @return XML string (allocated) or NULL on failure
 */
char *route_get_all(netd_state_t *state)
{
    netd_route_t *route;
    char *xml = NULL;
    char *temp_xml = NULL;
    char dest_str[INET6_ADDRSTRLEN];
    char gw_str[INET6_ADDRSTRLEN];
    char netmask_str[INET6_ADDRSTRLEN];
    
    if (!state) {
        return NULL;
    }

    /* Start XML */
    asprintf(&xml, "    <routing xmlns=\"urn:ietf:params:xml:ns:yang:ietf-routing\">\n"
                   "      <ribs>\n"
                   "        <rib>\n"
                   "          <name>default</name>\n"
                   "          <routes>\n");
    if (!xml) {
        return NULL;
    }

    debug_log(DEBUG_DEBUG, "Processing routes from state...");
    TAILQ_FOREACH(route, &state->routes, entries) {
        debug_log(DEBUG_DEBUG, "Found route in state");
        /* Format addresses */
        if (format_address(&route->destination, dest_str, sizeof(dest_str)) < 0) {
            strlcpy(dest_str, "invalid", sizeof(dest_str));
        }
        
        if (route->gateway.ss_family != AF_UNSPEC) {
            if (format_address(&route->gateway, gw_str, sizeof(gw_str)) < 0) {
                strlcpy(gw_str, "invalid", sizeof(gw_str));
            }
        } else {
            strlcpy(gw_str, "", sizeof(gw_str));
        }
        
        if (format_address(&route->netmask, netmask_str, sizeof(netmask_str)) < 0) {
            strlcpy(netmask_str, "", sizeof(netmask_str));
        }

        debug_log(DEBUG_DEBUG, "Adding route: %s via %s on %s", dest_str, gw_str, route->interface);

        asprintf(&temp_xml,
                "            <route>\n"
                "              <destination-prefix>%s</destination-prefix>\n"
                "              <next-hop>\n"
                "                <next-hop-address>%s</next-hop-address>\n"
                "                <outgoing-interface>%s</outgoing-interface>\n"
                "              </next-hop>\n"
                "            </route>\n",
                dest_str,
                gw_str,
                route->interface[0] != '\0' ? route->interface : "");

        if (temp_xml) {
            char *new_xml;
            asprintf(&new_xml, "%s%s", xml, temp_xml);
            free(xml);
            free(temp_xml);
            xml = new_xml;
        }
    }

    /* Close XML tags */
    asprintf(&temp_xml, "          </routes>\n"
                        "        </rib>\n"
                        "      </ribs>\n"
                        "    </routing>\n");
    if (temp_xml) {
        char *new_xml;
        asprintf(&new_xml, "%s%s", xml, temp_xml);
        free(xml);
        free(temp_xml);
        xml = new_xml;
    }

    return xml;
} 