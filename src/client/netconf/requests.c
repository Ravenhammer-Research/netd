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
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

#include <net.h>
#include <netconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Get interfaces from the server
 */
int netconf_get_interfaces(net_client_t *client, char **response) {
    char request[1024];
    
    if (!client || !response) {
        return -1;
    }
    
    /* Build get-interfaces request */
    snprintf(request, sizeof(request),
             "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
             "<get>"
             "<filter type=\"subtree\">"
             "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">"
             "<interface>"
             "<name/>"
             "<type/>"
             "<enabled/>"
             "<description/>"
             "<ipv4 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">"
             "<address>"
             "<ip/>"
             "<prefix-length/>"
             "</address>"
             "<mtu/>"
             "</ipv4>"
             "<ipv6 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">"
             "<address>"
             "<ip/>"
             "<prefix-length/>"
             "</address>"
             "</ipv6>"
             "</interface>"
             "</interfaces>"
             "</filter>"
             "</get>"
             "</rpc>");
    
    return netconf_send_request(client, request, response);
}

/**
 * Get VRFs from the server
 */
int netconf_get_vrfs(net_client_t *client, char **response) {
    char request[1024];
    
    if (!client || !response) {
        return -1;
    }
    
    /* Build get-vrfs request */
    snprintf(request, sizeof(request),
             "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
             "<get>"
             "<filter type=\"subtree\">"
             "<vrfs xmlns=\"urn:ietf:params:xml:ns:yang:frr-vrf\">"
             "<vrf>"
             "<name/>"
             "<fib/>"
             "<description/>"
             "</vrf>"
             "</vrfs>"
             "</filter>"
             "</get>"
             "</rpc>");
    
    return netconf_send_request(client, request, response);
}

/**
 * Get routes from the server
 */
int netconf_get_routes(net_client_t *client, uint32_t fib, int family, char **response) {
    char request[1024];
    const char *family_ns, *family_tag;
    
    if (!client || !response) {
        return -1;
    }
    
    /* Determine address family */
    if (family == AF_INET) {
        family_ns = "urn:ietf:params:xml:ns:yang:ietf-ipv4-unicast-routing";
        family_tag = "ipv4";
    } else if (family == AF_INET6) {
        family_ns = "urn:ietf:params:xml:ns:yang:ietf-ipv6-unicast-routing";
        family_tag = "ipv6";
    } else if (family == AF_UNSPEC) {
        /* For AF_UNSPEC, use a generic routing namespace that includes both IPv4 and IPv6 */
        family_ns = "urn:ietf:params:xml:ns:yang:ietf-routing";
        family_tag = "default";
    } else {
        fprintf(stderr, "Unsupported address family: %d\n", family);
        return -1;
    }
    
    /* Build get-routes request */
    snprintf(request, sizeof(request),
             "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
             "<get>"
             "<filter type=\"subtree\">"
             "<routing xmlns=\"%s\">"
             "<routing-instance>"
             "<name>%u</name>"
             "<ribs>"
             "<rib>"
             "<name>%s</name>"
             "<routes>"
             "<route>"
             "<destination-prefix/>"
             "<next-hop>"
             "<next-hop-address/>"
             "<outgoing-interface/>"
             "</next-hop>"
             "<source-protocol/>"
             "<route-preference/>"
             "</route>"
             "</routes>"
             "</rib>"
             "</ribs>"
             "</routing-instance>"
             "</routing>"
             "</filter>"
             "</get>"
             "</rpc>",
             family_ns, fib, family_tag);
    
    return netconf_send_request(client, request, response);
}

/**
 * Get interface groups from the server
 */
int netconf_get_interface_groups(net_client_t *client, char **response) {
    char request[1024];
    
    if (!client || !response) {
        return -1;
    }
    
    /* Build get-interface-groups request */
    snprintf(request, sizeof(request),
             "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">"
             "<get>"
             "<filter type=\"subtree\">"
             "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">"
             "<interface>"
             "<name/>"
             "<type/>"
             "<enabled/>"
             "<description/>"
             "<ipv4 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">"
             "<address>"
             "<ip/>"
             "<prefix-length/>"
             "</address>"
             "</ipv4>"
             "<ipv6 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">"
             "<address>"
             "<ip/>"
             "<prefix-length/>"
             "</address>"
             "</ipv6>"
             "</interface>"
             "</interfaces>"
             "</filter>"
             "</get>"
             "</rpc>");
    
    return netconf_send_request(client, request, response);
} 