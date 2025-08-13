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

#include <system.h>
#include <string.h>
#include <arpa/inet.h>

int parse_address(uint8_t address_num[16], netd_address_t *addr) {
    if (!address_num || !addr) {
        return -1;
    }

    memset(addr, 0, sizeof(netd_address_t));

    /* Check if it's IPv4 (first 12 bytes are zero) */
    int is_ipv4 = 1;
    for (int i = 0; i < 12; i++) {
        if (address_num[i] != 0) {
            is_ipv4 = 0;
            break;
        }
    }

    if (is_ipv4) {
        addr->family = AF_INET;
        memcpy(addr->data, &address_num[12], 4);
        return 0;
    } else {
        /* IPv6 */
        addr->family = AF_INET6;
        memcpy(addr->data, address_num, 16);
        return 0;
    }
}

int address_to_string(const netd_address_t *addr, char *str, size_t len) {
    if (!addr || !str || len == 0) {
        return -1;
    }

    if (addr->family == AF_INET) {
        struct in_addr addr4;
        memcpy(&addr4.s_addr, addr->data, 4);
        return inet_ntop(AF_INET, &addr4, str, len) ? 0 : -1;
    } else if (addr->family == AF_INET6) {
        struct in6_addr addr6;
        memcpy(&addr6.s6_addr, addr->data, 16);
        return inet_ntop(AF_INET6, &addr6, str, len) ? 0 : -1;
    }

    return -1;
} 