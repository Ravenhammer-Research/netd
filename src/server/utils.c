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
#include <sys/types.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <net/if_dl.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

/**
 * Convert interface type enum to string
 * @param type Interface type
 * @return String representation
 */
const char *interface_type_to_string(interface_type_t type) {
  switch (type) {
  case IF_TYPE_ETHERNET:
    return "ethernetCsmacd";
  case IF_TYPE_WIRELESS:
    return "ieee80211";
  case IF_TYPE_EPAIR:
    return "epair";
  case IF_TYPE_GIF:
    return "gif";
  case IF_TYPE_GRE:
    return "gre";
  case IF_TYPE_LAGG:
    return "lagg";
  case IF_TYPE_LOOPBACK:
    return "softwareLoopback";
  case IF_TYPE_OVPN:
    return "ovpn";
  case IF_TYPE_TUN:
    return "tun";
  case IF_TYPE_TAP:
    return "tap";
  case IF_TYPE_VLAN:
    return "l2vlan";
  case IF_TYPE_VXLAN:
    return "vxlan";
  case IF_TYPE_BRIDGE:
    return "bridge";
  default:
    return "other";
  }
}

/**
 * Get the namespace for an interface type
 * @param type Interface type
 * @return Namespace string (ianaift or netd)
 */
const char *interface_type_get_namespace(interface_type_t type) {
  switch (type) {
  case IF_TYPE_ETHERNET:
  case IF_TYPE_WIRELESS:
  case IF_TYPE_LOOPBACK:
  case IF_TYPE_VLAN:
  case IF_TYPE_BRIDGE:
    return "ianaift";
  case IF_TYPE_EPAIR:
  case IF_TYPE_GIF:
  case IF_TYPE_GRE:
  case IF_TYPE_LAGG:
  case IF_TYPE_OVPN:
  case IF_TYPE_TUN:
  case IF_TYPE_TAP:
  case IF_TYPE_VXLAN:
    return "netd";
  default:
    return "ianaift";
  }
}

/**
 * Convert string to interface type enum
 * @param str Interface type string
 * @return Interface type enum
 */
interface_type_t interface_type_from_string(const char *str) {
  if (!str) {
    return IF_TYPE_UNKNOWN;
  }

  if (strcmp(str, "ethernet") == 0 || strcmp(str, "ethernetCsmacd") == 0) {
    return IF_TYPE_ETHERNET;
  } else if (strcmp(str, "wireless") == 0 ||
             strcmp(str, "wireless80211") == 0 ||
             strcmp(str, "ieee80211") == 0) {
    return IF_TYPE_WIRELESS;
  } else if (strcmp(str, "epair") == 0) {
    return IF_TYPE_EPAIR;
  } else if (strcmp(str, "gif") == 0) {
    return IF_TYPE_GIF;
  } else if (strcmp(str, "gre") == 0) {
    return IF_TYPE_GRE;
  } else if (strcmp(str, "lagg") == 0) {
    return IF_TYPE_LAGG;
  } else if (strcmp(str, "lo") == 0 || strcmp(str, "loopback") == 0 ||
             strcmp(str, "softwareLoopback") == 0) {
    return IF_TYPE_LOOPBACK;
  } else if (strcmp(str, "ovpn") == 0 || strcmp(str, "tunnel") == 0) {
    return IF_TYPE_OVPN;
  } else if (strcmp(str, "tun") == 0) {
    return IF_TYPE_TUN;
  } else if (strcmp(str, "tap") == 0) {
    return IF_TYPE_TAP;
  } else if (strcmp(str, "vlan") == 0 || strcmp(str, "l2vlan") == 0) {
    return IF_TYPE_VLAN;
  } else if (strcmp(str, "vxlan") == 0) {
    return IF_TYPE_VXLAN;
  } else if (strcmp(str, "bridge") == 0) {
    return IF_TYPE_BRIDGE;
  }

  return IF_TYPE_UNKNOWN;
}

/**
 * Validate interface name
 * @param name Interface name
 * @return true if valid, false otherwise
 */
bool is_valid_interface_name(const char *name) {
  if (!name || strlen(name) == 0 || strlen(name) >= MAX_IFNAME_LEN) {
    debug_log(DEBUG,
              "Interface name validation failed: %s (length: %zu, max: %d)",
              name ? name : "NULL", name ? strlen(name) : 0, MAX_IFNAME_LEN);
    return false;
  }

  /* Check for valid characters */
  for (const char *p = name; *p; p++) {
    if (!isalnum(*p) && *p != '_' && *p != '-' && *p != '.') {
      debug_log(
          DEBUG,
          "Interface name validation failed: invalid character '%c' in '%s'",
          *p, name);
      return false;
    }
  }

  debug_log(DEBUG, "Interface name validation passed: '%s'", name);
  return true;
}

/**
 * Validate VRF name
 * @param name VRF name
 * @return true if valid, false otherwise
 */
bool is_valid_vrf_name(const char *name) {
  if (!name || strlen(name) == 0 || strlen(name) >= MAX_VRF_NAME_LEN) {
    debug_log(DEBUG,
              "VRF name validation failed: %s (length: %zu, max: %d)",
              name ? name : "NULL", name ? strlen(name) : 0, MAX_VRF_NAME_LEN);
    return false;
  }

  /* Check for valid characters */
  for (const char *p = name; *p; p++) {
    if (!isalnum(*p) && *p != '_' && *p != '-') {
      debug_log(DEBUG,
                "VRF name validation failed: invalid character '%c' in '%s'",
                *p, name);
      return false;
    }
  }

  debug_log(DEBUG, "VRF name validation passed: '%s'", name);
  return true;
}

/**
 * Validate FIB number
 * @param fib FIB number
 * @return true if valid, false otherwise
 */
/**
 * Get the number of FIBs configured in the system
 * @return Number of FIBs, or 1 if unable to determine
 */
uint32_t get_system_fib_count(void) {
  /* Try to determine the number of FIBs by attempting to enumerate routes
   * for different FIB numbers. Start with 1 and increment until we fail. */
  uint32_t fib_count = 1;
  int mib[6];
  size_t needed;
  
  /* Use the same sysctl approach as the working route enumeration code */
  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;           /* protocol */
  mib[3] = 0;           /* address family (0 = all) */
  mib[4] = NET_RT_DUMP; /* get all routes */
  
  /* Try FIB numbers starting from 0 */
  for (uint32_t test_fib = 0; test_fib < 256; test_fib++) { /* Reasonable upper limit */
    mib[5] = test_fib;  /* FIB number */
    
    /* Try to get the size needed for this FIB */
    if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0) {
      if (errno == EINVAL || errno == ENOTSUP) {
        /* This FIB doesn't exist, we've found the limit */
        fib_count = test_fib;
        break;
      } else if (errno == ENOENT) {
        /* This FIB exists but has no routes, continue */
        fib_count = test_fib + 1; /* Count it even if empty */
      } else {
        /* Some other error, assume this is the limit */
        fib_count = test_fib;
        break;
      }
    } else {
      /* This FIB exists and has routes */
      fib_count = test_fib + 1;
    }
  }
  
  debug_log(DEBUG, "System has %u FIBs configured", fib_count);
  return fib_count;
}

bool is_valid_fib_number(uint32_t fib) {
  uint32_t max_fibs = get_system_fib_count();
  bool valid = (fib < max_fibs);
  debug_log(DEBUG, "FIB number validation: %u is %s (max: %u)", fib,
            valid ? "valid" : "invalid", max_fibs);
  return valid;
}

/**
 * Parse address string to sockaddr_storage
 * @param addr_str Address string (IPv4, IPv6, or hostname)
 * @param addr Output sockaddr_storage structure
 * @return 0 on success, -1 on failure
 */
int parse_address(const char *addr_str, struct sockaddr_storage *addr) {
  struct addrinfo hints, *res;
  char *host_part, *port_part;
  char addr_copy[256];
  int ret;

  if (!addr_str || !addr) {
    debug_log(ERROR,
              "Invalid parameters for address parsing: addr_str=%p, addr=%p",
              addr_str, addr);
    return -1;
  }

  debug_log(DEBUG, "Parsing address: '%s'", addr_str);

  strlcpy(addr_copy, addr_str, sizeof(addr_copy));

  /* Try to parse as IPv4 or IPv6 first (these functions handle the formats correctly) */
  if (inet_pton(AF_INET, addr_copy, &((struct sockaddr_in *)addr)->sin_addr) == 1) {
    addr->ss_family = AF_INET;
    addr->ss_len = sizeof(struct sockaddr_in);
    ((struct sockaddr_in *)addr)->sin_port = 0;
    debug_log(DEBUG, "Successfully parsed as IPv4 address: %s", addr_str);
    return 0;
  }
  
  if (inet_pton(AF_INET6, addr_copy, &((struct sockaddr_in6 *)addr)->sin6_addr) == 1) {
    addr->ss_family = AF_INET6;
    addr->ss_len = sizeof(struct sockaddr_in6);
    ((struct sockaddr_in6 *)addr)->sin6_port = 0;
    debug_log(DEBUG, "Successfully parsed as IPv6 address: %s", addr_str);
    return 0;
  }

  /* Check if this is a link#<n> address */
  if (strncmp(addr_copy, "link#", 5) == 0) {
    int link_index = atoi(addr_copy + 6);
    if (link_index > 0) {
      struct sockaddr_dl *sdl = (struct sockaddr_dl *)addr;
      memset(sdl, 0, sizeof(*sdl));
      sdl->sdl_len = sizeof(*sdl);
      sdl->sdl_family = AF_LINK;
      sdl->sdl_index = link_index;
      addr->ss_family = AF_LINK;
      addr->ss_len = sizeof(*sdl);
      debug_log(DEBUG, "Successfully parsed as link address: %s (index %d)", addr_str, link_index);
      return 0;
    }
  }

  /* Now try to handle hostname:port format */
  port_part = strchr(addr_copy, ':');
  if (port_part) {
    *port_part = '\0';
    port_part++;
    debug_log(DEBUG, "Address has port: host='%s', port='%s'", addr_copy, port_part);
  } else {
    debug_log(DEBUG, "Address has no port: host='%s'", addr_copy);
  }

  host_part = addr_copy;





  /* Try to resolve as hostname */
  debug_log(DEBUG, "Attempting to resolve as hostname: '%s'", host_part);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  ret = getaddrinfo(host_part, port_part, &hints, &res);
  if (ret == 0) {
    memcpy(addr, res->ai_addr, res->ai_addrlen);
    /* Ensure ss_len is set correctly */
    if (addr->ss_family == AF_INET) {
      addr->ss_len = sizeof(struct sockaddr_in);
    } else if (addr->ss_family == AF_INET6) {
      addr->ss_len = sizeof(struct sockaddr_in6);
    }
    freeaddrinfo(res);
    debug_log(DEBUG, "Successfully resolved hostname: %s", addr_str);
    return 0;
  } else {
    debug_log(ERROR, "Failed to resolve hostname '%s': %s", host_part,
              gai_strerror(ret));
  }

  debug_log(ERROR, "Failed to parse address: %s", addr_str);
  return -1;
}

/**
 * Format sockaddr_storage to string
 * @param addr sockaddr_storage structure
 * @param str Output string buffer
 * @param len Buffer length
 * @return 0 on success, -1 on failure
 */
int format_address(const struct sockaddr_storage *addr, char *str, size_t len) {
  if (!addr || !str || len == 0) {
    debug_log(
        ERROR,
        "Invalid parameters for address formatting: addr=%p, str=%p, len=%zu",
        addr, str, len);
    return -1;
  }

  debug_log(DEBUG2, "Formatting address (family: %d)", addr->ss_family);

  switch (addr->ss_family) {
  case AF_INET: {
    struct sockaddr_in *sin = (struct sockaddr_in *)addr;
    if (inet_ntop(AF_INET, &sin->sin_addr, str, len) == NULL) {
      debug_log(ERROR, "Failed to format IPv4 address");
      return -1;
    }
    debug_log(DEBUG2, "Formatted IPv4 address: %s", str);
    break;
  }
  case AF_INET6: {
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)addr;
    if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, len) == NULL) {
      debug_log(ERROR, "Failed to format IPv6 address");
      return -1;
    }
    debug_log(DEBUG2, "Formatted IPv6 address: %s", str);
    break;
  }
  case AF_LINK: {
    struct sockaddr_dl *sdl = (struct sockaddr_dl *)addr;
    snprintf(str, len, "link#%d", sdl->sdl_index);
    debug_log(DEBUG2, "Formatted link address: %s", str);
    break;
  }
  default:
    debug_log(ERROR, "Unsupported address family: %d", addr->ss_family);
    return -1;
  }

  return 0;
}

/**
 * Get address family from sockaddr_storage
 * @param addr sockaddr_storage structure
 * @return Address family
 */
int get_address_family(const struct sockaddr_storage *addr) {
  if (!addr) {
    debug_log(DEBUG, "NULL address provided to get_address_family");
    return AF_UNSPEC;
  }

  debug_log(DEBUG, "Getting address family: %d", addr->ss_family);
  return addr->ss_family;
}

/**
 * Get prefix length from netmask in sockaddr_storage
 * @param addr sockaddr_storage structure containing netmask
 * @return Prefix length
 */
int get_prefix_length(const struct sockaddr_storage *addr) {
  int prefix_len = 0;
  unsigned char *bytes;
  int byte_count;

  if (!addr) {
    debug_log(ERROR, "NULL address provided to get_prefix_length");
    return -1;
  }

  debug_log(DEBUG, "Calculating prefix length for address family: %d",
            addr->ss_family);

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
    debug_log(ERROR,
              "Unsupported address family for prefix calculation: %d",
              addr->ss_family);
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

  debug_log(DEBUG, "Calculated prefix length: %d", prefix_len);
  return prefix_len;
}
