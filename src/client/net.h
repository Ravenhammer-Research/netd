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

#ifndef NET_H
#define NET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <debug.h>


/* Constants */
#define NETD_SOCKET_PATH "/var/run/netd.sock"
#define NETCONF_SOCKET_PATH "/var/run/netd.sock"
#define MAX_COLUMNS 16

/* Maximum number of VRFs - FreeBSD supports up to 65536 FIBs (0-65535) */
#define MAX_VRFS 65536

/* Command types */
typedef enum {
  CMD_UNKNOWN = 0,
  CMD_SET,
  CMD_SHOW,
  CMD_DELETE,
  CMD_COMMIT,
  CMD_SAVE,
  CMD_QUIT,
  CMD_EXIT,
  CMD_HELP,
  CMD_CLEAR,
  CMD_ROLLBACK
} command_type_t;

/* Object types */
typedef enum {
  OBJ_UNKNOWN = 0,
  OBJ_VRF,
  OBJ_INTERFACE,
  OBJ_ROUTE,
  OBJ_NONE
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
  int prefix_length;
  char scope_interface[64];
  int flags;
  int expire;
};

/* Interface data structure */
/* Base interface data structure */
struct interface_data {
  char name[64];
  char type[16];  /* Interface type: ethernet, vlan, vxlan, etc. */
  char enabled[16];
  char fib[16];
  char mtu[16];
  char flags[16];
  char addr[64];
  char addr6[64];
  char addr_list[10][64];  /* All IPv4 addresses */
  char addr6_list[10][64]; /* All IPv6 addresses */
  int addr_count;          /* Number of IPv4 addresses */
  int addr6_count;         /* Number of IPv6 addresses */
  char groups[256];
};

/* Bridge interface data - extends base interface */
struct bridge_interface_data {
  struct interface_data base;
  char bridge_members[256];
  int bridge_maxaddr;
  int bridge_timeout;
  char bridge_protocol[16];
};

/* LAGG interface data - extends base interface */
struct lagg_interface_data {
  struct interface_data base;
  char lagg_members[256];
  char lagg_protocol[16];
  int lagg_count;
  char lagg_ports[8][64];  /* Up to 8 LAGG ports */
};

/* VLAN interface data - extends base interface */
struct vlan_interface_data {
  struct interface_data base;
  int vlan_id;
  char vlan_proto[16];
  int vlan_pcp;
  char vlan_parent[64];
  char vlan_description[256];
};

/* VXLAN interface data - extends base interface */
struct vxlan_interface_data {
  struct interface_data base;
  int vni;  /* VXLAN Network Identifier */
  char vxlan_local[64];
  char vxlan_remote[64];
  char vxlan_dev[64];
  int vxlan_ttl;
  char vxlan_group[64];
};

/* WiFi interface data - extends base interface */
struct wifi_interface_data {
  struct interface_data base;
  char ssid[64];
  char channel[8];
  char frequency[16];
  char txpower[8];
  char mode[16]; /* station, ap, monitor, etc. */
  char wifi_regdomain[16];
  char wifi_country[8];
  char wifi_authmode[16];
  char wifi_privacy[8];
  int wifi_txpower;
  int wifi_bmiss;
  int wifi_scanvalid;
  char wifi_features[64];
  int wifi_bintval;
  char wifi_parent[64];
};



/* Interface table column widths structure */
struct if_table_widths {
  int name_width;
  int status_width;
  int vrf_width;
  int tunvrf_width;
  int mtu_width;
  int flags_width;
  int ipv4_width;
  int ipv6_width;
  int groups_width;
};

/* Table column structure */
struct table_column {
  const char *header;
  int width;
  int min_width;
};

/* Table format structure */
struct table_format {
  struct table_column columns[MAX_COLUMNS];
  int column_count;
  const char *title;
};

/* Command structure */
typedef struct command {
  command_type_t type;
  object_type_t object;
  char args[12][64]; /* Up to 12 arguments, 64 chars each */
  int arg_count;
} command_t;

/* Transaction structure */
typedef struct transaction {
  command_t commands[100]; /* Up to 100 commands per transaction */
  int command_count;
  bool active;
} transaction_t;

/* Client state */
typedef struct net_client {
  int socket_fd;
  struct ly_ctx *yang_ctx;
  bool connected;
  transaction_t transaction;
  debug_level_t debug_level;
} net_client_t;

/* Function declarations */

/* Error handling functions */
void print_error(const char *format, ...);

/* Client functions */
int client_init(net_client_t *client, bool interactive);
void client_cleanup(net_client_t *client);

/* Interactive mode functions */
void initialize_readline(void);
char *command_completion(const char *text, int state);
char **command_generator(const char *text, int start, int end);
int interactive_mode(net_client_t *client);

/* Include module headers */
#include <xml/xml.h>
#include <table/table.h>
#include <netconf/netconf.h>
#include <parser/utils.h>



/* Debug functions */


/* XML utilities */
/* XML parsing functions are now in xml/xml.h */

/* Table display functions */
/* Table functions are now in table/table.h */

/* Table utility functions are now in table/table.h */

/* Interface table helper functions are now in table/table.h */

/* WLAN interface functions are now in table/table.h */

/* Utility functions are now in parser/utils.h and netconf/netconf_utils.h */

#endif /* NET_H */