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

#include "80211.h"
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
#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_crypto.h>
#include <net80211/ieee80211_acl.h>
#include <net80211/ieee80211_amrr.h>
#include <net80211/ieee80211_ht.h>
#include <net80211/ieee80211_vht.h>
#include <net80211/ieee80211_he.h>
#include <net80211/ieee80211_radiotap.h>
#include <net80211/ieee80211_rssadapt.h>
#include <net80211/ieee80211_scan.h>
#include <net80211/ieee80211_sta.h>
#include <net80211/ieee80211_superg.h>
#include <net80211/ieee80211_tdma.h>
#include <net80211/ieee80211_wds.h>
#include <net80211/ieee80211_xauth.h>
#include <net80211/ieee80211_acl.h>
#include <net80211/ieee80211_amrr.h>
#include <net80211/ieee80211_ht.h>
#include <net80211/ieee80211_vht.h>
#include <net80211/ieee80211_he.h>
#include <net80211/ieee80211_radiotap.h>
#include <net80211/ieee80211_rssadapt.h>
#include <net80211/ieee80211_scan.h>
#include <net80211/ieee80211_sta.h>
#include <net80211/ieee80211_superg.h>
#include <net80211/ieee80211_tdma.h>
#include <net80211/ieee80211_wds.h>
#include <net80211/ieee80211_xauth.h>

/**
 * Create a wireless interface
 * @param name Wireless interface name
 * @param parent_name Parent interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_wireless_create(const char *name, const char *parent_name) {
    int sock;
    struct ifreq ifr;
    
    if (!name || !parent_name) {
        debug_log(DEBUG_ERROR, "Invalid parameters for wireless interface creation");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Creating wireless interface %s on parent %s", name, parent_name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for wireless interface creation: %s", strerror(errno));
        return -1;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    
    if (ioctl(sock, SIOCIFCREATE, &ifr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to create wireless interface: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Created wireless interface %s", name);
    return 0;
}

/**
 * Set wireless regulatory domain
 * @param name Wireless interface name
 * @param regdomain Regulatory domain string
 * @return 0 on success, -1 on failure
 */
int freebsd_wireless_set_regdomain(const char *name, const char *regdomain) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name || !regdomain) {
        debug_log(DEBUG_ERROR, "Invalid parameters for setting wireless regulatory domain");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Setting wireless regulatory domain to %s for interface %s", regdomain, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for setting wireless regulatory domain: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_REGDOMAIN;
    ireq.i_val = atoi(regdomain);
    
    if (ioctl(sock, SIOCS80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set wireless regulatory domain: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Set wireless regulatory domain to %s for interface %s", regdomain, name);
    return 0;
}

/**
 * Set wireless country code
 * @param name Wireless interface name
 * @param country Country code string
 * @return 0 on success, -1 on failure
 */
int freebsd_wireless_set_country(const char *name, const char *country) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name || !country) {
        debug_log(DEBUG_ERROR, "Invalid parameters for setting wireless country code");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Setting wireless country code to %s for interface %s", country, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for setting wireless country code: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_COUNTRY;
    strlcpy((char *)&ireq.i_data, country, sizeof(ireq.i_data));
    
    if (ioctl(sock, SIOCS80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set wireless country code: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Set wireless country code to %s for interface %s", country, name);
    return 0;
}

/**
 * Set wireless authentication mode
 * @param name Wireless interface name
 * @param authmode Authentication mode string
 * @return 0 on success, -1 on failure
 */
int freebsd_wireless_set_authmode(const char *name, const char *authmode) {
    int sock;
    struct ieee80211req ireq;
    int mode;
    
    if (!name || !authmode) {
        debug_log(DEBUG_ERROR, "Invalid parameters for setting wireless authentication mode");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Setting wireless authentication mode to %s for interface %s", authmode, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for setting wireless authentication mode: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_AUTHMODE;
    
    if (strcmp(authmode, "open") == 0) {
        mode = IEEE80211_AUTH_OPEN;
    } else if (strcmp(authmode, "shared") == 0) {
        mode = IEEE80211_AUTH_SHARED;
    } else if (strcmp(authmode, "8021x") == 0) {
        mode = IEEE80211_AUTH_8021X;
    } else if (strcmp(authmode, "wpa") == 0) {
        mode = IEEE80211_AUTH_WPA;
    } else if (strcmp(authmode, "wpa2") == 0) {
        mode = IEEE80211_AUTH_WPA2;
    } else if (strcmp(authmode, "wpa3") == 0) {
        mode = IEEE80211_AUTH_WPA3;
    } else {
        debug_log(DEBUG_ERROR, "Invalid authentication mode: %s", authmode);
        close(sock);
        return -1;
    }
    
    ireq.i_val = mode;
    
    if (ioctl(sock, SIOCS80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set wireless authentication mode: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Set wireless authentication mode to %s for interface %s", authmode, name);
    return 0;
}

/**
 * Set wireless privacy mode
 * @param name Wireless interface name
 * @param privacy Privacy mode string
 * @return 0 on success, -1 on failure
 */
int freebsd_wireless_set_privacy(const char *name, const char *privacy) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name || !privacy) {
        debug_log(DEBUG_ERROR, "Invalid parameters for setting wireless privacy mode");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Setting wireless privacy mode to %s for interface %s", privacy, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for setting wireless privacy mode: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_PRIVACY;
    
    if (strcmp(privacy, "none") == 0) {
        ireq.i_val = 0;
    } else if (strcmp(privacy, "wep") == 0) {
        ireq.i_val = 1;
    } else if (strcmp(privacy, "wpa") == 0) {
        ireq.i_val = 2;
    } else if (strcmp(privacy, "wpa2") == 0) {
        ireq.i_val = 3;
    } else if (strcmp(privacy, "wpa3") == 0) {
        ireq.i_val = 4;
    } else {
        debug_log(DEBUG_ERROR, "Invalid privacy mode: %s", privacy);
        close(sock);
        return -1;
    }
    
    if (ioctl(sock, SIOCS80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set wireless privacy mode: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Set wireless privacy mode to %s for interface %s", privacy, name);
    return 0;
}

/**
 * Set wireless transmit power
 * @param name Wireless interface name
 * @param txpower Transmit power in dBm
 * @return 0 on success, -1 on failure
 */
int freebsd_wireless_set_txpower(const char *name, int txpower) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name) {
        debug_log(DEBUG_ERROR, "Invalid parameters for setting wireless transmit power");
        return -1;
    }
    
    if (txpower < -10 || txpower > 30) {
        debug_log(DEBUG_ERROR, "Invalid transmit power value: %d dBm (must be between -10 and 30)", txpower);
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Setting wireless transmit power to %d dBm for interface %s", txpower, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for setting wireless transmit power: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_TXPOWER;
    ireq.i_val = txpower;
    
    if (ioctl(sock, SIOCS80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set wireless transmit power: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Set wireless transmit power to %d dBm for interface %s", txpower, name);
    return 0;
}

/**
 * Create a WLAN interface
 * @param name WLAN interface name
 * @param parent Parent interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_create(const char *name, const char *parent) {
    int sock;
    struct ifreq ifr;
    
    if (!name || !parent) {
        debug_log(DEBUG_ERROR, "Invalid parameters for WLAN interface creation");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Creating WLAN interface %s on parent %s", name, parent);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for WLAN interface creation: %s", strerror(errno));
        return -1;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    
    if (ioctl(sock, SIOCIFCREATE, &ifr) < 0) {
        debug_log(DEBUG_ERROR, "Failed to create WLAN interface: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Created WLAN interface %s", name);
    return 0;
}

/**
 * Set WLAN SSID
 * @param name WLAN interface name
 * @param ssid SSID string
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_set_ssid(const char *name, const char *ssid) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name || !ssid) {
        debug_log(DEBUG_ERROR, "Invalid parameters for setting WLAN SSID");
        return -1;
    }
    
    if (strlen(ssid) > IEEE80211_NWID_LEN) {
        debug_log(DEBUG_ERROR, "SSID too long: %zu characters (maximum is %d)", strlen(ssid), IEEE80211_NWID_LEN);
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Setting WLAN SSID to %s for interface %s", ssid, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for setting WLAN SSID: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_SSID;
    strlcpy((char *)&ireq.i_data, ssid, sizeof(ireq.i_data));
    
    if (ioctl(sock, SIOCS80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set WLAN SSID: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Set WLAN SSID to %s for interface %s", ssid, name);
    return 0;
}

/**
 * Set WLAN BSSID
 * @param name WLAN interface name
 * @param bssid BSSID string
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_set_bssid(const char *name, const char *bssid) {
    int sock;
    struct ieee80211req ireq;
    uint8_t mac[IEEE80211_ADDR_LEN];
    
    if (!name || !bssid) {
        debug_log(DEBUG_ERROR, "Invalid parameters for setting WLAN BSSID");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Setting WLAN BSSID to %s for interface %s", bssid, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for setting WLAN BSSID: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_BSSID;
    
    if (ether_aton_r(bssid, (struct ether_addr *)mac) == NULL) {
        debug_log(DEBUG_ERROR, "Invalid BSSID format: %s", bssid);
        close(sock);
        return -1;
    }
    
    memcpy(&ireq.i_data, mac, IEEE80211_ADDR_LEN);
    
    if (ioctl(sock, SIOCS80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set WLAN BSSID: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Set WLAN BSSID to %s for interface %s", bssid, name);
    return 0;
}

/**
 * Set WLAN channel
 * @param name WLAN interface name
 * @param channel Channel number
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_set_channel(const char *name, int channel) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name) {
        debug_log(DEBUG_ERROR, "Invalid parameters for setting WLAN channel");
        return -1;
    }
    
    if (channel < 1 || channel > 165) {
        debug_log(DEBUG_ERROR, "Invalid channel number: %d (must be between 1 and 165)", channel);
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Setting WLAN channel to %d for interface %s", channel, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for setting WLAN channel: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_CHANNEL;
    ireq.i_val = channel;
    
    if (ioctl(sock, SIOCS80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set WLAN channel: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Set WLAN channel to %d for interface %s", channel, name);
    return 0;
}

/**
 * Set WLAN security
 * @param name WLAN interface name
 * @param security Security string
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_set_security(const char *name, const char *security) {
    int sock;
    struct ieee80211req ireq;
    
    if (!name || !security) {
        debug_log(DEBUG_ERROR, "Invalid parameters for setting WLAN security");
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Setting WLAN security to %s for interface %s", security, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for setting WLAN security: %s", strerror(errno));
        return -1;
    }
    
    memset(&ireq, 0, sizeof(ireq));
    strlcpy(ireq.i_name, name, sizeof(ireq.i_name));
    ireq.i_type = IEEE80211_IOC_SECURITY;
    
    if (strcmp(security, "none") == 0) {
        ireq.i_val = IEEE80211_SECURITY_NONE;
    } else if (strcmp(security, "wep") == 0) {
        ireq.i_val = IEEE80211_SECURITY_WEP;
    } else if (strcmp(security, "wpa") == 0) {
        ireq.i_val = IEEE80211_SECURITY_WPA;
    } else if (strcmp(security, "wpa2") == 0) {
        ireq.i_val = IEEE80211_SECURITY_WPA2;
    } else if (strcmp(security, "wpa3") == 0) {
        ireq.i_val = IEEE80211_SECURITY_WPA3;
    } else {
        debug_log(DEBUG_ERROR, "Invalid security type: %s", security);
        close(sock);
        return -1;
    }
    
    if (ioctl(sock, SIOCS80211, &ireq) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set WLAN security: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Set WLAN security to %s for interface %s", security, name);
    return 0;
}

/**
 * Set WLAN key
 * @param name WLAN interface name
 * @param key Key string
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_set_key(const char *name, const char *key) {
    int sock;
    struct ieee80211req_key wk;
    
    if (!name || !key) {
        debug_log(DEBUG_ERROR, "Invalid parameters for setting WLAN key");
        return -1;
    }
    
    if (strlen(key) < 8 || strlen(key) > 64) {
        debug_log(DEBUG_ERROR, "Invalid key length: %zu characters (must be between 8 and 64)", strlen(key));
        return -1;
    }
    
    debug_log(DEBUG_DEBUG, "Setting WLAN key for interface %s", name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket for setting WLAN key: %s", strerror(errno));
        return -1;
    }
    
    memset(&wk, 0, sizeof(wk));
    strlcpy(wk.ik_name, name, sizeof(wk.ik_name));
    wk.ik_type = IEEE80211_KEY_XMIT | IEEE80211_KEY_RECV;
    wk.ik_keylen = strlen(key);
    memcpy(wk.ik_keydata, key, wk.ik_keylen);
    
    if (ioctl(sock, SIOCS80211KEY, &wk) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set WLAN key: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(DEBUG_INFO, "Set WLAN key for interface %s", name);
    return 0;
} 