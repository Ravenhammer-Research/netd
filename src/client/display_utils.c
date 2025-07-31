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

#include "net.h"
#include <stdio.h>
#include <net/if.h>

/**
 * Print a horizontal line for table formatting
 * @param width Total width of the line
 */
void print_separator(int width)
{
    for (int i = 0; i < width; i++) {
        printf("-");
    }
    printf("\n");
}

/**
 * Format interface flags into a readable string
 * @param flags Flag value
 * @param flag_str Buffer to store formatted flags
 * @param max_len Maximum length of flag buffer
 */
void format_interface_flags(int flags, char *flag_str, size_t max_len)
{
    (void)max_len; /* Suppress unused parameter warning */
    flag_str[0] = '\0';
    
    if (flags & IFF_UP) strcat(flag_str, "U");
    if (flags & IFF_BROADCAST) strcat(flag_str, "B");
    if (flags & IFF_RUNNING) strcat(flag_str, "R");
    if (flags & IFF_PROMISC) strcat(flag_str, "P");
    if (flags & IFF_MULTICAST) strcat(flag_str, "M");
    if (flags & IFF_LOOPBACK) strcat(flag_str, "L");
} 