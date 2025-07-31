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
#include <string.h>
#include <stdio.h>

/* Forward declarations for functions from other modules */
extern int find_tag_content(const char *xml, const char *tag, char *content, size_t max_len);
extern void print_separator(int width);

/**
 * Print route table from XML response
 * @param xml_response XML response string
 * @return 0 on success, -1 on failure
 */
int print_route_table(const char *xml_response)
{
    const char *pos = xml_response;
    char destination[64], next_hop[64], interface[64];
    int count = 0;
    
    if (!xml_response) {
        return -1;
    }

    /* Print table header */
    printf("\nRoute Table:\n");
    print_separator(70);
    printf("%-20s %-20s %-20s\n", "Destination", "Gateway", "Interface");
    print_separator(70);

    /* Find routes section */
    pos = strstr(xml_response, "<routes");
    if (!pos) {
        print_error("Could not find routes section in XML");
        return -1;
    }

    /* Find each route */
    while ((pos = strstr(pos, "<route>")) != NULL) {
        pos += 7; /* Skip "<route>" */
        
        /* Extract route data */
        if (find_tag_content(pos, "destination-prefix", destination, sizeof(destination)) == 0) {
            find_tag_content(pos, "next-hop-address", next_hop, sizeof(next_hop));
            find_tag_content(pos, "outgoing-interface", interface, sizeof(interface));

            printf("%-20s %-20s %-20s\n", 
                   destination,
                   next_hop[0] != '\0' ? next_hop : "-",
                   interface[0] != '\0' ? interface : "-");

            count++;
        }
        
        /* Move to next route */
        pos = strstr(pos, "</route>");
        if (pos) {
            pos += 8; /* Skip "</route>" */
        }
    }

    print_separator(70);
    printf("Total routes: %d\n\n", count);

    return 0;
} 