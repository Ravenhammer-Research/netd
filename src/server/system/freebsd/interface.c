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

#include "netd.h"
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
 * Create a network interface
 * @param name Interface name
 * @param type Interface type
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_create(const char *name, interface_type_t type) {
  int sock;
  struct ifreq ifr;
  const char *type_str;

  if (!name) {
    return -1;
  }

  /* Get interface type string */
  type_str = interface_type_to_string(type);
  if (!type_str) {
    debug_log(DEBUG_ERROR, "NULL type string for interface %s", name);
    return -1;
  }
  if (strcmp(type_str, "unknown") == 0) {
    debug_log(DEBUG_ERROR, "Unknown interface type for %s", name);
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Creating interface %s of type %s", name, type_str);

  /* Create socket for ioctl */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for interface creation: %s",
              strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* For epair interfaces, we need to use the cloning mechanism */
  if (type == IF_TYPE_EPAIR) {
    /* For epair interfaces, we create the base name and the system creates both
     * a and b interfaces */
    /* First, try to load the epair module if it's not already loaded */
    int kld_id = kldload("if_epair");
    if (kld_id < 0 && errno != EEXIST) {
      debug_log(DEBUG_WARN,
                "Failed to load if_epair module: %s (continuing anyway)",
                strerror(errno));
    } else if (kld_id >= 0) {
      debug_log(DEBUG_INFO, "Loaded if_epair module (kld_id: %d)", kld_id);
    } else {
      debug_log(DEBUG_DEBUG, "if_epair module already loaded");
    }

    /* Check if the epair module is loaded */
    if (ioctl(sock, SIOCIFCREATE2, &ifr) < 0) {
      debug_log(DEBUG_ERROR, "Failed to create epair interface %s: %s", name,
                strerror(errno));
      close(sock);
      return -1;
    }
    debug_log(DEBUG_INFO,
              "Created epair interface %s (which creates %sa and %sb)", name,
              name, name);
  } else {
    /* Create interface for other types */
    if (ioctl(sock, SIOCIFCREATE2, &ifr) < 0) {
      debug_log(DEBUG_ERROR, "Failed to create interface %s: %s", name,
                strerror(errno));
      close(sock);
      return -1;
    }
    debug_log(DEBUG_INFO, "Created interface %s of type %s", name, type_str);
  }

  close(sock);
  return 0;
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
 * Delete a network interface
 * @param name Interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_delete(const char *name) {
  int sock;
  struct ifreq ifr;

  if (!name) {
    return -1;
  }

  /* Create socket for ioctl */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for interface deletion: %s",
              strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Delete interface */
  if (ioctl(sock, SIOCIFDESTROY, &ifr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to delete interface %s: %s", name,
              strerror(errno));
    close(sock);
    return -1;
  }

  debug_log(DEBUG_INFO, "Deleted interface %s", name);
  close(sock);
  return 0;
}

/**
 * Set interface FIB assignment
 * @param name Interface name
 * @param fib FIB number
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_set_fib(const char *name, uint32_t fib) {
  int sock;
  struct ifreq ifr;

  if (!name) {
    return -1;
  }

  /* Create socket for ioctl */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for FIB assignment: %s",
              strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
  ifr.ifr_fib = fib;

  /* Set FIB */
  if (ioctl(sock, SIOCSIFFIB, &ifr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to set FIB %u for interface %s: %s", fib,
              name, strerror(errno));
    close(sock);
    return -1;
  }

  debug_log(DEBUG_INFO, "Set FIB %u for interface %s", fib, name);
  close(sock);
  return 0;
}

/**
 * Set interface address
 * @param name Interface name
 * @param address Address string
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_set_address(const char *name, const char *address,
                                  int family) {
  int sock;
  struct ifreq ifr;
  struct sockaddr_storage addr;

  if (!name || !address) {
    return -1;
  }

  /* Parse address */
  if (parse_address(address, &addr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to parse address %s", address);
    return -1;
  }

  /* Create socket for ioctl */
  sock = socket(family, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for address assignment: %s",
              strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
  memcpy(&ifr.ifr_addr, &addr, sizeof(ifr.ifr_addr));

  /* Set address */
  if (ioctl(sock, SIOCSIFADDR, &ifr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to set address %s for interface %s: %s",
              address, name, strerror(errno));
    close(sock);
    return -1;
  }

  debug_log(DEBUG_INFO, "Set address %s for interface %s", address, name);
  close(sock);
  return 0;
}

/**
 * Delete interface address
 * @param name Interface name
 * @param family Address family
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_delete_address(const char *name, int family) {
  int sock;
  struct ifreq ifr;
  struct sockaddr_storage addr;

  if (!name) {
    return -1;
  }

  /* Create socket for ioctl */
  sock = socket(family, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for address deletion: %s",
              strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
  memset(&addr, 0, sizeof(addr));
  addr.ss_family = family;
  memcpy(&ifr.ifr_addr, &addr, sizeof(ifr.ifr_addr));

  /* Delete address */
  if (ioctl(sock, SIOCDIFADDR, &ifr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to delete address from interface %s: %s",
              name, strerror(errno));
    close(sock);
    return -1;
  }

  debug_log(DEBUG_INFO, "Deleted address from interface %s", name);
  close(sock);
  return 0;
}

/**
 * Set interface MTU
 * @param name Interface name
 * @param mtu MTU value
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_set_mtu(const char *name, int mtu) {
  int sock;
  struct ifreq ifr;

  if (!name || mtu <= 0) {
    return -1;
  }

  /* Create socket for ioctl */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for MTU setting: %s",
              strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
  ifr.ifr_mtu = mtu;

  /* Set MTU */
  if (ioctl(sock, SIOCSIFMTU, &ifr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to set MTU %d for interface %s: %s", mtu,
              name, strerror(errno));
    close(sock);
    return -1;
  }

  debug_log(DEBUG_INFO, "Set MTU %d for interface %s", mtu, name);
  close(sock);
  return 0;
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
    debug_log(DEBUG_ERROR, "Failed to create socket for FIB query: %s",
              strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Get FIB */
  if (ioctl(sock, SIOCGIFFIB, &ifr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to get FIB for interface %s: %s", name,
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
int freebsd_interface_get_mtu(const char *name, int *mtu) {
  int sock;
  struct ifreq ifr;

  if (!name || !mtu) {
    return -1;
  }

  /* Create socket for ioctl */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for MTU query: %s",
              strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Get MTU */
  if (ioctl(sock, SIOCGIFMTU, &ifr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to get MTU for interface %s: %s", name,
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
 * @param groups Array to store group names
 * @param max_groups Maximum number of groups to store
 * @param group_count Pointer to store actual number of groups
 * @return 0 on success, -1 on failure
 */
int freebsd_interface_get_groups(const char *name,
                                 char (*groups)[MAX_GROUP_NAME_LEN],
                                 int max_groups, int *group_count) {
  int sock;
  struct ifgroupreq ifgr;
  struct ifg_req *ifg;
  size_t len;

  if (!name || !groups || !group_count) {
    return -1;
  }

  *group_count = 0;

  /* Create socket for ioctl */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for group query: %s",
              strerror(errno));
    return -1;
  }

  /* Set up interface group request */
  memset(&ifgr, 0, sizeof(ifgr));
  strlcpy(ifgr.ifgr_name, name, IFNAMSIZ);

  /* First call to get the required buffer size */
  if (ioctl(sock, SIOCGIFGROUP, (caddr_t)&ifgr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to get group info for %s: %s", name,
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
             ifg && len >= sizeof(*ifg) && *group_count < max_groups; ifg++) {
          len -= sizeof(*ifg);
          strlcpy(groups[*group_count], ifg->ifgrq_group, MAX_GROUP_NAME_LEN);
          (*group_count)++;
        }
      } else {
        debug_log(DEBUG_ERROR, "Failed to get groups for %s: %s", name,
                  strerror(errno));
      }
      free(ifgr.ifgr_groups);
    }
  }

  close(sock);
  return 0;
}

 