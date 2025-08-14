#ifndef NETD_TYPES_H
#define NETD_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>
#include <string.h>
#include <stdlib.h>

/* Define constants if not already defined */
#ifndef MAX_VRF_NAME_LEN
#define MAX_VRF_NAME_LEN 64
#endif

#ifndef MAX_IFNAME_LEN
#define MAX_IFNAME_LEN 15
#endif

#ifndef MAX_GROUP_NAME_LEN
#define MAX_GROUP_NAME_LEN 15
#endif

#ifndef MAX_GROUPS_PER_IF
#define MAX_GROUPS_PER_IF 65536
#endif

#ifndef MAX_BRIDGE_MEMBERS
#define MAX_BRIDGE_MEMBERS 65536
#endif

#ifndef MAX_LAGG_MEMBERS
#define MAX_LAGG_MEMBERS 65536
#endif

#ifndef MAX_VRFS
#define MAX_VRFS 65536
#endif

#ifndef MAX_IPV4_ADDRESSES_PER_IF
#define MAX_IPV4_ADDRESSES_PER_IF 256
#endif

#ifndef MAX_IPV6_ADDRESSES_PER_IF
#define MAX_IPV6_ADDRESSES_PER_IF 256
#endif

#ifndef MAX_NETCONF_SESSIONS
#define MAX_NETCONF_SESSIONS 64
#endif



/* Address structure for IPv4/IPv6 */
typedef struct netd_address {
  uint8_t address[16];  /* IPv4 or IPv6 address in network byte order */
  uint8_t family;       /* AF_INET or AF_INET6 */
  uint8_t prefixlen;    /* Network prefix length */
  uint8_t mask[16];     /* Network mask */
} netd_address_t;

/* VRF types */
typedef enum : uint8_t {
  NETD_VRF_TYPE_LITE = 0       /* VRF-lite (software-based) */
} netd_vrf_type_t;

/* MIB (Management Information Base) structure */
typedef struct netd_mib {
  char key[64];        /* MIB key/name */
  char value[256];     /* MIB value */
  char desc[512];      /* MIB description */
  TAILQ_ENTRY(netd_mib) entries;
} netd_mib_t;

/* Route flags - matching FreeBSD routing flags */
typedef enum : uint32_t {
  NETD_ROUTE_FLAG_NONE = 0x0000,
  NETD_ROUTE_FLAG_XRESOLVE = 0x0001,   /* Emit mesg on use (for external lookup) */
  NETD_ROUTE_FLAG_GATEWAY = 0x0002,    /* Destination is a gateway */
  NETD_ROUTE_FLAG_STATIC = 0x0004,     /* Manually added route */
  NETD_ROUTE_FLAG_REJECT = 0x0008,     /* Emit an ICMP unreachable when matched */
  NETD_ROUTE_FLAG_BLACKHOLE = 0x0010,  /* Silently discard pkts (during updates) */
  NETD_ROUTE_FLAG_PROTO1 = 0x0020,     /* Protocol specific routing flag #1 */
  NETD_ROUTE_FLAG_PROTO2 = 0x0040      /* Protocol specific routing flag #2 */
} netd_route_flag_t;

/* Interface flags - matching FreeBSD interface flags */
typedef enum : uint32_t {
  NETD_IF_FLAG_UP = 0x00000001,              /* Interface is up */
  NETD_IF_FLAG_BROADCAST = 0x00000002,       /* Interface supports broadcast */
  NETD_IF_FLAG_DEBUG = 0x00000004,           /* Turn on debugging */
  NETD_IF_FLAG_LOOPBACK = 0x00000008,        /* Interface is a loopback */
  NETD_IF_FLAG_POINTOPOINT = 0x00000010,     /* Interface is point-to-point link */
  NETD_IF_FLAG_SMART = 0x00000020,           /* Interface supports error recovery */
  NETD_IF_FLAG_RUNNING = 0x00000040,         /* Resources allocated */
  NETD_IF_FLAG_NOARP = 0x00000080,           /* No address resolution protocol */
  NETD_IF_FLAG_PROMISC = 0x00000100,         /* Interface receives all packets */
  NETD_IF_FLAG_ALLMULTI = 0x00000200,        /* Interface receives all multicast packets */
  NETD_IF_FLAG_OACTIVE = 0x00000400,         /* Transmission in progress */
  NETD_IF_FLAG_SIMPLEX = 0x00000800,         /* Interface cannot hear own transmissions */
  NETD_IF_FLAG_LINK0 = 0x00001000,           /* Per link layer defined bit */
  NETD_IF_FLAG_LINK1 = 0x00002000,           /* Per link layer defined bit */
  NETD_IF_FLAG_LINK2 = 0x00004000,           /* Per link layer defined bit */
  NETD_IF_FLAG_ALTPHYS = 0x00008000,         /* Hardware supports alternate physical connection */
  NETD_IF_FLAG_MULTICAST = 0x00010000,       /* Interface supports multicast */
  NETD_IF_FLAG_CANTCONFIG = 0x00020000,      /* Unconfigurable using ioctl(2) */
  NETD_IF_FLAG_PPROMISC = 0x00040000,        /* User-requested promiscuous mode */
  NETD_IF_FLAG_MONITOR = 0x00080000,         /* User-requested monitor mode */
  NETD_IF_FLAG_STATICARP = 0x00100000,       /* Static ARP */
  NETD_IF_FLAG_DYING = 0x00200000,           /* Interface is winding down */
  NETD_IF_FLAG_RENAMING = 0x00400000,        /* Interface is being renamed */
  NETD_IF_FLAG_NOGROUP = 0x00800000,         /* Interface does not support groups */
  NETD_IF_FLAG_ATTACHED = 0x01000000,        /* Interface is attached */
  NETD_IF_FLAG_ATTACHING = 0x02000000,       /* Interface is attaching */
  NETD_IF_FLAG_DETACHING = 0x04000000,       /* Interface is detaching */
  NETD_IF_FLAG_DETACHED = 0x08000000,        /* Interface is detached */
  NETD_IF_FLAG_DEAD = 0x10000000,            /* Interface is dead */
  NETD_IF_FLAG_LOWER_UP = 0x20000000,        /* Lower interface is up */
  NETD_IF_FLAG_DORMANT = 0x40000000,         /* Interface is dormant */
  NETD_IF_FLAG_ECHO = 0x80000000             /* Interface is echoing */
} netd_interface_flag_t;

/* IPv6 interface flags */
typedef enum : uint32_t {
  NETD_IF6_FLAG_ANYCAST = 0x00000001,        /* Interface is anycast */
  NETD_IF6_FLAG_TENTATIVE = 0x00000002,      /* Interface address is tentative */
  NETD_IF6_FLAG_DUPLICATED = 0x00000004,     /* Interface address is duplicated */
  NETD_IF6_FLAG_DETACHED = 0x00000008,       /* Interface is detached */
  NETD_IF6_FLAG_DEPRECATED = 0x00000010,     /* Interface address is deprecated */
  NETD_IF6_FLAG_NODAD = 0x00000020,          /* No DAD on this interface */
  NETD_IF6_FLAG_AUTOCONF = 0x00000040,       /* Interface address is autoconfigured */
  NETD_IF6_FLAG_TEMPORARY = 0x00000080,      /* Interface address is temporary */
  NETD_IF6_FLAG_NOPFX = 0x00000100,          /* Interface address has no prefix */
  NETD_IF6_FLAG_MANUAL = 0x00000200,         /* Interface address is manually configured */
  NETD_IF6_FLAG_DISABLED = 0x00000400,       /* Interface is disabled */
  NETD_IF6_FLAG_SECURED = 0x00000800,        /* Interface is secured */
  NETD_IF6_FLAG_IPV6_ONLY_MANUAL = 0x00001000 /* IPv6-only interface (manual) */
} netd_interface6_flag_t;

/* Interface types */
typedef enum : uint8_t {
  NETD_IF_TYPE_UNKNOWN = 0,
  NETD_IF_TYPE_ETHERNET,
  NETD_IF_TYPE_WIRELESS,
  NETD_IF_TYPE_EPAIR,
  NETD_IF_TYPE_GIF,
  NETD_IF_TYPE_GRE,
  NETD_IF_TYPE_LAGG,
  NETD_IF_TYPE_LOOPBACK,
  NETD_IF_TYPE_OVPN,
  NETD_IF_TYPE_TUN,
  NETD_IF_TYPE_TAP,
  NETD_IF_TYPE_VLAN,
  NETD_IF_TYPE_VXLAN,
  NETD_IF_TYPE_BRIDGE,
  NETD_IF_TYPE_ENC,
  NETD_IF_TYPE_IPSEC,
  NETD_IF_TYPE_PFLOG,
  NETD_IF_TYPE_PFSYNC,
  NETD_IF_TYPE_STF,
  NETD_IF_TYPE_CARP,      /* Common Address Redundancy Protocol */
  NETD_IF_TYPE_WG,        /* WireGuard VPN */
  NETD_IF_TYPE_MPLS,      /* MPLS interface */
  NETD_IF_TYPE_PPP,       /* Point-to-Point Protocol */
  NETD_IF_TYPE_SLIP,      /* Serial Line IP */
  NETD_IF_TYPE_ATM,       /* Asynchronous Transfer Mode */
  NETD_IF_TYPE_FDDI,      /* Fiber Distributed Data Interface */
  NETD_IF_TYPE_TOKENRING, /* Token Ring */
  NETD_IF_TYPE_ARC,       /* Attached Resource Computer Network */
  NETD_IF_TYPE_HDLC,      /* High-Level Data Link Control */
  NETD_IF_TYPE_FRAME      /* Frame Relay */
} netd_interface_type_t;

/* WiFi regulatory domain types */
typedef enum : uint8_t {
  NETD_WIFI_REGDOMAIN_FCC,
  NETD_WIFI_REGDOMAIN_ETSI,
  NETD_WIFI_REGDOMAIN_MKK,
  NETD_WIFI_REGDOMAIN_OTHER
} netd_wifi_regdomain_t;

/* WiFi country codes */
typedef enum : uint8_t {
  NETD_WIFI_COUNTRY_US,    /* United States */
  NETD_WIFI_COUNTRY_CA,    /* Canada */
  NETD_WIFI_COUNTRY_GB,    /* United Kingdom */
  NETD_WIFI_COUNTRY_DE,    /* Germany */
  NETD_WIFI_COUNTRY_FR,    /* France */
  NETD_WIFI_COUNTRY_JP,    /* Japan */
  NETD_WIFI_COUNTRY_CN,    /* China */
  NETD_WIFI_COUNTRY_AU,    /* Australia */
  NETD_WIFI_COUNTRY_BR,    /* Brazil */
  NETD_WIFI_COUNTRY_IN,    /* India */
  NETD_WIFI_COUNTRY_RU,    /* Russia */
  NETD_WIFI_COUNTRY_KR,    /* South Korea */
  NETD_WIFI_COUNTRY_MX,    /* Mexico */
  NETD_WIFI_COUNTRY_IT,    /* Italy */
  NETD_WIFI_COUNTRY_ES,    /* Spain */
  NETD_WIFI_COUNTRY_NL,    /* Netherlands */
  NETD_WIFI_COUNTRY_SE,    /* Sweden */
  NETD_WIFI_COUNTRY_NO,    /* Norway */
  NETD_WIFI_COUNTRY_DK,    /* Denmark */
  NETD_WIFI_COUNTRY_FI,    /* Finland */
  NETD_WIFI_COUNTRY_PL,    /* Poland */
  NETD_WIFI_COUNTRY_CZ,    /* Czech Republic */
  NETD_WIFI_COUNTRY_HU,    /* Hungary */
  NETD_WIFI_COUNTRY_AT,    /* Austria */
  NETD_WIFI_COUNTRY_CH,    /* Switzerland */
  NETD_WIFI_COUNTRY_BE,    /* Belgium */
  NETD_WIFI_COUNTRY_PT,    /* Portugal */
  NETD_WIFI_COUNTRY_IE,    /* Ireland */
  NETD_WIFI_COUNTRY_GR,    /* Greece */
  NETD_WIFI_COUNTRY_TR,    /* Turkey */
  NETD_WIFI_COUNTRY_IL,    /* Israel */
  NETD_WIFI_COUNTRY_ZA,    /* South Africa */
  NETD_WIFI_COUNTRY_EG,    /* Egypt */
  NETD_WIFI_COUNTRY_SA,    /* Saudi Arabia */
  NETD_WIFI_COUNTRY_AE,    /* United Arab Emirates */
  NETD_WIFI_COUNTRY_TH,    /* Thailand */
  NETD_WIFI_COUNTRY_SG,    /* Singapore */
  NETD_WIFI_COUNTRY_MY,    /* Malaysia */
  NETD_WIFI_COUNTRY_ID,    /* Indonesia */
  NETD_WIFI_COUNTRY_PH,    /* Philippines */
  NETD_WIFI_COUNTRY_VN,    /* Vietnam */
  NETD_WIFI_COUNTRY_NZ,    /* New Zealand */
  NETD_WIFI_COUNTRY_AR,    /* Argentina */
  NETD_WIFI_COUNTRY_CL,    /* Chile */
  NETD_WIFI_COUNTRY_CO,    /* Colombia */
  NETD_WIFI_COUNTRY_PE,    /* Peru */
  NETD_WIFI_COUNTRY_VE,    /* Venezuela */
  NETD_WIFI_COUNTRY_UA,    /* Ukraine */
  NETD_WIFI_COUNTRY_RO,    /* Romania */
  NETD_WIFI_COUNTRY_BG,    /* Bulgaria */
  NETD_WIFI_COUNTRY_HR,    /* Croatia */
  NETD_WIFI_COUNTRY_SI,    /* Slovenia */
  NETD_WIFI_COUNTRY_SK,    /* Slovakia */
  NETD_WIFI_COUNTRY_LT,    /* Lithuania */
  NETD_WIFI_COUNTRY_LV,    /* Latvia */
  NETD_WIFI_COUNTRY_EE,    /* Estonia */
  NETD_WIFI_COUNTRY_IS,    /* Iceland */
  NETD_WIFI_COUNTRY_LU,    /* Luxembourg */
  NETD_WIFI_COUNTRY_MT,    /* Malta */
  NETD_WIFI_COUNTRY_CY,    /* Cyprus */
  NETD_WIFI_COUNTRY_OTHER  /* Other/Unknown */
} netd_wifi_country_t;

/* WiFi authentication modes */
typedef enum : uint8_t {
  NETD_WIFI_AUTHMODE_OPEN,
  NETD_WIFI_AUTHMODE_WEP,
  NETD_WIFI_AUTHMODE_WPA,
  NETD_WIFI_AUTHMODE_WPA2,
  NETD_WIFI_AUTHMODE_WPA3
} netd_wifi_authmode_t;

/* WiFi privacy settings */
typedef enum : uint8_t {
  NETD_WIFI_PRIVACY_OFF,
  NETD_WIFI_PRIVACY_ON
} netd_wifi_privacy_t;

/* WiFi WEP modes */
typedef enum : uint8_t {
  NETD_WIFI_WEP_OFF,
  NETD_WIFI_WEP_ON,
  NETD_WIFI_WEP_MIXED
} netd_wifi_wepmode_t;

/* WiFi WME access categories */
typedef enum : uint8_t {
  NETD_WIFI_WME_AC_BE,  /* Best Effort */
  NETD_WIFI_WME_AC_BK,  /* Background */
  NETD_WIFI_WME_AC_VI,  /* Video */
  NETD_WIFI_WME_AC_VO   /* Voice */
} netd_wifi_wme_ac_t;

/* WiFi WME ACK policies */
typedef enum : uint8_t {
  NETD_WIFI_WME_ACK_NORMAL,
  NETD_WIFI_WME_ACK_DISABLED
} netd_wifi_wme_ack_t;

/* WiFi WME ACM policies */
typedef enum : uint8_t {
  NETD_WIFI_WME_ACM_DISABLED,
  NETD_WIFI_WME_ACM_ENABLED
} netd_wifi_wme_acm_t;

/* WiFi features */
typedef enum : uint32_t {
  NETD_WIFI_FEATURE_NONE = 0,
  NETD_WIFI_FEATURE_WME = 1,
  NETD_WIFI_FEATURE_80211N = 2,
  NETD_WIFI_FEATURE_80211AC = 4,
  NETD_WIFI_FEATURE_80211AX = 8
} netd_wifi_features_t;

/* WiFi operating modes */
typedef enum : uint8_t {
  NETD_WIFI_MODE_STA,        /* Station mode */
  NETD_WIFI_MODE_AH_DEMO,    /* Ad-hoc demo mode */
  NETD_WIFI_MODE_IBSS,       /* Independent Basic Service Set */
  NETD_WIFI_MODE_AP,         /* Access Point mode */
  NETD_WIFI_MODE_WDS,        /* Wireless Distribution System */
  NETD_WIFI_MODE_TDMA,       /* Time Division Multiple Access */
  NETD_WIFI_MODE_MESH,       /* Mesh mode */
  NETD_WIFI_MODE_MONITOR     /* Monitor mode */
} netd_wifi_mode_t;

/* WiFi protection modes */
typedef enum : uint8_t {
  NETD_WIFI_PROT_OFF,        /* No protection */
  NETD_WIFI_PROT_CTS,        /* CTS to self */
  NETD_WIFI_PROT_RTSCTS      /* RTS/CTS */
} netd_wifi_prot_t;

/* WiFi HT protection modes */
typedef enum : uint8_t {
  NETD_WIFI_HTPROT_OFF,      /* No protection */
  NETD_WIFI_HTPROT_RTS       /* RTS/CTS */
} netd_wifi_htprot_t;

/* Mesh path selection protocols */
typedef enum : uint8_t {
  NETD_MESH_PATH_HWMP         /* Hybrid Wireless Mesh Protocol */
} netd_mesh_path_t;

/* Mesh link metric protocols */
typedef enum : uint8_t {
  NETD_MESH_METRIC_AIRTIME    /* Airtime link metric */
} netd_mesh_metric_t;

/* HWM root modes */
typedef enum : uint8_t {
  NETD_HWM_ROOT_DISABLED,
  NETD_HWM_ROOT_NORMAL,
  NETD_HWM_ROOT_PROACTIVE,
  NETD_HWM_ROOT_RANN
} netd_hwm_root_mode_t;

/* LAGG protocols */
typedef enum : uint8_t {
  NETD_LAGG_PROTO_FAILOVER,  /* Failover */
  NETD_LAGG_PROTO_LACP,      /* Link Aggregation Control Protocol */
  NETD_LAGG_PROTO_LOADBALANCE, /* Load Balance */
  NETD_LAGG_PROTO_ROUNDROBIN,  /* Round Robin */
  NETD_LAGG_PROTO_BROADCAST,   /* Broadcast */
  NETD_LAGG_PROTO_NONE         /* None */
} netd_lagg_proto_t;

/* LAGG hash options */
typedef enum : uint32_t {
  NETD_LAGG_HASH_L2 = 0x01,    /* Layer 2 (MAC) */
  NETD_LAGG_HASH_L3 = 0x02,    /* Layer 3 (IP) */
  NETD_LAGG_HASH_L4 = 0x04     /* Layer 4 (TCP/UDP) */
} netd_lagg_hash_t;

/* CARP states */
typedef enum : uint8_t {
  NETD_CARP_STATE_INIT,      /* Initial state */
  NETD_CARP_STATE_BACKUP,    /* Backup state */
  NETD_CARP_STATE_MASTER     /* Master state */
} netd_carp_state_t;

/* VLAN protocols */
typedef enum : uint8_t {
  NETD_VLAN_PROTO_8021Q,     /* IEEE 802.1Q */
  NETD_VLAN_PROTO_8021AD,    /* IEEE 802.1ad (Q-in-Q) */
  NETD_VLAN_PROTO_QINQ       /* Q-in-Q (alias for 802.1ad) */
} netd_vlan_proto_t;

/* Bridge protocols */
typedef enum : uint8_t {
  NETD_BRIDGE_PROTO_STP,     /* Spanning Tree Protocol */
  NETD_BRIDGE_PROTO_RSTP,    /* Rapid Spanning Tree Protocol */
  NETD_BRIDGE_PROTO_MSTP     /* Multiple Spanning Tree Protocol */
} netd_bridge_proto_t;

/* Bridge port states */
typedef enum : uint8_t {
  NETD_BRIDGE_PORT_DISABLED,
  NETD_BRIDGE_PORT_LISTENING,
  NETD_BRIDGE_PORT_LEARNING,
  NETD_BRIDGE_PORT_FORWARDING,
  NETD_BRIDGE_PORT_BLOCKING
} netd_bridge_port_state_t;

/* Bridge port types */
typedef enum : uint8_t {
  NETD_BRIDGE_PORT_EDGE,
  NETD_BRIDGE_PORT_P2P,
  NETD_BRIDGE_PORT_NORMAL
} netd_bridge_port_type_t;



/* Data structures */
typedef struct netd_vrf {
  uint32_t fib_number;
  netd_vrf_type_t type;        /* VRF type */
  char name[MAX_VRF_NAME_LEN];
  char description[256];
  TAILQ_ENTRY(netd_vrf) entries;
} netd_vrf_t;

/* TAILQ structures for capacity-constrained lists */
typedef struct netd_interface_group {
  char name[MAX_GROUP_NAME_LEN];
  TAILQ_ENTRY(netd_interface_group) entries;
} netd_interface_group_t;

/* Capacity-constrained TAILQ heads */
typedef struct netd_interface_groups {
  TAILQ_HEAD(netd_interface_groups_head, netd_interface_group) head;
  uint32_t count;
  uint32_t max_count;
} netd_interface_groups_t;

/* Base interface structure */
typedef struct netd_interface {
  char name[MAX_IFNAME_LEN];
  netd_interface_type_t type;
  uint32_t fib;
  netd_interface_groups_t groups;   /* Interface groups as TAILQ */
  uint32_t mtu;
  netd_interface_flag_t flags;      /* IPv4 interface flags */
  netd_interface6_flag_t flags6;    /* IPv6 interface flags */
  netd_address_t addresses[MAX_IPV4_ADDRESSES_PER_IF];  /* IPv4 addresses */
  netd_address_t addresses6[MAX_IPV6_ADDRESSES_PER_IF]; /* IPv6 addresses */
  TAILQ_ENTRY(netd_interface) entries;
} netd_interface_t;

typedef struct netd_bridge_member {
  netd_interface_t *interface;
  TAILQ_ENTRY(netd_bridge_member) entries;
} netd_bridge_member_t;

typedef struct netd_lagg_member {
  netd_interface_t *interface;
  TAILQ_ENTRY(netd_lagg_member) entries;
} netd_lagg_member_t;

typedef struct netd_bridge_members {
  TAILQ_HEAD(netd_bridge_members_head, netd_bridge_member) head;
  uint32_t count;
  uint32_t max_count;
} netd_bridge_members_t;

typedef struct netd_lagg_members {
  TAILQ_HEAD(netd_lagg_members_head, netd_lagg_member) head;
  uint32_t count;
  uint32_t max_count;
} netd_lagg_members_t;

/* Bridge-specific data structure */
typedef struct netd_bridge {
  netd_interface_t base;  /* Extends netd_interface_t */
  netd_bridge_members_t members;  /* Bridge member interfaces as TAILQ */
  uint32_t timeout;
  netd_bridge_proto_t protocol;
  TAILQ_ENTRY(netd_bridge) entries;
} netd_bridge_t;

/* VLAN-specific data structure */
typedef struct netd_vlan {
  netd_interface_t base;  /* Extends netd_interface_t */
  uint16_t vlan_id;          /* VLAN ID */
  netd_vlan_proto_t vlan_proto;  /* VLAN protocol */
  uint8_t vlan_pcp;         /* VLAN Priority Code Point */
  netd_interface_t *vlan_parent; /* Parent interface */
  TAILQ_ENTRY(netd_vlan) entries;
} netd_vlan_t;

/* WiFi-specific data structure */
typedef struct netd_wifi {
  netd_interface_t base;  /* Extends netd_interface_t */
  netd_wifi_regdomain_t regdomain;   /* Regulatory domain */
  netd_wifi_country_t country;       /* Country code */
  netd_wifi_authmode_t authmode;    /* Authentication mode */
  netd_wifi_privacy_t privacy;      /* Privacy setting */
  int16_t txpower;          /* Transmit power in dBm (can be negative) */
  uint32_t bmiss;           /* Beacon miss threshold */
  uint32_t scanvalid;       /* Scan cache validity period */
  netd_wifi_features_t features;    /* Supported features */
  uint32_t bintval;         /* Beacon interval */
  netd_interface_t *parent; /* Parent interface */
  TAILQ_ENTRY(netd_wifi) entries;
} netd_wifi_t;

/* E-pair specific data structure */
typedef struct netd_epair {
  netd_interface_t base;  /* Extends netd_interface_t */
  netd_interface_t *peer; /* Peer interface */
  TAILQ_ENTRY(netd_epair) entries;
} netd_epair_t;

/* Base tunnel structure for common tunnel properties */
typedef struct netd_tunnel {
  netd_interface_t base;  /* Extends netd_interface_t */
  netd_address_t local;   /* Local tunnel endpoint */
  netd_address_t remote;  /* Remote tunnel endpoint */
  netd_vrf_t *tunnel_vrf; /* Tunnel VRF (if different from interface VRF) */
} netd_tunnel_t;

/* GIF-specific data structure */
typedef struct netd_gif {
  netd_tunnel_t base;     /* Extends netd_tunnel_t */
  uint16_t local_port;    /* Local port */
  uint16_t remote_port;   /* Remote port */
  TAILQ_ENTRY(netd_gif) entries;
} netd_gif_t;

/* VXLAN-specific data structure */
typedef struct netd_vxlan {
  netd_tunnel_t base;     /* Extends netd_tunnel_t */
  uint32_t vni;           /* VXLAN Network Identifier */
  uint16_t local_port;    /* Local port */
  uint16_t remote_port;   /* Remote port */
  netd_address_t mc_group;   /* Multicast group address */
  uint16_t port_range_low; /* Port range low */
  uint16_t port_range_high; /* Port range high */
  uint32_t timeout;       /* Forwarding table timeout */
  uint32_t max_addr;      /* Maximum forwarding table entries */
  uint8_t ttl;           /* TTL for encapsulated packets */
  bool learn;            /* Enable learning */
  netd_interface_t *dev; /* Device interface for multicast */
  bool accept_rev_ethip_ver; /* Accept reversed EtherIP version */
  bool ignore_source;    /* Ignore source address */
  bool send_rev_ethip_ver; /* Send reversed EtherIP version */
  TAILQ_ENTRY(netd_vxlan) entries;
} netd_vxlan_t;

/* GRE-specific data structure */
typedef struct netd_gre {
  netd_tunnel_t base;     /* Extends netd_tunnel_t */
  uint32_t gre_key;       /* GRE key for outgoing packets */
  TAILQ_ENTRY(netd_gre) entries;
} netd_gre_t;

typedef struct netd_route {
  netd_address_t destination;  /* Destination address */
  netd_address_t gateway;      /* Gateway address */
  netd_address_t netmask;      /* Network mask */
  netd_interface_t *interface; /* Interface */
  netd_vrf_t *vrf;            /* VRF */
  uint32_t expire;            /* Route expiration time */
  netd_interface_t *scope_interface; /* Scope interface */
  netd_route_flag_t flags;    /* Route flags */
  TAILQ_ENTRY(netd_route) entries;
} netd_route_t;

/* LAGG-specific data structure */
typedef struct netd_lagg {
  netd_interface_t base;  /* Extends netd_interface_t */
  netd_lagg_proto_t lagg_proto;       /* LAGG protocol */
  netd_lagg_members_t members;  /* LAGG member interfaces as TAILQ */
  netd_lagg_hash_t hash_options;
  bool use_flowid;
  bool use_numa;
  bool lacp_fast_timeout;
  bool lacp_strict;
  uint32_t rr_limit;
  TAILQ_ENTRY(netd_lagg) entries;
} netd_lagg_t;

/* CARP-specific data structure */
typedef struct netd_carp {
  netd_interface_t base;  /* Extends netd_interface_t */
  uint8_t vhid;           /* Virtual Host ID */
  uint8_t advbase;        /* Advertisement base interval */
  uint8_t advskew;        /* Advertisement skew */
  char passphrase[64];    /* Authentication passphrase */
  netd_carp_state_t state; /* Current CARP state */
  netd_address_t peer;    /* Peer address */
  bool multicast;         /* Use multicast (default) */
  TAILQ_ENTRY(netd_carp) entries;
} netd_carp_t;

/* WireGuard-specific data structure */
typedef struct netd_wg {
  netd_interface_t base;  /* Extends netd_interface_t */
  uint16_t listen_port;   /* Listen port */
  uint32_t fwmark;        /* Firewall mark */
  uint32_t private_key[8]; /* Private key (256-bit) */
  uint32_t public_key[8];  /* Public key (256-bit) */
  TAILQ_ENTRY(netd_wg) entries;
} netd_wg_t;

/* Function declarations for capacity-constrained TAILQ operations */

/* Initialize capacity-constrained TAILQ structures */
void netd_interface_groups_init(netd_interface_groups_t *groups);
void netd_bridge_members_init(netd_bridge_members_t *members);
void netd_lagg_members_init(netd_lagg_members_t *members);

/* Add operations (FIFO append) */
bool netd_interface_groups_add(netd_interface_groups_t *groups, const char *group_name);
bool netd_bridge_members_add(netd_bridge_members_t *members, netd_interface_t *interface);
bool netd_lagg_members_add(netd_lagg_members_t *members, netd_interface_t *interface);

/* Remove operations (FIFO pop from head) */
netd_interface_group_t *netd_interface_groups_remove(netd_interface_groups_t *groups);
netd_bridge_member_t *netd_bridge_members_remove(netd_bridge_members_t *members);
netd_lagg_member_t *netd_lagg_members_remove(netd_lagg_members_t *members);

/* Find operations */
netd_interface_group_t *netd_interface_groups_find(netd_interface_groups_t *groups, const char *group_name);
netd_bridge_member_t *netd_bridge_members_find(netd_bridge_members_t *members, netd_interface_t *interface);
netd_lagg_member_t *netd_lagg_members_find(netd_lagg_members_t *members, netd_interface_t *interface);

/* Remove specific item operations */
bool netd_interface_groups_remove_item(netd_interface_groups_t *groups, const char *group_name);
bool netd_bridge_members_remove_item(netd_bridge_members_t *members, netd_interface_t *interface);
bool netd_lagg_members_remove_item(netd_lagg_members_t *members, netd_interface_t *interface);

/* Clear operations */
void netd_interface_groups_clear(netd_interface_groups_t *groups);
void netd_bridge_members_clear(netd_bridge_members_t *members);
void netd_lagg_members_clear(netd_lagg_members_t *members);

#endif /* NETD_TYPES_H */ 