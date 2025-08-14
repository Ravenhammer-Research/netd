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

#include <freebsd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>



uint8_t* parse_address_freebsd(const char *address_str, struct sockaddr_storage *addr) {
    if (!address_str || !addr) {
        return NULL;
    }

    memset(addr, 0, sizeof(struct sockaddr_storage));

    /* Try IPv4 first */
    struct in_addr addr4;
    if (inet_pton(AF_INET, address_str, &addr4) == 1) {
        struct sockaddr_in *sa4 = (struct sockaddr_in *)addr;
        sa4->sin_family = AF_INET;
        sa4->sin_len = sizeof(struct sockaddr_in);
        memcpy(&sa4->sin_addr.s_addr, &addr4.s_addr, 4);
        
        /* Return numeric representation - 4 bytes for IPv4 */
        uint8_t *result = malloc(4);
        if (result) {
            memcpy(result, &addr4.s_addr, 4);
        }
        return result;
    }

    /* Try IPv6 */
    struct in6_addr addr6;
    if (inet_pton(AF_INET6, address_str, &addr6) == 1) {
        struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)addr;
        sa6->sin6_family = AF_INET6;
        sa6->sin6_len = sizeof(struct sockaddr_in6);
        memcpy(&sa6->sin6_addr.s6_addr, &addr6.s6_addr, 16);
        
        /* Return numeric representation */
        uint8_t *result = malloc(16);
        if (result) {
            memcpy(result, &addr6.s6_addr, 16);
        }
        return result;
    }

    return NULL;
} 