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

#include <netd.h>
#include <interface.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/un.h>
#include <net/if.h>
#include <net/if_dl.h> // For struct sockaddr_dl
#include <net/if_bridgevar.h>
#include <net/if_vlan_var.h>
#include <net80211/ieee80211_ioctl.h>
#include <sys/sockio.h> // For SIOCSIFFIB
#include <net/if_mib.h> // For struct ifmibdata
#include <ifaddrs.h> // For getifaddrs, freeifaddrs, struct ifaddrs
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/queue.h>
#include <sys/sysctl.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if_types.h>
#include <net/if_var.h>
#include <time.h>
#include <sys/module.h>
#include <sys/linker.h>

/**
 * Get interface operational status based on flags
 * @param flags Interface flags
 * @return "up" if IFF_RUNNING is set, "down" otherwise
 */
const char *freebsd_get_interface_oper_status(int flags) {
  return (flags & IFF_RUNNING) ? "up" : "down";
}

/**
 * Check if interface exists in system
 * @param name Interface name
 * @return true if exists, false otherwise
 */
bool freebsd_interface_exists(const char *name) {
  int sock;
  struct ifreq ifr;

  if (!name) {
    return false;
  }

  /* Create socket for ioctl */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return false;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Try to get interface flags - if it exists, this will succeed */
  bool exists = (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0);

  close(sock);
  return exists;
}

/**
 * Get interface FIB
 * @param name Interface name
 * @param fib Pointer to store FIB number
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_get_fib(const char *name, uint32_t *fib) {
  int sock;
  struct ifreq ifr;

  if (!name || !fib) {
    return -1;
  }

  /* Create socket for ioctl */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(ERROR, "Failed to create socket for FIB query: %s",
              strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Get FIB */
  if (ioctl(sock, SIOCGIFFIB, &ifr) < 0) {
    debug_log(ERROR, "Failed to get FIB for interface %s: %s", name,
              strerror(errno));
    close(sock);
    return -1;
  }

  *fib = ifr.ifr_fib;
  close(sock);
  return 0;
}

/**
 * Get interface MTU
 * @param name Interface name
 * @param mtu Pointer to store MTU value
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_get_mtu(const char *name, uint32_t *mtu) {
  int sock;
  struct ifreq ifr;

  if (!name || !mtu) {
    return -1;
  }

  /* Create socket for ioctl */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(ERROR, "Failed to create socket for MTU query: %s",
              strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Get MTU */
  if (ioctl(sock, SIOCGIFMTU, &ifr) < 0) {
    debug_log(ERROR, "Failed to get MTU for interface %s: %s", name,
              strerror(errno));
    close(sock);
    return -1;
  }

  *mtu = ifr.ifr_mtu;
  close(sock);
  return 0;
}

/**
 * Get interface groups
 * @param name Interface name
 * @param groups TAILQ structure to store group names
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_get_groups(const char *name, netd_interface_groups_t *groups) {
  int sock;
  struct ifgroupreq ifgr;
  struct ifg_req *ifg;
  size_t len;

  if (!name || !groups) {
    return -1;
  }

  /* Initialize the groups TAILQ if not already done */
  if (groups->count == 0) {
    netd_interface_groups_init(groups);
  }

  /* Create socket for ioctl */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(ERROR, "Failed to create socket for group query: %s",
              strerror(errno));
    return -1;
  }

  /* Set up interface group request */
  memset(&ifgr, 0, sizeof(ifgr));
  strlcpy(ifgr.ifgr_name, name, IFNAMSIZ);

  /* First call to get the required buffer size */
  if (ioctl(sock, SIOCGIFGROUP, (caddr_t)&ifgr) < 0) {
    debug_log(ERROR, "Failed to get group info for %s: %s", name,
              strerror(errno));
    close(sock);
    return -1;
  }

  len = ifgr.ifgr_len;
  if (len > 0) {
    /* Allocate memory for group list */
    ifgr.ifgr_groups = (struct ifg_req *)calloc(len / sizeof(struct ifg_req),
                                                sizeof(struct ifg_req));
    if (ifgr.ifgr_groups != NULL) {
      /* Second call to get the actual group data */
      if (ioctl(sock, SIOCGIFGROUP, (caddr_t)&ifgr) == 0) {
        /* Process each group */
        for (ifg = ifgr.ifgr_groups;
             ifg && len >= sizeof(*ifg); ifg++) {
          len -= sizeof(*ifg);
          /* Add group to TAILQ structure */
          if (!netd_interface_groups_add(groups, ifg->ifgrq_group)) {
            debug_log(DEBUG, "Failed to add group %s to interface %s (capacity limit reached)", 
                     ifg->ifgrq_group, name);
            break;
          }
        }
      } else {
        debug_log(ERROR, "Failed to get groups for %s: %s", name,
                  strerror(errno));
      }
      free(ifgr.ifgr_groups);
    }
  }

  close(sock);
  return 0;
}

/**
 * Enumerate all interfaces in the system
 * @param system_query Pointer to system query structure to populate
 * @return 0 on success, -1 on failure
 */
int freebsd_enumerate_interfaces(netd_system_query_t *system_query) {
  struct ifaddrs *ifaddrs, *ifa;
  netd_interface_t *interface;
  int ret;

  if (!system_query) {
    return -1;
  }

  /* Initialize the interface list */
  TAILQ_INIT(&system_query->interfaces);

  /* Get interface addresses */
  if (getifaddrs(&ifaddrs) == -1) {
    debug_log(ERROR, "Failed to get interface addresses: %s", strerror(errno));
    return -1;
  }

  /* Iterate through all interfaces */
  for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
    /* Skip if no address or if this is a duplicate interface name */
    if (!ifa->ifa_addr || !ifa->ifa_name) {
      continue;
    }

    /* Check if we already have this interface */
    TAILQ_FOREACH(interface, &system_query->interfaces, entries) {
      if (strcmp(interface->name, ifa->ifa_name) == 0) {
        break;
      }
    }

    /* If interface not found, create new one */
    if (!interface) {
      interface = calloc(1, sizeof(netd_interface_t));
      if (!interface) {
        debug_log(ERROR, "Failed to allocate memory for interface %s", ifa->ifa_name);
        continue;
      }

      /* Initialize interface */
      strlcpy(interface->name, ifa->ifa_name, MAX_IFNAME_LEN);
      interface->type = NETD_IF_TYPE_UNKNOWN; /* Will be determined later */
      interface->fib = 0; /* Default FIB */
      interface->mtu = 1500; /* Default MTU */
      interface->flags = 0;
      interface->flags6 = 0;

      /* Get interface flags */
      if (ifa->ifa_flags & IFF_UP) {
        interface->flags |= NETD_IF_FLAG_UP;
      }
      if (ifa->ifa_flags & IFF_RUNNING) {
        interface->flags |= NETD_IF_FLAG_RUNNING;
      }
      if (ifa->ifa_flags & IFF_LOOPBACK) {
        interface->flags |= NETD_IF_FLAG_LOOPBACK;
        interface->type = NETD_IF_TYPE_LOOPBACK;
      }
      if (ifa->ifa_flags & IFF_BROADCAST) {
        interface->flags |= NETD_IF_FLAG_BROADCAST;
      }
      if (ifa->ifa_flags & IFF_POINTOPOINT) {
        interface->flags |= NETD_IF_FLAG_POINTOPOINT;
      }
      if (ifa->ifa_flags & IFF_MULTICAST) {
        interface->flags |= NETD_IF_FLAG_MULTICAST;
      }

      /* Get MTU */
      ret = freebsd_interface_get_mtu(ifa->ifa_name, &interface->mtu);
      if (ret != 0) {
        debug_log(DEBUG, "Failed to get MTU for interface %s", ifa->ifa_name);
      }

      /* Initialize interface groups */
      netd_interface_groups_init(&interface->groups);
      
      /* Get groups */
      ret = freebsd_interface_get_groups(ifa->ifa_name, &interface->groups);
      if (ret != 0) {
        debug_log(DEBUG, "Failed to get groups for interface %s", ifa->ifa_name);
      }

      /* Add interface to list */
      TAILQ_INSERT_TAIL(&system_query->interfaces, interface, entries);
    }

    /* Add address to interface - for now just store the first address of each type */
    if (ifa->ifa_addr->sa_family == AF_INET) {
      struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
      netd_address_t *addr = &interface->addresses[0];
      
      addr->family = AF_INET;
      memcpy(addr->address, &sa->sin_addr, 4);
      addr->prefixlen = 24; /* Default prefix length */
    } else if (ifa->ifa_addr->sa_family == AF_INET6) {
      struct sockaddr_in6 *sa = (struct sockaddr_in6 *)ifa->ifa_addr;
      netd_address_t *addr = &interface->addresses6[0];
      
      addr->family = AF_INET6;
      memcpy(addr->address, &sa->sin6_addr, 16);
      addr->prefixlen = 64; /* Default prefix length */
    }
  }

  freeifaddrs(ifaddrs);
  return 0;
} 