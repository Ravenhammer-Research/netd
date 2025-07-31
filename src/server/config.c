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
#include <fcntl.h>
#include <ifaddrs.h>

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

    fp = fopen(NETD_CONFIG_FILE, "r");
    if (!fp) {
        debug_log(DEBUG_INFO, "No configuration file found, enumerating existing system interfaces");
        /* Enumerate existing system interfaces when no config file exists */
        if (interface_enumerate_system(state) < 0) {
            debug_log(DEBUG_ERROR, "Failed to enumerate system interfaces");
            return -1;
        }
        debug_log(DEBUG_INFO, "System interface enumeration complete");
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

                if (interface_create(state, name, type) < 0) {
                    debug_log(DEBUG_WARN, "Line %d: failed to create interface %s", line_num, name);
                    continue;
                }

                /* Parse additional parameters */
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
    FILE *fp = NULL;
    vrf_t *vrf;
    struct ifaddrs *ifap = NULL, *ifa;
    int sock = -1;
    int ret = -1;

    if (!state) {
        return -1;
    }

    /* Create config directory if it doesn't exist */
    char *config_dir = strdup(NETD_CONFIG_FILE);
    char *last_slash = strrchr(config_dir, '/');
    if (last_slash) {
        *last_slash = '\0';
        mkdir(config_dir, 0755);
    }
    free(config_dir);

    fp = fopen(NETD_CONFIG_FILE, "w");
    if (!fp) {
        debug_log(DEBUG_ERROR, "Failed to open configuration file for writing: %s", strerror(errno));
        return -1;
    }

    debug_log(DEBUG_INFO, "Saving configuration to %s", NETD_CONFIG_FILE);

    /* Write header */
    fprintf(fp, "# netd configuration file\n");
    fprintf(fp, "# Generated automatically - do not edit manually\n\n");

    /* Write VRF configurations */
    TAILQ_FOREACH(vrf, &state->vrfs, entries) {
        fprintf(fp, "set vrf %s table %u\n", vrf->name, vrf->fib_number);
    }

    /* Get system interface information */
    if (getifaddrs(&ifap) != 0) {
        debug_log(DEBUG_ERROR, "Failed to get interface addresses: %s", strerror(errno));
        fclose(fp);
        return -1;
    }

    /* Create socket for ioctl calls */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for interface queries: %s", strerror(errno));
        freeifaddrs(ifap);
        fclose(fp);
        return -1;
    }

    /* Track processed interfaces to avoid duplicates */
    char **processed_interfaces = NULL;
    int processed_count = 0;
    int processed_capacity = 0;

    /* Write interface configurations from system state */
    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_name) {
            continue;
        }

        /* Skip if we've already processed this interface */
        bool already_processed = false;
        for (int i = 0; i < processed_count; i++) {
            if (strcmp(processed_interfaces[i], ifa->ifa_name) == 0) {
                already_processed = true;
                break;
            }
        }
        if (already_processed) {
            continue;
        }

        /* Add to processed list */
        if (processed_count >= processed_capacity) {
            processed_capacity = processed_capacity ? processed_capacity * 2 : 16;
            processed_interfaces = realloc(processed_interfaces, processed_capacity * sizeof(char*));
            if (!processed_interfaces) {
                debug_log(DEBUG_ERROR, "Failed to allocate memory for interface tracking");
                goto cleanup;
            }
        }
        processed_interfaces[processed_count++] = strdup(ifa->ifa_name);

        /* Determine interface type from name */
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
        } else if (strncmp(ifa->ifa_name, "lo", 2) == 0) {
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
        }

        /* Get interface FIB */
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strlcpy(ifr.ifr_name, ifa->ifa_name, sizeof(ifr.ifr_name));
        uint32_t fib = 0;
        if (ioctl(sock, SIOCGIFFIB, &ifr) == 0) {
            fib = ifr.ifr_fib;
        }

        /* Write interface configuration */
        fprintf(fp, "set interface %s %s", 
                interface_type_to_string(type), ifa->ifa_name);
        
        /* Write VRF assignment if not default */
        if (fib != 0) {
            vrf_t *vrf = vrf_find_by_fib(state, fib);
            if (vrf) {
                fprintf(fp, " vrf %s", vrf->name);
            }
        }

        fprintf(fp, "\n");
    }

    /* Write route configurations */
    netd_route_t *route;
    char addr_str[INET6_ADDRSTRLEN];
    TAILQ_FOREACH(route, &state->routes, entries) {
        vrf_t *vrf = vrf_find_by_fib(state, route->fib);
        const char *vrf_name = vrf ? vrf->name : "default";
        
        if (format_address(&route->destination, addr_str, sizeof(addr_str)) == 0) {
            int family = get_address_family(&route->destination);
            const char *family_str = (family == AF_INET) ? "inet" : "inet6";
            
            fprintf(fp, "set vrf %s %s static %s", vrf_name, family_str, addr_str);
            
            if (route->gateway.ss_family != AF_UNSPEC) {
                if (format_address(&route->gateway, addr_str, sizeof(addr_str)) == 0) {
                    fprintf(fp, " %s", addr_str);
                }
            } else if (route->flags & RTF_REJECT) {
                fprintf(fp, " reject");
            } else if (route->flags & RTF_BLACKHOLE) {
                fprintf(fp, " blackhole");
            }
            
            fprintf(fp, "\n");
        }
    }

    fclose(fp);
    debug_log(DEBUG_INFO, "Configuration saved successfully");
    ret = 0;

cleanup:
    /* Clean up resources */
    if (processed_interfaces) {
        for (int i = 0; i < processed_count; i++) {
            free(processed_interfaces[i]);
        }
        free(processed_interfaces);
    }
    if (sock >= 0) {
        close(sock);
    }
    if (ifap) {
        freeifaddrs(ifap);
    }
    if (ret != 0 && fp) {
        fclose(fp);
    }
    return ret;
} 