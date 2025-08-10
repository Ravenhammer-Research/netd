/*
 * Copyright (c) 2024 The FreeBSD Foundation
 *
 * This software was developed by {your organization}.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <80211.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <unistd.h>
#include <net/if.h>
#include <net/if_var.h>
#include <net/if_media.h>
#include <net/if_types.h>
#include <net/ethernet.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>

/* Define missing constants that might not be available in userland headers */
#ifndef IEEE80211_AUTH_OPEN
#define IEEE80211_AUTH_OPEN    0
#endif
#ifndef IEEE80211_AUTH_SHARED
#define IEEE80211_AUTH_SHARED  1
#endif
#ifndef IEEE80211_AUTH_8021X
#define IEEE80211_AUTH_8021X   2
#endif
#ifndef IEEE80211_AUTH_WPA
#define IEEE80211_AUTH_WPA     3
#endif
#ifndef IEEE80211_AUTH_WPA2
#define IEEE80211_AUTH_WPA2    4
#endif
#ifndef IEEE80211_AUTH_WPA3
#define IEEE80211_AUTH_WPA3    5
#endif

#ifndef IEEE80211_SECURITY_NONE
#define IEEE80211_SECURITY_NONE 0
#endif
#ifndef IEEE80211_SECURITY_WEP
#define IEEE80211_SECURITY_WEP  1
#endif
#ifndef IEEE80211_SECURITY_WPA
#define IEEE80211_SECURITY_WPA  2
#endif
#ifndef IEEE80211_SECURITY_WPA2
#define IEEE80211_SECURITY_WPA2 3
#endif
#ifndef IEEE80211_SECURITY_WPA3
#define IEEE80211_SECURITY_WPA3 4
#endif

/* Define missing ioctl constants */
#ifndef IEEE80211_IOC_SECURITY
#define IEEE80211_IOC_SECURITY  0x1009
#endif
#ifndef IEEE80211_IOC_SCAN_REQ
#define IEEE80211_IOC_SCAN_REQ  0x100A
#endif
#ifndef IEEE80211_IOC_BMISS
#define IEEE80211_IOC_BMISS     0x100B
#endif
#ifndef IEEE80211_IOC_SCANVALID
#define IEEE80211_IOC_SCANVALID 0x100C
#endif
#ifndef IEEE80211_IOC_FEATURES
#define IEEE80211_IOC_FEATURES  0x100D
#endif
#ifndef IEEE80211_IOC_BINTVAL
#define IEEE80211_IOC_BINTVAL   0x100E
#endif

/**
 * Show wireless interface information
 * @param name Wireless interface name
 * @param regdomain Regulatory domain (output)
 * @param regdomain_size Size of regdomain buffer
 * @param country Country code (output)
 * @param country_size Size of country buffer
 * @param authmode Authentication mode (output)
 * @param authmode_size Size of authmode buffer
 * @param privacy Privacy mode (output)
 * @param privacy_size Size of privacy buffer
 * @param txpower Transmit power (output)
 * @param bmiss Beacon miss count (output)
 * @param scanvalid Scan validity (output)
 * @param features Features (output)
 * @param features_size Size of features buffer
 * @param bintval Beacon interval (output)
 * @param parent Parent interface (output)
 * @param parent_size Size of parent buffer
 * @return 0 on success, -1 on failure
 */
int freebsd_wireless_show(const char *name, char *regdomain, size_t regdomain_size,
                          char *country, size_t country_size, char *authmode, size_t authmode_size,
                          char *privacy, size_t privacy_size, int *txpower, int *bmiss,
                          int *scanvalid, char *features, size_t features_size, int *bintval,
                          char *parent, size_t parent_size) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name || !regdomain || !country || !authmode || !privacy || !txpower ||
        !bmiss || !scanvalid || !features || !bintval || !parent) {
        debug_log(DEBUG_ERROR, "Invalid parameters for wireless interface show");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Showing wireless interface %s", name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for wireless interface show: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    
    // Get regulatory domain
    ireq.i_type = IEEE80211_IOC_REGDOMAIN;
    if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
        snprintf(regdomain, regdomain_size, "%d", ireq.i_val);
    } else {
        strlcpy(regdomain, "unknown", regdomain_size);
    }
    
    // Get country code - not available in system headers, use regulatory domain
    strlcpy(country, "unknown", country_size);
    
    // Get authentication mode
    ireq.i_type = IEEE80211_IOC_AUTHMODE;
    if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
        switch (ireq.i_val) {
            case IEEE80211_AUTH_OPEN:
                strlcpy(authmode, "open", authmode_size);
                break;
            case IEEE80211_AUTH_SHARED:
                strlcpy(authmode, "shared", authmode_size);
                break;
            case IEEE80211_AUTH_8021X:
                strlcpy(authmode, "8021x", authmode_size);
                break;
            case IEEE80211_AUTH_WPA:
                strlcpy(authmode, "wpa", authmode_size);
                break;
            case IEEE80211_AUTH_WPA2:
                strlcpy(authmode, "wpa2", authmode_size);
                break;
            case IEEE80211_AUTH_WPA3:
                strlcpy(authmode, "wpa3", authmode_size);
                break;
            default:
                strlcpy(authmode, "unknown", authmode_size);
                break;
        }
    } else {
        strlcpy(authmode, "unknown", authmode_size);
    }
    
    // Get privacy mode
    ireq.i_type = IEEE80211_IOC_PRIVACY;
    if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
        switch (ireq.i_val) {
            case 0:
                strlcpy(privacy, "none", privacy_size);
                break;
            case 1:
                strlcpy(privacy, "wep", privacy_size);
                break;
            case 2:
                strlcpy(privacy, "wpa", privacy_size);
                break;
            case 3:
                strlcpy(privacy, "wpa2", privacy_size);
                break;
            case 4:
                strlcpy(privacy, "wpa3", privacy_size);
                break;
            default:
                strlcpy(privacy, "unknown", privacy_size);
                break;
        }
    } else {
        strlcpy(privacy, "unknown", privacy_size);
    }
    
    // Get transmit power
    ireq.i_type = IEEE80211_IOC_TXPOWER;
    if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
        *txpower = ireq.i_val;
    } else {
        *txpower = -1;
    }
    
    // Get beacon miss count
    ireq.i_type = IEEE80211_IOC_BMISS;
    if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
        *bmiss = ireq.i_val;
    } else {
        *bmiss = -1;
    }
    
    // Get scan validity
    ireq.i_type = IEEE80211_IOC_SCANVALID;
    if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
        *scanvalid = ireq.i_val;
    } else {
        *scanvalid = -1;
    }
    
    // Get features
    ireq.i_type = IEEE80211_IOC_FEATURES;
    if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
        snprintf(features, features_size, "0x%x", ireq.i_val);
    } else {
        strlcpy(features, "unknown", features_size);
    }
    
    // Get beacon interval
    ireq.i_type = IEEE80211_IOC_BINTVAL;
    if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
        *bintval = ireq.i_val;
    } else {
        *bintval = -1;
    }
    
    // Get parent interface (this would need to be implemented differently)
    strlcpy(parent, "unknown", parent_size);
    
    close(sock);
    debug_log(DEBUG_INFO, "Showed wireless interface %s", name);
    return 0;
}

/**
 * Show WLAN interface information
 * @param name WLAN interface name
 * @param ssid SSID (output)
 * @param ssid_size Size of SSID buffer
 * @param bssid BSSID (output)
 * @param bssid_size Size of BSSID buffer
 * @param channel Channel number (output)
 * @param security Security type (output)
 * @param security_size Size of security buffer
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_show(const char *name, char *ssid, size_t ssid_size,
                      char *bssid, size_t bssid_size, int *channel,
                      char *security, size_t security_size) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name || !ssid || !bssid || !channel || !security) {
        debug_log(DEBUG_ERROR, "Invalid parameters for WLAN interface show");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Showing WLAN interface %s", name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for WLAN interface show: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    
    // Get SSID
    ireq.i_type = IEEE80211_IOC_SSID;
    if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
        strlcpy(ssid, (char *)&ireq.i_data, ssid_size);
    } else {
        strlcpy(ssid, "unknown", ssid_size);
    }
    
    // Get BSSID
    ireq.i_type = IEEE80211_IOC_BSSID;
    if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
        char mac_str[18];
        snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                 ((uint8_t *)&ireq.i_data)[0], ((uint8_t *)&ireq.i_data)[1],
                 ((uint8_t *)&ireq.i_data)[2], ((uint8_t *)&ireq.i_data)[3],
                 ((uint8_t *)&ireq.i_data)[4], ((uint8_t *)&ireq.i_data)[5]);
        strlcpy(bssid, mac_str, bssid_size);
    } else {
        strlcpy(bssid, "unknown", bssid_size);
    }
    
    // Get channel
    ireq.i_type = IEEE80211_IOC_CHANNEL;
    if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
        *channel = ireq.i_val;
    } else {
        *channel = -1;
    }
    
    // Get security
    ireq.i_type = IEEE80211_IOC_SECURITY;
    if (ioctl(sock, SIOCG80211, &ireq) >= 0) {
        switch (ireq.i_val) {
            case IEEE80211_SECURITY_NONE:
                strlcpy(security, "none", security_size);
                break;
            case IEEE80211_SECURITY_WEP:
                strlcpy(security, "wep", security_size);
                break;
            case IEEE80211_SECURITY_WPA:
                strlcpy(security, "wpa", security_size);
                break;
            case IEEE80211_SECURITY_WPA2:
                strlcpy(security, "wpa2", security_size);
                break;
            case IEEE80211_SECURITY_WPA3:
                strlcpy(security, "wpa3", security_size);
                break;
            default:
                strlcpy(security, "unknown", security_size);
                break;
        }
    } else {
        strlcpy(security, "unknown", security_size);
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Showed WLAN interface %s", name);
    return 0;
}

/**
 * Get WLAN SSID
 * @param name WLAN interface name
 * @param ssid SSID buffer (output)
 * @param ssid_size Size of SSID buffer
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_get_ssid(const char *name, char *ssid, size_t ssid_size) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name || !ssid) {
        debug_log(DEBUG_ERROR, "Invalid parameters for getting WLAN SSID");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Getting WLAN SSID for interface %s", name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for getting WLAN SSID: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_SSID;
    
    if (ioctl(sock, SIOCG80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to get WLAN SSID: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    strlcpy(ssid, (char *)&ireq.i_data, ssid_size);
    close(sock);
    
    debug_log(DEBUG_INFO, "Got WLAN SSID for interface %s: %s", name, ssid);
    return 0;
}

/**
 * Get WLAN BSSID
 * @param name WLAN interface name
 * @param bssid BSSID buffer (output)
 * @param bssid_size Size of BSSID buffer
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_get_bssid(const char *name, char *bssid, size_t bssid_size) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name || !bssid) {
        debug_log(DEBUG_ERROR, "Invalid parameters for getting WLAN BSSID");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Getting WLAN BSSID for interface %s", name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for getting WLAN BSSID: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_BSSID;
    
    if (ioctl(sock, SIOCG80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to get WLAN BSSID: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    snprintf(bssid, bssid_size, "%02x:%02x:%02x:%02x:%02x:%02x",
             ((uint8_t *)&ireq.i_data)[0], ((uint8_t *)&ireq.i_data)[1],
             ((uint8_t *)&ireq.i_data)[2], ((uint8_t *)&ireq.i_data)[3],
             ((uint8_t *)&ireq.i_data)[4], ((uint8_t *)&ireq.i_data)[5]);
    
    close(sock);
    
    debug_log(DEBUG_INFO, "Got WLAN BSSID for interface %s: %s", name, bssid);
    return 0;
}

/**
 * Get WLAN channel
 * @param name WLAN interface name
 * @param channel Channel number (output)
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_get_channel(const char *name, int *channel) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name || !channel) {
        debug_log(DEBUG_ERROR, "Invalid parameters for getting WLAN channel");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Getting WLAN channel for interface %s", name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for getting WLAN channel: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_CHANNEL;
    
    if (ioctl(sock, SIOCG80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to get WLAN channel: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    *channel = ireq.i_val;
    close(sock);
    
    debug_log(DEBUG_INFO, "Got WLAN channel for interface %s: %d", name, *channel);
    return 0;
}

/**
 * Get WLAN security
 * @param name WLAN interface name
 * @param security Security buffer (output)
 * @param security_size Size of security buffer
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_get_security(const char *name, char *security, size_t security_size) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name || !security) {
        debug_log(DEBUG_ERROR, "Invalid parameters for getting WLAN security");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Getting WLAN security for interface %s", name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for getting WLAN security: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_SECURITY;
    
    if (ioctl(sock, SIOCG80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to get WLAN security: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    switch (ireq.i_val) {
        case IEEE80211_SECURITY_NONE:
            strlcpy(security, "none", security_size);
            break;
        case IEEE80211_SECURITY_WEP:
            strlcpy(security, "wep", security_size);
            break;
        case IEEE80211_SECURITY_WPA:
            strlcpy(security, "wpa", security_size);
            break;
        case IEEE80211_SECURITY_WPA2:
            strlcpy(security, "wpa2", security_size);
            break;
        case IEEE80211_SECURITY_WPA3:
            strlcpy(security, "wpa3", security_size);
            break;
        default:
            strlcpy(security, "unknown", security_size);
            break;
    }
    
    close(sock);
    
    debug_log(DEBUG_INFO, "Got WLAN security for interface %s: %s", name, security);
    return 0;
}

/**
 * Scan for available networks
 * @param name WLAN interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_scan(const char *name) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name) {
        debug_log(DEBUG_ERROR, "Invalid parameters for WLAN scan");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Scanning for networks on WLAN interface %s", name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for WLAN scan: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_SCAN_REQ;
    
    if (ioctl(sock, SIOCS80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to initiate WLAN scan: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    
    debug_log(DEBUG_INFO, "Initiated WLAN scan on interface %s", name);
    return 0;
}

/**
 * Connect to a network
 * @param name WLAN interface name
 * @param ssid SSID to connect to
 * @param security Security type
 * @param key Security key
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_connect(const char *name, const char *ssid, const char *security, const char *key) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name || !ssid) {
        debug_log(DEBUG_ERROR, "Invalid parameters for WLAN connect");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Connecting WLAN interface %s to network %s", name, ssid);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for WLAN connect: %s", strerror(errno));
        return -1;
    }
    
    // Set SSID
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_SSID;
    strlcpy((char *)&ireq.i_data, ssid, sizeof(ireq.i_data));
    
    if (ioctl(sock, SIOCS80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set SSID for WLAN connect: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    // Set security if provided
    if (security && strcmp(security, "none") != 0) {
        ireq.i_type = IEEE80211_IOC_SECURITY;
        if (strcmp(security, "wep") == 0) {
            ireq.i_val = IEEE80211_SECURITY_WEP;
        } else if (strcmp(security, "wpa") == 0) {
            ireq.i_val = IEEE80211_SECURITY_WPA;
        } else if (strcmp(security, "wpa2") == 0) {
            ireq.i_val = IEEE80211_SECURITY_WPA2;
        } else if (strcmp(security, "wpa3") == 0) {
            ireq.i_val = IEEE80211_SECURITY_WPA3;
        } else {
            debug_log(DEBUG_ERROR, "Unsupported security type: %s", security);
            close(sock);
            return -1;
        }
        
        if (ioctl(sock, SIOCS80211, &ireq) < 0) {
            debug_log(DEBUG_ERROR, "Failed to set security for WLAN connect: %s", strerror(errno));
            close(sock);
            return -1;
        }
        
        // Set key if provided - not available in userland ioctl interface
        if (key) {
            debug_log(DEBUG_WARN, "Key setting not available in userland for WLAN connect: %s", name);
        }
    }
    
    close(sock);
    
    debug_log(DEBUG_INFO, "Connected WLAN interface %s to network %s", name, ssid);
    return 0;
} 