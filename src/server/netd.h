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

#ifndef NETD_H
#define NETD_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>
#include <netinet/in.h>

/* Forward declarations for libyang structures */
struct ly_ctx;
struct lyd_node;

/* Socket address family constants */
#define AF_UNSPEC 0
#define AF_INET   2
#define AF_INET6  28

/* Constants */
#define NETD_SOCKET_PATH "/var/run/netd.sock"
#define NETD_CONFIG_FILE "/etc/netd.conf"
#define MAX_IFNAME_LEN 64
#define MAX_VRF_NAME_LEN 64
#define MAX_GROUP_NAME_LEN 64
#define MAX_GROUPS_PER_IF 10

/* Debug levels */
typedef enum {
    DEBUG_NONE = 0,
    DEBUG_ERROR,
    DEBUG_WARN,
    DEBUG_INFO,
    DEBUG_DEBUG,
    DEBUG_TRACE
} debug_level_t;

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

/* Route flags - use system definitions from net/route.h */

/* Data structures */
typedef struct vrf {
    uint32_t fib_number;
    char name[MAX_VRF_NAME_LEN];
    char description[256];
    TAILQ_ENTRY(vrf) entries;
} vrf_t;

typedef struct interface {
    char name[MAX_IFNAME_LEN];
    interface_type_t type;
    uint32_t fib;
    char groups[MAX_GROUPS_PER_IF][MAX_GROUP_NAME_LEN];
    int group_count;
    bool enabled;
    int mtu;
    int flags;
    char primary_address[64];  /* Primary IPv4 address */
    char primary_address6[64]; /* Primary IPv6 address */
    char alias_addresses[10][64];  /* Up to 10 IPv4 alias addresses */
    char alias_addresses6[10][64]; /* Up to 10 IPv6 alias addresses */
    int alias_count;
    int alias_count6;
    char bridge_members[64]; /* Bridge member interfaces as comma-separated string */
    TAILQ_ENTRY(interface) entries;
} interface_t;

typedef struct netd_route {
    struct sockaddr_storage destination;
    struct sockaddr_storage gateway;
    struct sockaddr_storage netmask;
    char interface[MAX_IFNAME_LEN];
    uint32_t fib;
    int flags;
    TAILQ_ENTRY(netd_route) entries;
} netd_route_t;

typedef struct pending_change {
    enum {
        CHANGE_VRF_CREATE,
        CHANGE_VRF_DELETE,
        CHANGE_INTERFACE_CREATE,
        CHANGE_INTERFACE_DELETE,
        CHANGE_INTERFACE_SET_FIB,
        CHANGE_INTERFACE_SET_ADDRESS,
        CHANGE_INTERFACE_DELETE_ADDRESS,
        CHANGE_INTERFACE_SET_MTU,
        CHANGE_INTERFACE_ADD_GROUP,
        CHANGE_INTERFACE_REMOVE_GROUP,
        CHANGE_ROUTE_ADD,
        CHANGE_ROUTE_DELETE
    } type;
    union {
        struct {
            char name[MAX_VRF_NAME_LEN];
            uint32_t fib;
        } vrf;
        struct {
            char name[MAX_IFNAME_LEN];
            interface_type_t type;
            uint32_t fib;
            char address[64];
            int family;
            int mtu;
            char group[MAX_GROUP_NAME_LEN];
        } interface;
        struct {
            uint32_t fib;
            char destination[64];
            char gateway[64];
            char interface[MAX_IFNAME_LEN];
            int flags;
        } route;
    } data;
    TAILQ_ENTRY(pending_change) entries;
} pending_change_t;

typedef struct netd_state {
    TAILQ_HEAD(vrf_list, vrf) vrfs;
    TAILQ_HEAD(interface_list, interface) interfaces;
    TAILQ_HEAD(route_list, netd_route) routes;
    TAILQ_HEAD(pending_list, pending_change) pending_changes;
    struct ly_ctx *yang_ctx;
    int socket_fd;
    debug_level_t debug_level;
    bool transaction_active;
} netd_state_t;

/* Function declarations */

/* Debug logging */
void debug_log(debug_level_t level, const char *format, ...);
void debug_init(debug_level_t level);

/* VRF management */
int vrf_create(netd_state_t *state, const char *name, uint32_t fib_number);
int vrf_delete(netd_state_t *state, const char *name);
vrf_t *vrf_find_by_name(netd_state_t *state, const char *name);
vrf_t *vrf_find_by_fib(netd_state_t *state, uint32_t fib_number);
int vrf_list(netd_state_t *state);
char *vrf_get_all(netd_state_t *state);

/* Interface management */
int interface_create(netd_state_t *state, const char *name, interface_type_t type);
int interface_delete(netd_state_t *state, const char *name);
int interface_set_fib(netd_state_t *state, const char *name, uint32_t fib);
int interface_add_group(netd_state_t *state, const char *name, const char *group);
int interface_remove_group(netd_state_t *state, const char *name, const char *group);
int interface_set_address(netd_state_t *state, const char *name, const char *address, int family);
int interface_delete_address(netd_state_t *state, const char *name, int family);
int interface_set_mtu(netd_state_t *state, const char *name, int mtu);
interface_t *interface_find(netd_state_t *state, const char *name);
int interface_list(netd_state_t *state, interface_type_t type);
int interface_enumerate_system(netd_state_t *state);
char *interface_get_all(netd_state_t *state);

/* Route management */
int route_add(netd_state_t *state, uint32_t fib, const char *destination, 
              const char *gateway, const char *interface, int flags);
int route_delete(netd_state_t *state, uint32_t fib, const char *destination);
int route_list(netd_state_t *state, uint32_t fib, int family);
int route_flush_fib(netd_state_t *state, uint32_t fib);
int route_enumerate_system(netd_state_t *state, uint32_t fib);
int route_clear_all(netd_state_t *state);
char *route_get_all(netd_state_t *state);

/* Configuration management */
int config_load(netd_state_t *state);
int config_save(netd_state_t *state);

/* Transaction management */
int transaction_begin(netd_state_t *state);
int transaction_commit(netd_state_t *state);
int transaction_rollback(netd_state_t *state);

/* YANG/Netconf functions */
int yang_init(netd_state_t *state);
void yang_cleanup(netd_state_t *state);
int yang_validate_xml(netd_state_t *state, const char *xml_data);
int yang_validate_config(netd_state_t *state, const char *xml_config);
int yang_validate_rpc(netd_state_t *state, const char *rpc_xml);
int yang_validate_leafrefs(netd_state_t *state, struct lyd_node *data_tree);
char *yang_get_validation_error(const struct ly_ctx *ctx);
int yang_validate_netd_operation(netd_state_t *state, const char *operation, const char *data);
bool yang_module_loaded(netd_state_t *state, const char *module_name);
int netconf_handle_request(netd_state_t *state, const char *request, char **response);

/* System interface functions */
int freebsd_interface_create(const char *name, interface_type_t type);
bool freebsd_interface_exists(const char *name);
int freebsd_interface_delete(const char *name);
int freebsd_interface_set_fib(const char *name, uint32_t fib);
int freebsd_interface_get_fib(const char *name, uint32_t *fib);
int freebsd_interface_set_address(const char *name, const char *address, int family);
int freebsd_interface_delete_address(const char *name, int family);
int freebsd_interface_set_mtu(const char *name, int mtu);
int freebsd_interface_get_mtu(const char *name, int *mtu);
int freebsd_interface_get_groups(const char *name, char (*groups)[MAX_GROUP_NAME_LEN], int max_groups, int *group_count);
int freebsd_get_bridge_members(const char *ifname, char *members, size_t members_size);
int freebsd_enumerate_interfaces(netd_state_t *state);
bool freebsd_is_hardware_interface(const char *name);
const char *freebsd_get_interface_oper_status(int flags);

/* Route functions */
int freebsd_route_add(uint32_t fib, const char *destination, const char *gateway, 
                      const char *interface, int flags);
int freebsd_route_delete(uint32_t fib, const char *destination);
int freebsd_route_list(uint32_t fib, int family);
int freebsd_route_enumerate_system(netd_state_t *state, uint32_t fib);

/* Utility functions */
const char *interface_type_to_string(interface_type_t type);
interface_type_t interface_type_from_string(const char *str);
bool is_valid_interface_name(const char *name);
bool is_valid_vrf_name(const char *name);
bool is_valid_fib_number(uint32_t fib);
int parse_address(const char *addr_str, struct sockaddr_storage *addr);
int format_address(const struct sockaddr_storage *addr, char *str, size_t len);
int get_address_family(const struct sockaddr_storage *addr);
int get_prefix_length(const struct sockaddr_storage *addr);

#endif /* NETD_H */ 