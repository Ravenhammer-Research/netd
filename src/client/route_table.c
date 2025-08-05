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
#include <stdlib.h>

/**
 * Print route table from XML response
 * @param xml_response XML response string
 * @return 0 on success, -1 on failure
 */
int print_route_table(const char *xml_response)
{
    struct route_data routes[100]; /* Max 100 routes */
    int count;
    struct table_format fmt;
    
    debug_log(DEBUG_INFO, "Printing route table");
    
    if (!xml_response) {
        return -1;
    }

    /* Parse routes from XML */
    debug_log(DEBUG_DEBUG, "Parsing routes from XML");
    count = parse_routes_from_xml(xml_response, routes, 100);
    if (count < 0) {
        print_error("Failed to parse route XML");
        return -1;
    }
    debug_log(DEBUG_INFO, "Parsed %d routes from XML", count);

    /* Initialize table format */
    table_init(&fmt, "Route Table");
    table_add_column(&fmt, "Destination", 11);
    table_add_column(&fmt, "Gateway", 7);
    table_add_column(&fmt, "Interface", 9);

    /* First pass: calculate column widths */
    for (int i = 0; i < count; i++) {
        table_update_width(&fmt, 0, strlen(routes[i].destination));
        table_update_width(&fmt, 1, strlen(routes[i].gateway));
        table_update_width(&fmt, 2, strlen(routes[i].interface));
    }

    /* Print table header */
    table_print_header(&fmt);

    /* Print each route */
    for (int i = 0; i < count; i++) {
        table_print_row(&fmt, 
                       routes[i].destination,
                       routes[i].gateway[0] != '\0' ? routes[i].gateway : "-",
                       routes[i].interface[0] != '\0' ? routes[i].interface : "-");
    }

    /* Print table footer */
    char footer_text[64];
    snprintf(footer_text, sizeof(footer_text), "Total routes: %d", count);
    table_print_footer(&fmt, footer_text);

    return 0;
} 