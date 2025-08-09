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
#include <net80211/ieee80211_freebsd.h>
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
 * Get WiFi information for an interface
 * @param ifname Interface name
 * @param regdomain Regulatory domain (output)
 * @param regdomain_size Size of regdomain buffer
 * @param country Country code (output)
 * @param country_size Size of country buffer
 * @param authmode Authentication mode (output)
 * @param authmode_size Size of authmode buffer
 * @param privacy Privacy setting (output)
 * @param privacy_size Size of privacy buffer
 * @param txpower Transmit power (output)
 * @param bmiss Beacon miss threshold (output)
 * @param scanvalid Scan validity period (output)
 * @param features WiFi features (output)
 * @param features_size Size of features buffer
 * @param bintval Beacon interval (output)
 * @param parent Parent interface name (output)
 * @param parent_size Size of parent buffer
 * @return 0 on success, -1 on failure
 */
int freebsd_get_wifi_info(const char *ifname, char *regdomain,
                          size_t regdomain_size, char *country,
                          size_t country_size, char *authmode,
                          size_t authmode_size, char *privacy,
                          size_t privacy_size, int *txpower,
                          int *bmiss, int *scanvalid, char *features,
                          size_t features_size, int *bintval,
                          char *parent, size_t parent_size) {
  (void)regdomain_size;
  (void)country_size;
  (void)authmode_size;
  (void)privacy_size;
  (void)features_size;
  (void)parent_size;
  int sock;
  struct ifreq ifr;
  int found = 0;

  debug_log(DEBUG_DEBUG, "freebsd_get_wifi_info called for interface: %s", ifname);

  if (!ifname || !regdomain || !country || !authmode || !privacy ||
      !txpower || !bmiss || !scanvalid || !features || !bintval || !parent) {
    return -1;
  }

  regdomain[0] = '\0';
  country[0] = '\0';
  authmode[0] = '\0';
  privacy[0] = '\0';
  features[0] = '\0';
  parent[0] = '\0';
  *txpower = 0;
  *bmiss = 0;
  *scanvalid = 0;
  *bintval = 0;

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for wifi info: %s", strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

  /* Try to get wireless information using IEEE 802.11 ioctls */
  struct ieee80211req ireq;
  
  /* Set up IEEE 802.11 request structure */
  memset(&ireq, 0, sizeof(ireq));
  strlcpy(ireq.i_name, ifname, sizeof(ireq.i_name));

  /* Get transmit power */
  ireq.i_type = IEEE80211_IOC_TXPOWER;
  if (ioctl(sock, SIOCG80211, &ireq) == 0) {
    *txpower = ireq.i_val;
    debug_log(DEBUG_DEBUG, "Got txpower: %d", *txpower);
    found = 1;
  }

  /* Get beacon miss threshold */
  memset(&ireq, 0, sizeof(ireq));
  strlcpy(ireq.i_name, ifname, sizeof(ireq.i_name));
  ireq.i_type = IEEE80211_IOC_BMISSTHRESHOLD;
  if (ioctl(sock, SIOCG80211, &ireq) == 0) {
    *bmiss = ireq.i_val;
    debug_log(DEBUG_DEBUG, "Got bmiss: %d", *bmiss);
  }

  /* Get scan valid time */
  memset(&ireq, 0, sizeof(ireq));
  strlcpy(ireq.i_name, ifname, sizeof(ireq.i_name));
  ireq.i_type = IEEE80211_IOC_SCANVALID;
  if (ioctl(sock, SIOCG80211, &ireq) == 0) {
    *scanvalid = ireq.i_val;
    debug_log(DEBUG_DEBUG, "Got scanvalid: %d", *scanvalid);
  }

  /* Get beacon interval */
  memset(&ireq, 0, sizeof(ireq));
  strlcpy(ireq.i_name, ifname, sizeof(ireq.i_name));
  ireq.i_type = IEEE80211_IOC_BEACON_INTERVAL;
  if (ioctl(sock, SIOCG80211, &ireq) == 0) {
    *bintval = ireq.i_val;
    debug_log(DEBUG_DEBUG, "Got bintval: %d", *bintval);
  }

  /* Get authentication mode */
  memset(&ireq, 0, sizeof(ireq));
  strlcpy(ireq.i_name, ifname, sizeof(ireq.i_name));
  ireq.i_type = IEEE80211_IOC_AUTHMODE;
  if (ioctl(sock, SIOCG80211, &ireq) == 0) {
    switch (ireq.i_val) {
      case 0: /* IEEE80211_AUTH_OPEN */
        strlcpy(authmode, "OPEN", authmode_size);
        break;
      case 1: /* IEEE80211_AUTH_SHARED */
        strlcpy(authmode, "SHARED", authmode_size);
        break;
      case 2: /* IEEE80211_AUTH_8021X */
        strlcpy(authmode, "8021X", authmode_size);
        break;
      case 3: /* IEEE80211_AUTH_WPA */
        strlcpy(authmode, "WPA", authmode_size);
        break;
      default:
        strlcpy(authmode, "UNKNOWN", authmode_size);
        break;
    }
    debug_log(DEBUG_DEBUG, "Got authmode: %s", authmode);
  }

  /* Get privacy setting */
  memset(&ireq, 0, sizeof(ireq));
  strlcpy(ireq.i_name, ifname, sizeof(ireq.i_name));
  ireq.i_type = IEEE80211_IOC_PRIVACY;
  if (ioctl(sock, SIOCG80211, &ireq) == 0) {
    strlcpy(privacy, ireq.i_val ? "ON" : "OFF", privacy_size);
    debug_log(DEBUG_DEBUG, "Got privacy: %s", privacy);
  }

  /* Get features - check for WME support */
  memset(&ireq, 0, sizeof(ireq));
  strlcpy(ireq.i_name, ifname, sizeof(ireq.i_name));
  ireq.i_type = IEEE80211_IOC_WME;
  if (ioctl(sock, SIOCG80211, &ireq) == 0) {
    snprintf(features, features_size, "%d", ireq.i_val);
    debug_log(DEBUG_DEBUG, "Got features: %s", features);
  }

  /* Get regulatory domain and country */
  struct ieee80211_regdomain regdomain_data;
  memset(&ireq, 0, sizeof(ireq));
  strlcpy(ireq.i_name, ifname, sizeof(ireq.i_name));
  ireq.i_type = IEEE80211_IOC_REGDOMAIN;
  ireq.i_data = &regdomain_data;
  ireq.i_len = sizeof(regdomain_data);
  if (ioctl(sock, SIOCG80211, &ireq) == 0) {
    /* Extract country code from regulatory domain */
    if (regdomain_data.country != 0) {
      switch (regdomain_data.country) {
        case 840: /* US */
          strlcpy(country, "US", country_size);
          break;
        case 124: /* CA */
          strlcpy(country, "CA", country_size);
          break;
        case 826: /* GB */
          strlcpy(country, "GB", country_size);
          break;
        case 276: /* DE */
          strlcpy(country, "DE", country_size);
          break;
        case 250: /* FR */
          strlcpy(country, "FR", country_size);
          break;
        case 392: /* JP */
          strlcpy(country, "JAPAN", country_size);
          break;
        case 410: /* KR */
          strlcpy(country, "KOREA", country_size);
          break;
        case 36: /* AU */
          strlcpy(country, "AUSTRALIA", country_size);
          break;
        default:
          snprintf(country, country_size, "%d", regdomain_data.country);
          break;
      }
      debug_log(DEBUG_DEBUG, "Got country: %s", country);
    }

    /* Set regulatory domain name based on SKU */
    switch (regdomain_data.regdomain) {
      case 0: /* SKU0 - FCC */
        strlcpy(regdomain, "FCC", regdomain_size);
        break;
      case 1: /* SKU1 - ETSI */
        strlcpy(regdomain, "ETSI", regdomain_size);
        break;
      case 2: /* SKU2 - Japan */
        strlcpy(regdomain, "JAPAN", regdomain_size);
        break;
      case 3: /* SKU3 - Korea */
        strlcpy(regdomain, "KOREA", regdomain_size);
        break;
      case 4: /* SKU4 - Australia */
        strlcpy(regdomain, "AUSTRALIA", regdomain_size);
        break;
      case 5: /* SKU5 - Canada */
        strlcpy(regdomain, "CANADA", regdomain_size);
        break;
      case 6: /* SKU6 - World */
        strlcpy(regdomain, "WORLD", regdomain_size);
        break;
      default:
        snprintf(regdomain, regdomain_size, "SKU%d", regdomain_data.regdomain);
        break;
    }
    debug_log(DEBUG_DEBUG, "Got regdomain: %s", regdomain);
  } else {
    debug_log(DEBUG_DEBUG, "Failed to get regulatory domain for %s: %s", ifname, strerror(errno));
  }

  /* Get parent interface - use the same method as ifconfig */
  char parent_data[IFNAMSIZ];
  int parent_len;
  
  memset(&ireq, 0, sizeof(ireq));
  strlcpy(ireq.i_name, ifname, sizeof(ireq.i_name));
  ireq.i_type = IEEE80211_IOC_IC_NAME;
  ireq.i_val = -1;
  ireq.i_data = parent_data;
  ireq.i_len = sizeof(parent_data);
  
  if (ioctl(sock, SIOCG80211, &ireq) == 0) {
    parent_len = ireq.i_len;
    if (parent_len > 0 && parent_data[0] != '\0') {
      strlcpy(parent, parent_data, parent_size);
      debug_log(DEBUG_DEBUG, "Found parent interface from ioctl: %s", parent);
    }
  }

  close(sock);
  return found ? 0 : -1;
} 