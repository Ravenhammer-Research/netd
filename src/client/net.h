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

#ifndef NET_H
#define NET_H

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <libnetconf2/netconf.h>
#include <libyang/libyang.h>
#include "if_table.h"

/* Constants */
#define NETD_SOCKET_PATH "/var/run/netd.sock"

/* Command types */
typedef enum {
    CMD_UNKNOWN = 0,
    CMD_SET,
    CMD_SHOW,
    CMD_DELETE,
    CMD_COMMIT,
    CMD_SAVE
} command_type_t;

/* Object types */
typedef enum {
    OBJ_UNKNOWN = 0,
    OBJ_VRF,
    OBJ_INTERFACE,
    OBJ_ROUTE
} object_type_t;

/* Interface types */
typedef enum {
    IF_TYPE_UNKNOWN = 0,
    IF_TYPE_ETHERNET,
    IF_TYPE_WIRELESS,
    IF_TYPE_EPAIR,
    IF_TYPE_GIF,
    IF_TYPE_GRE,
    IF_TYPE_LAGG,
    IF_TYPE_LOOPBACK,
    IF_TYPE_OVPN,
    IF_TYPE_TUN,
    IF_TYPE_TAP,
    IF_TYPE_VLAN,
    IF_TYPE_VXLAN,
    IF_TYPE_BRIDGE
} interface_type_t;

/* VRF data structure */
struct vrf_data {
    char name[64];
    char description[256];
    int fib;
};

/* Route data structure */
struct route_data {
    char destination[64];
    char gateway[64];
    char interface[64];
};

/* Command structure */
typedef struct command {
    command_type_t type;
    object_type_t object;
    char args[10][64];  /* Up to 10 arguments, 64 chars each */
    int arg_count;
} command_t;

/* Transaction structure */
typedef struct transaction {
    command_t commands[100];  /* Up to 100 commands per transaction */
    int command_count;
    bool active;
} transaction_t;

/* Debug levels */
typedef enum {
    DEBUG_NONE = 0,
    DEBUG_ERROR = 1,
    DEBUG_WARNING = 2,
    DEBUG_INFO = 3,
    DEBUG_DEBUG = 4,
    DEBUG_TRACE = 5
} debug_level_t;

/* Client state */
typedef struct net_client {
    int socket_fd;
    struct ly_ctx *yang_ctx;
    bool connected;
    transaction_t transaction;
    debug_level_t debug_level;
} net_client_t;

/* Function declarations */

/* Client initialization and cleanup */
int client_init(net_client_t *client, bool interactive);
void client_cleanup(net_client_t *client);
int execute_save_command(net_client_t *client, const command_t *cmd);

/* Command parsing and execution */
int parse_command(const char *line, command_t *cmd);
int execute_command(net_client_t *client, const command_t *cmd);
int execute_set_command(net_client_t *client, const command_t *cmd);
int execute_show_command(net_client_t *client, const command_t *cmd);
int execute_delete_command(net_client_t *client, const command_t *cmd);

/* Transaction management */
int transaction_begin(net_client_t *client);
int transaction_commit(net_client_t *client);
int transaction_rollback(net_client_t *client);
int transaction_add_command(net_client_t *client, const command_t *cmd);

/* Interactive mode */
int interactive_mode(net_client_t *client);
void initialize_readline(void);
char *command_generator(const char *text, int state);
char **command_completion(const char *text, int start, int end);

/* NETCONF communication */
int netconf_connect(net_client_t *client);
void netconf_disconnect(net_client_t *client);
int netconf_send_request(net_client_t *client, const char *request, char **response);
int netconf_get_interfaces(net_client_t *client, char **response);
int netconf_get_vrfs(net_client_t *client, char **response);
int netconf_get_routes(net_client_t *client, uint32_t fib, int family, char **response);
int netconf_get_interface_groups(net_client_t *client, char **response);

/* YANG context management */
int yang_init_client(net_client_t *client);
void yang_cleanup_client(net_client_t *client);

/* Utility functions */
const char *command_type_to_string(command_type_t type);
command_type_t command_type_from_string(const char *str);
const char *object_type_to_string(object_type_t type);
object_type_t object_type_from_string(const char *str);
const char *interface_type_to_string(interface_type_t type);
interface_type_t interface_type_from_string(const char *str);
int parse_address(const char *addr_str, struct sockaddr_storage *addr);
int format_address(const struct sockaddr_storage *addr, char *str, size_t len);
int get_address_family(const struct sockaddr_storage *addr);
int get_prefix_length(const struct sockaddr_storage *addr);

/* Output functions */
void print_error(const char *format, ...);
void print_success(const char *format, ...);
void print_info(const char *format, ...);

/* XML utilities */
int find_tag_content(const char *xml, const char *tag, char *content, size_t max_len);
char* extract_xml_content(const char *xml, const char *tag, char *buffer, size_t max_len);
int parse_interfaces_from_xml(const char *xml, struct interface_data *interfaces, int max_interfaces);
int parse_vrfs_from_xml(const char *xml, struct vrf_data *vrfs, int max_vrfs);
int parse_routes_from_xml(const char *xml, struct route_data *routes, int max_routes);

/* Display utilities */


/* Table display functions */
int print_interface_table(const char *xml_response);
int print_interface_table_filtered(const char *xml_response, const char *group_name);
int print_interface_groups_summary(const char *xml_response);
int print_wlan_interface_groups_summary(const char *xml_response);
int print_wlan_interface_table(const char *xml_response);
int print_vrf_table(const char *xml_response);
int print_route_table(const char *xml_response);

/* Debug logging */
void debug_init(debug_level_t level);
void debug_log(debug_level_t level, const char *format, ...);

#endif /* NET_H */ 