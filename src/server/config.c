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
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Load configuration from file
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int config_load(netd_state_t *state)
{
    FILE *fp;
    char line[1024];
    char *token, *saveptr;
    int line_num = 0;

    if (!state) {
        return -1;
    }

    /* Always enumerate system interfaces first to see what exists */
    debug_log(DEBUG_INFO, "Enumerating existing system interfaces");
    if (interface_enumerate_system(state) < 0) {
        debug_log(DEBUG_ERROR, "Failed to enumerate system interfaces");
        return -1;
    }
    
    /* Enumerate system routes */
    debug_log(DEBUG_INFO, "Enumerating existing system routes");
    if (route_enumerate_system(state, 0) < 0) {
        debug_log(DEBUG_ERROR, "Failed to enumerate system routes");
        return -1;
    }
    debug_log(DEBUG_INFO, "System route enumeration complete");

    /* Now try to load config if it exists */
    fp = fopen(NETD_CONFIG_FILE, "r");
    if (!fp) {
        debug_log(DEBUG_INFO, "No configuration file found");
        return 0;
    }

    debug_log(DEBUG_INFO, "Loading configuration from %s", NETD_CONFIG_FILE);

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        
        /* Skip comments and empty lines */
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\0') {
            continue;
        }

        /* Remove newline */
        line[strcspn(line, "\n")] = '\0';

        /* Parse command line */
        token = strtok_r(line, " ", &saveptr);
        if (!token) {
            continue;
        }

        if (strcmp(token, "set") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (!token) {
                debug_log(DEBUG_WARN, "Line %d: incomplete set command", line_num);
                continue;
            }

            if (strcmp(token, "vrf") == 0) {
                /* set vrf <name> table <fib> */
                char *name = strtok_r(NULL, " ", &saveptr);
                char *table = strtok_r(NULL, " ", &saveptr);
                char *fib_str = strtok_r(NULL, " ", &saveptr);

                if (!name || !table || !fib_str || strcmp(table, "table") != 0) {
                    debug_log(DEBUG_WARN, "Line %d: invalid vrf command format", line_num);
                    continue;
                }

                uint32_t fib = atoi(fib_str);
                if (vrf_create(state, name, fib) < 0) {
                    debug_log(DEBUG_WARN, "Line %d: failed to create VRF %s", line_num, name);
                }
            } else if (strcmp(token, "interface") == 0) {
                /* set interface <type> <name> [params] */
                char *type_str = strtok_r(NULL, " ", &saveptr);
                char *name = strtok_r(NULL, " ", &saveptr);

                if (!type_str || !name) {
                    debug_log(DEBUG_WARN, "Line %d: invalid interface command format", line_num);
                    continue;
                }

                interface_type_t type = interface_type_from_string(type_str);
                if (type == IF_TYPE_UNKNOWN) {
                    /* Try to infer type from interface name */
                    if (strncmp(name, "bridge", 6) == 0) {
                        type = IF_TYPE_BRIDGE;
                        debug_log(DEBUG_INFO, "Line %d: inferred bridge type for interface '%s'", line_num, name);
                    } else if (strncmp(name, "vlan", 4) == 0) {
                        type = IF_TYPE_VLAN;
                        debug_log(DEBUG_INFO, "Line %d: inferred vlan type for interface '%s'", line_num, name);
                    } else if (strncmp(name, "tap", 3) == 0) {
                        type = IF_TYPE_TAP;
                        debug_log(DEBUG_INFO, "Line %d: inferred tap type for interface '%s'", line_num, name);
                    } else if (strncmp(name, "tun", 3) == 0) {
                        type = IF_TYPE_TUN;
                        debug_log(DEBUG_INFO, "Line %d: inferred tun type for interface '%s'", line_num, name);
                    } else if (strncmp(name, "epair", 5) == 0) {
                        type = IF_TYPE_EPAIR;
                        debug_log(DEBUG_INFO, "Line %d: inferred epair type for interface '%s'", line_num, name);
                    } else if (strncmp(name, "gif", 3) == 0) {
                        type = IF_TYPE_GIF;
                        debug_log(DEBUG_INFO, "Line %d: inferred gif type for interface '%s'", line_num, name);
                    } else if (strncmp(name, "gre", 3) == 0) {
                        type = IF_TYPE_GRE;
                        debug_log(DEBUG_INFO, "Line %d: inferred gre type for interface '%s'", line_num, name);
                    } else if (strncmp(name, "lagg", 4) == 0) {
                        type = IF_TYPE_LAGG;
                        debug_log(DEBUG_INFO, "Line %d: inferred lagg type for interface '%s'", line_num, name);
                    } else if (strncmp(name, "vxlan", 5) == 0) {
                        type = IF_TYPE_VXLAN;
                        debug_log(DEBUG_INFO, "Line %d: inferred vxlan type for interface '%s'", line_num, name);
                    } else {
                        debug_log(DEBUG_WARN, "Line %d: unknown interface type '%s' for interface '%s'", line_num, type_str, name);
                        continue;
                    }
                }

                /* Check if interface already exists before creating */
                if (interface_find(state, name)) {
                    debug_log(DEBUG_INFO, "Line %d: interface %s already exists, skipping creation and configuration", line_num, name);
                    /* Skip all configuration parameters for existing interfaces */
                    while ((token = strtok_r(NULL, " ", &saveptr)) != NULL) {
                        /* Just consume tokens until end of line */
                    }
                } else {
                    if (interface_create(state, name, type) < 0) {
                        debug_log(DEBUG_WARN, "Line %d: failed to create interface %s", line_num, name);
                        continue;
                    }

                    /* Parse additional parameters only for newly created interfaces */
                    while ((token = strtok_r(NULL, " ", &saveptr)) != NULL) {
                    if (strcmp(token, "vrf") == 0) {
                        char *vrf_name = strtok_r(NULL, " ", &saveptr);
                        if (vrf_name) {
                            vrf_t *vrf = vrf_find_by_name(state, vrf_name);
                            if (vrf) {
                                interface_set_fib(state, name, vrf->fib_number);
                            }
                        }
                    } else if (strcmp(token, "address") == 0) {
                        char *family = strtok_r(NULL, " ", &saveptr);
                        char *address = strtok_r(NULL, " ", &saveptr);
                        if (family && address) {
                            int af = (strcmp(family, "inet") == 0) ? AF_INET : AF_INET6;
                            interface_set_address(state, name, address, af);
                        }
                    } else if (strcmp(token, "mtu") == 0) {
                        char *mtu_str = strtok_r(NULL, " ", &saveptr);
                        if (mtu_str) {
                            int mtu = atoi(mtu_str);
                            interface_set_mtu(state, name, mtu);
                        }
                    } else if (strcmp(token, "group") == 0) {
                        char *group = strtok_r(NULL, " ", &saveptr);
                        if (group) {
                            interface_add_group(state, name, group);
                        }
                    }
                }
                }
            }
        }
    }

    fclose(fp);
    debug_log(DEBUG_INFO, "Configuration loaded successfully");
    return 0;
}

/**
 * Save configuration to file
 * @param state Server state
 * @return 0 on success, -1 on failure
 */
int config_save(netd_state_t *state)
{
    if (!state) {
        return -1;
    }

    debug_log(DEBUG_INFO, "Configuration save requested (stubbed - no file written)");
    return 0;
} 