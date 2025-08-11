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

#ifndef NETD_H
#define NETD_H

#include <libyang/log.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/queue.h>
#include <debug.h>

/* Constants */
/* Maximum VRF name length - VRF names are typically short identifiers like "vrf1", "mgmt", etc. */
#define MAX_VRF_NAME_LEN 64

/* Maximum interface name length - FreeBSD interface names like "em0", "lagg0", "vlan100" are typically under 32 chars */
#define MAX_IFNAME_LEN 15

/* Maximum interface group name length - Group names like "lan", "wan", "dmz" are typically short */
#define MAX_GROUP_NAME_LEN 15

/* Maximum number of groups per interface - Most interfaces belong to 1-3 groups, rarely more than 8 */
#define MAX_GROUPS_PER_IF 65536

/* Maximum number of bridge members per bridge interface */
#define MAX_BRIDGE_MEMBERS 65536

/* Maximum number of LAGG members per LAGG interface */
#define MAX_LAGG_MEMBERS 65536

/* Maximum number of VRFs - FreeBSD supports up to 65536 FIBs (0-65535) */
#define MAX_VRFS 65536

/* Interface address structure */
typedef struct if_addr {
  char addr[64];
  int prefixlen;
} if_addr_t;

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



/* Route flags - simplified versions for our application */
typedef enum {
  ROUTE_FLAG_NONE = 0,
  ROUTE_FLAG_REJECT = 1,    /* Route rejects packets */
  ROUTE_FLAG_BLACKHOLE = 2  /* Route blackholes packets */
} route_flag_t;

/* Forward declarations for libyang structures */
struct ly_ctx;
struct lyd_node;

/* Socket address family constants */
#define AF_UNSPEC 0
#define AF_INET 2
#define AF_INET6 28

/* Constants */
#define NETD_SOCKET_PATH "/var/run/netd.sock"
#define NETD_CONFIG_FILE "/etc/netd.conf"

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
  if_addr_t addresses[11];  /* IPv4 addresses (primary at index 0, aliases at 1-10) */
  if_addr_t addresses6[11]; /* IPv6 addresses (primary at index 0, aliases at 1-10) */
  int address_count;             /* Total IPv4 addresses (0-11) */
  int address_count6;            /* Total IPv6 addresses (0-11) */
  TAILQ_ENTRY(interface) entries;
} interface_t;

/* Bridge-specific data structure */
typedef struct bridge {
  char name[MAX_IFNAME_LEN];
  char members[MAX_BRIDGE_MEMBERS][MAX_IFNAME_LEN]; /* Bridge member interfaces as array */
  int member_count; /* Number of bridge members */
  int maxaddr;
  int timeout;
  char protocol[16];
  TAILQ_ENTRY(bridge) entries;
} bridge_t;

/* VLAN-specific data structure */
typedef struct vlan {
  char name[MAX_IFNAME_LEN];
  int vlan_id;          /* VLAN ID */
  char vlan_proto[16];  /* VLAN protocol (e.g., "802.1q", "802.1ad") */
  int vlan_pcp;         /* VLAN Priority Code Point */
  char vlan_parent[64]; /* Parent interface name */
  TAILQ_ENTRY(vlan) entries;
} vlan_t;

/* WiFi-specific data structure */
typedef struct wifi {
  char name[MAX_IFNAME_LEN];
  char regdomain[16];   /* Regulatory domain (e.g., "FCC") */
  char country[8];      /* Country code (e.g., "US") */
  char authmode[16];    /* Authentication mode (e.g., "OPEN", "WPA") */
  char privacy[8];      /* Privacy setting (e.g., "OFF", "ON") */
  int txpower;          /* Transmit power in dBm */
  int bmiss;            /* Beacon miss threshold */
  int scanvalid;        /* Scan validity period */
  char features[64];    /* WiFi features (e.g., "wme") */
  int bintval;          /* Beacon interval */
  char parent[64];      /* Parent interface name */
  TAILQ_ENTRY(wifi) entries;
} wifi_t;

/* Epair-specific data structure */
typedef struct epair {
  char name[MAX_IFNAME_LEN];
  char peer_name[64];   /* Peer interface name */
  TAILQ_ENTRY(epair) entries;
} epair_t;

/* GIF-specific data structure */
typedef struct gif {
  char name[MAX_IFNAME_LEN];
  char tunnel_local[64];   /* Local tunnel endpoint address */
  char tunnel_remote[64];  /* Remote tunnel endpoint address */
  TAILQ_ENTRY(gif) entries;
} gif_t;

/* LAGG-specific data structure */
typedef struct lagg {
  char name[MAX_IFNAME_LEN];
  char lagg_proto[16];       /* LAGG protocol (e.g., "failover", "lacp", "roundrobin") */
  char members[MAX_LAGG_MEMBERS][MAX_IFNAME_LEN]; /* LAGG member interfaces as array */
  int member_count;     /* Number of LAGG members */
  TAILQ_ENTRY(lagg) entries;
} lagg_t;

typedef struct netd_route {
  struct sockaddr_storage destination;
  struct sockaddr_storage gateway;
  struct sockaddr_storage netmask;
  char interface[MAX_IFNAME_LEN];
  uint32_t fib;
  int flags;
  int prefix_length;        /* CIDR prefix length */
  char scope_interface[MAX_IFNAME_LEN]; /* Interface for scope (IPv6) */
  int expire;               /* Route expiration time */
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
  TAILQ_HEAD(bridge_list, bridge) bridges;
  TAILQ_HEAD(vlan_list, vlan) vlans;
  TAILQ_HEAD(wifi_list, wifi) wifis;
  TAILQ_HEAD(epair_list, epair) epairs;
  TAILQ_HEAD(gif_list, gif) gifs;
  TAILQ_HEAD(lagg_list, lagg) laggs;
  TAILQ_HEAD(route_list, netd_route) routes;
  TAILQ_HEAD(pending_list, pending_change) pending_changes;
  struct ly_ctx *yang_ctx;
  int socket_fd;
  debug_level_t debug_level;
  bool transaction_active;
} netd_state_t;

/* Function declarations */
/* Configuration management */
int config_load(netd_state_t *state);
int config_save(netd_state_t *state);

/* Transaction management */
int transaction_begin(netd_state_t *state);
int transaction_commit(netd_state_t *state);
int transaction_rollback(netd_state_t *state);
int add_pending_vrf_create(netd_state_t *state, const char *name, uint32_t fib);
int add_pending_vrf_delete(netd_state_t *state, const char *name);
int add_pending_route_add(netd_state_t *state, uint32_t fib,
                          const char *destination, const char *gateway,
                          const char *interface, int flags);
int add_pending_route_delete(netd_state_t *state, uint32_t fib,
                             const char *destination);
int add_pending_interface_create(netd_state_t *state, const char *name,
                                 interface_type_t type);
int add_pending_interface_set_fib(netd_state_t *state, const char *name,
                                  uint32_t fib);



int freebsd_get_vlan_info(const char *ifname, int *vlan_id, char *vlan_proto,
                          size_t proto_size, int *vlan_pcp, char *vlan_parent,
                          size_t parent_size);
int freebsd_get_wifi_info(const char *ifname, char *regdomain,
                          size_t regdomain_size, char *country,
                          size_t country_size, char *authmode,
                          size_t authmode_size, char *privacy,
                          size_t privacy_size, int *txpower, int *bmiss,
                          int *scanvalid, char *features, size_t features_size,
                          int *bintval, char *parent, size_t parent_size);

/* Utility functions */
const char *interface_type_to_string(interface_type_t type);
const char *interface_type_get_namespace(interface_type_t type);
interface_type_t interface_type_from_string(const char *str);
bool is_valid_interface_name(const char *name);
bool is_valid_vrf_name(const char *name);
uint32_t get_system_fib_count(void);
bool is_valid_fib_number(uint32_t fib);
int parse_address(const char *addr_str, struct sockaddr_storage *addr);
int format_address(const struct sockaddr_storage *addr, char *str, size_t len);
int get_address_family(const struct sockaddr_storage *addr);
int get_prefix_length(const struct sockaddr_storage *addr);

#endif /* NETD_H */