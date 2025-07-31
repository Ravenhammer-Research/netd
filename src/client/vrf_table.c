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
 * Print VRF table from XML response
 * @param xml_response XML response string
 * @return 0 on success, -1 on failure
 */
int print_vrf_table(const char *xml_response)
{
    const char *pos = xml_response;
    char name[64], description[256];
    int count = 0;
    
    if (!xml_response) {
        return -1;
    }

    /* Print table header */
    printf("\nVRF Table:\n");
    print_separator(70);
    printf("%-15s %-8s %-35s\n", "Name", "FIB", "Description");
    print_separator(70);

    /* Find vrfs section */
    pos = strstr(xml_response, "<vrfs");
    if (!pos) {
        print_error("Could not find vrfs section in XML");
        return -1;
    }

    /* Find each VRF */
    while ((pos = strstr(pos, "<vrf>")) != NULL) {
        pos += 5; /* Skip "<vrf>" */
        
        /* Extract VRF data */
        if (find_tag_content(pos, "name", name, sizeof(name)) == 0) {
            find_tag_content(pos, "description", description, sizeof(description));
            
            /* Determine FIB number based on name */
            int fib = 0;
            if (strcmp(name, "default") != 0) {
                /* For non-default VRFs, we'd need to extract FIB from XML */
                /* For now, just show 0 for default */
            }

            printf("%-15s %-8d %-35s\n", 
                   name,
                   fib,
                   description[0] != '\0' ? description : "");

            count++;
        }
        
        /* Move to next VRF */
        pos = strstr(pos, "</vrf>");
        if (pos) {
            pos += 6; /* Skip "</vrf>" */
        }
    }

    print_separator(70);
    printf("Total VRFs: %d\n\n", count);

    return 0;
} 