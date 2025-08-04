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

#include <sys/types.h>
#include <netinet/in.h>
#include "net.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <net/if_dl.h>
#include <time.h>
#include <stdarg.h>

static debug_level_t current_debug_level = DEBUG_NONE;

/**
 * Convert command type enum to string
 * @param type Command type
 * @return String representation
 */
const char *command_type_to_string(command_type_t type)
{
    switch (type) {
        case CMD_SET:
            return "set";
        case CMD_SHOW:
            return "show";
        case CMD_DELETE:
            return "delete";
        case CMD_COMMIT:
            return "commit";
        case CMD_SAVE:
            return "save";
        default:
            return "unknown";
    }
}

/**
 * Convert string to command type enum
 * @param str Command type string
 * @return Command type enum
 */
command_type_t command_type_from_string(const char *str)
{
    if (!str) {
        return CMD_UNKNOWN;
    }

    if (strcmp(str, "set") == 0) {
        return CMD_SET;
    } else if (strcmp(str, "show") == 0) {
        return CMD_SHOW;
    } else if (strcmp(str, "delete") == 0) {
        return CMD_DELETE;
    } else if (strcmp(str, "commit") == 0) {
        return CMD_COMMIT;
    } else if (strcmp(str, "save") == 0) {
        return CMD_SAVE;
    }

    return CMD_UNKNOWN;
}

/**
 * Convert object type enum to string
 * @param type Object type
 * @return String representation
 */
const char *object_type_to_string(object_type_t type)
{
    switch (type) {
        case OBJ_VRF:
            return "vrf";
        case OBJ_INTERFACE:
            return "interface";
        case OBJ_ROUTE:
            return "route";
        default:
            return "unknown";
    }
}

/**
 * Convert string to object type enum
 * @param str Object type string
 * @return Object type enum
 */
object_type_t object_type_from_string(const char *str)
{
    if (!str) {
        return OBJ_UNKNOWN;
    }

    if (strcmp(str, "vrf") == 0) {
        return OBJ_VRF;
    } else if (strcmp(str, "interface") == 0) {
        return OBJ_INTERFACE;
    } else if (strcmp(str, "route") == 0) {
        return OBJ_ROUTE;
    }

    return OBJ_UNKNOWN;
}

/**
 * Convert interface type enum to string
 * @param type Interface type
 * @return String representation
 */
const char *interface_type_to_string(interface_type_t type)
{
    switch (type) {
        case IF_TYPE_ETHERNET:
            return "ethernet";
        case IF_TYPE_WIRELESS:
            return "wireless80211";
        case IF_TYPE_EPAIR:
            return "epair";
        case IF_TYPE_GIF:
            return "gif";
        case IF_TYPE_GRE:
            return "gre";
        case IF_TYPE_LAGG:
            return "lagg";
        case IF_TYPE_LOOPBACK:
            return "lo";
        case IF_TYPE_OVPN:
            return "ovpn";
        case IF_TYPE_TUN:
            return "tun";
        case IF_TYPE_TAP:
            return "tap";
        case IF_TYPE_VLAN:
            return "vlan";
            case IF_TYPE_VXLAN:
        return "vxlan";
    case IF_TYPE_BRIDGE:
        return "bridge";
    default:
        return "unknown";
    }
}

/**
 * Convert string to interface type enum
 * @param str Interface type string
 * @return Interface type enum
 */
interface_type_t interface_type_from_string(const char *str)
{
    if (!str) {
        return IF_TYPE_UNKNOWN;
    }

    if (strcmp(str, "ethernet") == 0) {
        return IF_TYPE_ETHERNET;
    } else if (strcmp(str, "wireless") == 0 || strcmp(str, "wireless80211") == 0) {
        return IF_TYPE_WIRELESS;
    } else if (strcmp(str, "epair") == 0) {
        return IF_TYPE_EPAIR;
    } else if (strcmp(str, "gif") == 0) {
        return IF_TYPE_GIF;
    } else if (strcmp(str, "gre") == 0) {
        return IF_TYPE_GRE;
    } else if (strcmp(str, "lagg") == 0) {
        return IF_TYPE_LAGG;
    } else if (strcmp(str, "lo") == 0 || strcmp(str, "loopback") == 0) {
        return IF_TYPE_LOOPBACK;
    } else if (strcmp(str, "ovpn") == 0) {
        return IF_TYPE_OVPN;
    } else if (strcmp(str, "tun") == 0) {
        return IF_TYPE_TUN;
    } else if (strcmp(str, "tap") == 0) {
        return IF_TYPE_TAP;
    } else if (strcmp(str, "vlan") == 0) {
        return IF_TYPE_VLAN;
    } else if (strcmp(str, "vxlan") == 0) {
        return IF_TYPE_VXLAN;
    } else if (strcmp(str, "bridge") == 0) {
        return IF_TYPE_BRIDGE;
    }

    return IF_TYPE_UNKNOWN;
}

/**
 * Parse command line into command structure
 * @param line Command line string
 * @param cmd Command structure to fill
 * @return 0 on success, -1 on failure
 */
int parse_command(const char *line, command_t *cmd)
{
    char *line_copy, *token, *saveptr;
    int arg_count = 0;

    if (!line || !cmd) {
        return -1;
    }

    /* Initialize command structure */
    memset(cmd, 0, sizeof(*cmd));

    /* Make a copy of the line for parsing */
    line_copy = strdup(line);
    if (!line_copy) {
        return -1;
    }

    /* Parse first token (command type) */
    token = strtok_r(line_copy, " ", &saveptr);
    if (!token) {
        free(line_copy);
        return -1;
    }

    cmd->type = command_type_from_string(token);
    if (cmd->type == CMD_UNKNOWN) {
        print_error("Unknown command: %s", token);
        free(line_copy);
        return -1;
    }

    /* Parse remaining tokens */
    while ((token = strtok_r(NULL, " ", &saveptr)) != NULL && arg_count < 10) {
        strlcpy(cmd->args[arg_count], token, sizeof(cmd->args[arg_count]));
        arg_count++;
    }

    cmd->arg_count = arg_count;

    /* Determine object type based on arguments */
    if (arg_count > 0) {
        cmd->object = object_type_from_string(cmd->args[0]);
    }

    free(line_copy);
    return 0;
}

/**
 * Parse address string to sockaddr_storage
 * @param addr_str Address string (IPv4, IPv6, or hostname)
 * @param addr Output sockaddr_storage structure
 * @return 0 on success, -1 on failure
 */
int parse_address(const char *addr_str, struct sockaddr_storage *addr)
{
    struct addrinfo hints, *res;
    char *host_part, *port_part;
    char addr_copy[256];
    int ret;

    if (!addr_str || !addr) {
        return -1;
    }

    strlcpy(addr_copy, addr_str, sizeof(addr_copy));

    /* Split address and port */
    port_part = strchr(addr_copy, ':');
    if (port_part) {
        *port_part = '\0';
        port_part++;
    }

    host_part = addr_copy;

    /* Try to parse as IPv4 */
    if (inet_pton(AF_INET, host_part, &((struct sockaddr_in *)addr)->sin_addr) == 1) {
        addr->ss_family = AF_INET;
        ((struct sockaddr_in *)addr)->sin_port = port_part ? htons(atoi(port_part)) : 0;
        return 0;
    }

    /* Try to parse as IPv6 */
    if (inet_pton(AF_INET6, host_part, &((struct sockaddr_in6 *)addr)->sin6_addr) == 1) {
        addr->ss_family = AF_INET6;
        ((struct sockaddr_in6 *)addr)->sin6_port = port_part ? htons(atoi(port_part)) : 0;
        return 0;
    }

    /* Try to resolve as hostname */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    ret = getaddrinfo(host_part, port_part, &hints, &res);
    if (ret == 0) {
        memcpy(addr, res->ai_addr, res->ai_addrlen);
        freeaddrinfo(res);
        return 0;
    }

    return -1;
}

/**
 * Format sockaddr_storage to string
 * @param addr sockaddr_storage structure
 * @param str Output string buffer
 * @param len Buffer length
 * @return 0 on success, -1 on failure
 */
int format_address(const struct sockaddr_storage *addr, char *str, size_t len)
{
    if (!addr || !str || len == 0) {
        return -1;
    }

    switch (addr->ss_family) {
        case AF_INET: {
            struct sockaddr_in *sin = (struct sockaddr_in *)addr;
            if (inet_ntop(AF_INET, &sin->sin_addr, str, len) == NULL) {
                return -1;
            }
            break;
        }
        case AF_INET6: {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)addr;
            if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, len) == NULL) {
                return -1;
            }
            break;
        }
        case AF_LINK: {
            struct sockaddr_dl *sdl = (struct sockaddr_dl *)addr;
            snprintf(str, len, "link#%d", sdl->sdl_index);
            break;
        }
        default:
            return -1;
    }

    return 0;
}

/**
 * Get address family from sockaddr_storage
 * @param addr sockaddr_storage structure
 * @return Address family
 */
int get_address_family(const struct sockaddr_storage *addr)
{
    if (!addr) {
        return AF_UNSPEC;
    }
    return addr->ss_family;
}

/**
 * Get prefix length from netmask in sockaddr_storage
 * @param addr sockaddr_storage structure containing netmask
 * @return Prefix length
 */
int get_prefix_length(const struct sockaddr_storage *addr)
{
    int prefix_len = 0;
    unsigned char *bytes;
    int byte_count;

    if (!addr) {
        return -1;
    }

    switch (addr->ss_family) {
        case AF_INET:
            bytes = (unsigned char *)&((struct sockaddr_in *)addr)->sin_addr;
            byte_count = 4;
            break;
        case AF_INET6:
            bytes = (unsigned char *)&((struct sockaddr_in6 *)addr)->sin6_addr;
            byte_count = 16;
            break;
        default:
            return -1;
    }

    for (int i = 0; i < byte_count; i++) {
        unsigned char byte = bytes[i];
        if (byte == 0xff) {
            prefix_len += 8;
        } else if (byte == 0) {
            break;
        } else {
            while (byte & 0x80) {
                prefix_len++;
                byte <<= 1;
            }
            break;
        }
    }

    return prefix_len;
}

/**
 * Print error message
 * @param format Format string
 * @param ... Variable arguments
 */
void print_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

/**
 * Print success message
 * @param format Format string
 * @param ... Variable arguments
 */
void print_success(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    fprintf(stdout, "\n");
    va_end(args);
}

/**
 * Print info message
 * @param format Format string
 * @param ... Variable arguments
 */
void print_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    fprintf(stdout, "\n");
    va_end(args);
}

/**
 * Initialize debug logging
 * @param level Debug level
 */
void debug_init(debug_level_t level)
{
    current_debug_level = level;
}

/**
 * Log debug message
 * @param level Debug level
 * @param format Format string
 * @param ... Variable arguments
 */
void debug_log(debug_level_t level, const char *format, ...)
{
    va_list args;
    time_t now;
    struct tm *tm_info;
    char time_str[26];

    if (level > current_debug_level) {
        return;
    }

    /* Get current time */
    time(&now);
    tm_info = localtime(&now);
    strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    /* Print timestamp and message to stderr */
    fprintf(stderr, "[%s] [%s] ", time_str, 
            level == DEBUG_ERROR ? "ERROR" :
            level == DEBUG_WARNING ? "WARNING" :
            level == DEBUG_INFO ? "INFO" :
            level == DEBUG_DEBUG ? "DEBUG" :
            level == DEBUG_TRACE ? "TRACE" : "UNKNOWN");

    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
} 