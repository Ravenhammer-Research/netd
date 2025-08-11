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
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

#include <80211.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
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
#include <lib80211/lib80211_ioctl.h>
#include <netd.h>

/* IEEE 802.11 constants - using actual FreeBSD values */
/* Note: These are already defined in ieee80211_ioctl.h, so we use the system values */

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
        debug_log(ERROR, "Invalid parameters for wireless interface creation");
        return -1;
    }
    
    debug_log(DEBUG, "Creating wireless interface %s on parent %s", name, parent_name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for wireless interface creation: %s", strerror(errno));
        return -1;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    
    if (ioctl(sock, SIOCIFCREATE, &ifr) < 0) {
        debug_log(ERROR, "Failed to create wireless interface: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Created wireless interface %s", name);
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
    int val;
    
    if (!name || !regdomain) {
        debug_log(ERROR, "Invalid parameters for setting wireless regulatory domain");
        return -1;
    }
    
    debug_log(DEBUG, "Setting wireless regulatory domain to %s for interface %s", regdomain, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for setting wireless regulatory domain: %s", strerror(errno));
        return -1;
    }
    
    val = atoi(regdomain);
    
    if (lib80211_set80211(sock, name, IEEE80211_IOC_REGDOMAIN, val, 0, NULL) < 0) {
        debug_log(ERROR, "Failed to set wireless regulatory domain: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Set wireless regulatory domain to %s for interface %s", regdomain, name);
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
    
    if (!name || !country) {
        debug_log(ERROR, "Invalid parameters for setting wireless country code");
        return -1;
    }
    
    debug_log(DEBUG, "Setting wireless country code to %s for interface %s", country, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for setting wireless country code: %s", strerror(errno));
        return -1;
    }
    
    
    /* Use regulatory domain approach for country code */
    int regdomain = 0;
    if (strcmp(country, "US") == 0) {
        regdomain = 1; /* FCC */
    } else if (strcmp(country, "EU") == 0) {
        regdomain = 2; /* ETSI */
    } else if (strcmp(country, "JP") == 0) {
        regdomain = 3; /* Japan */
    } else if (strcmp(country, "CA") == 0) {
        regdomain = 4; /* Canada */
    } else {
        /* Default to FCC */
        regdomain = 1;
        debug_log(WARN, "Unknown country code %s, defaulting to FCC", country);
    }
    
    if (lib80211_set80211(sock, name, IEEE80211_IOC_REGDOMAIN, regdomain, 0, NULL) < 0) {
        debug_log(ERROR, "Failed to set wireless country code: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Set wireless country code to %s for interface %s", country, name);
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
    int mode = 0;
    
    if (!name || !authmode) {
        debug_log(ERROR, "Invalid parameters for setting wireless authentication mode");
        return -1;
    }
    
    debug_log(DEBUG, "Setting wireless authentication mode to %s for interface %s", authmode, name);
    
    /* Map authmode string to IEEE80211_AUTH_* constants */
    if (strcmp(authmode, "open") == 0) {
        mode = IEEE80211_AUTH_OPEN;
    } else if (strcmp(authmode, "shared") == 0) {
        mode = IEEE80211_AUTH_SHARED;
    } else if (strcmp(authmode, "wpa") == 0) {
        mode = IEEE80211_AUTH_WPA;
    } else {
        debug_log(ERROR, "Unsupported authentication mode: %s", authmode);
        return -1;
    }
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for setting wireless authentication mode: %s", strerror(errno));
        return -1;
    }
    
    if (lib80211_set80211(sock, name, IEEE80211_IOC_AUTHMODE, mode, 0, NULL) < 0) {
        debug_log(ERROR, "Failed to set wireless authentication mode: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Set wireless authentication mode to %s for interface %s", authmode, name);
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
    int mode = 0;
    
    if (!name || !privacy) {
        debug_log(ERROR, "Invalid parameters for setting wireless privacy mode");
        return -1;
    }
    
    debug_log(DEBUG, "Setting wireless privacy mode to %s for interface %s", privacy, name);
    
    /* Map privacy string to IEEE80211_PRIVACY_* constants */
    if (strcmp(privacy, "off") == 0) {
        mode = 0;
    } else if (strcmp(privacy, "on") == 0) {
        mode = 1;
    } else {
        debug_log(ERROR, "Unsupported privacy mode: %s", privacy);
        return -1;
    }
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for setting wireless privacy mode: %s", strerror(errno));
        return -1;
    }
    
    if (lib80211_set80211(sock, name, IEEE80211_IOC_PRIVACY, mode, 0, NULL) < 0) {
        debug_log(ERROR, "Failed to set wireless privacy mode: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Set wireless privacy mode to %s for interface %s", privacy, name);
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
    
    if (!name) {
        debug_log(ERROR, "Invalid parameters for setting wireless transmit power");
        return -1;
    }
    
    debug_log(DEBUG, "Setting wireless transmit power to %d dBm for interface %s", txpower, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for setting wireless transmit power: %s", strerror(errno));
        return -1;
    }
    
    if (lib80211_set80211(sock, name, IEEE80211_IOC_TXPOWER, txpower, 0, NULL) < 0) {
        debug_log(ERROR, "Failed to set wireless transmit power: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Set wireless transmit power to %d dBm for interface %s", txpower, name);
    return 0;
}

/**
 * Delete a wireless interface
 * @param name Wireless interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_wireless_delete(const char *name) {
    int sock;
    struct ifreq ifr;
    
    if (!name) {
        debug_log(ERROR, "Invalid parameters for wireless interface deletion");
        return -1;
    }
    
    debug_log(DEBUG, "Deleting wireless interface %s", name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for wireless interface deletion: %s", strerror(errno));
        return -1;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    
    if (ioctl(sock, SIOCIFDESTROY, &ifr) < 0) {
        debug_log(ERROR, "Failed to delete wireless interface: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Deleted wireless interface %s", name);
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
        debug_log(ERROR, "Invalid parameters for WLAN interface creation");
        return -1;
    }
    
    debug_log(DEBUG, "Creating WLAN interface %s on parent %s", name, parent);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for WLAN interface creation: %s", strerror(errno));
        return -1;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    
    if (ioctl(sock, SIOCIFCREATE, &ifr) < 0) {
        debug_log(ERROR, "Failed to create WLAN interface: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Created WLAN interface %s", name);
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
    
    if (!name || !ssid) {
        debug_log(ERROR, "Invalid parameters for setting WLAN SSID");
        return -1;
    }
    
    debug_log(DEBUG, "Setting WLAN SSID to %s for interface %s", ssid, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for setting WLAN SSID: %s", strerror(errno));
        return -1;
    }
    
    if (lib80211_set80211(sock, name, IEEE80211_IOC_SSID, 0, strlen(ssid), (void *)ssid) < 0) {
        debug_log(ERROR, "Failed to set WLAN SSID: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Set WLAN SSID to %s for interface %s", ssid, name);
    return 0;
}

/**
 * Set WLAN BSSID
 * @param name WLAN interface name
 * @param bssid BSSID string (MAC address)
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_set_bssid(const char *name, const char *bssid) {
    int sock;
    struct ether_addr mac;
    
    if (!name || !bssid) {
        debug_log(ERROR, "Invalid parameters for setting WLAN BSSID");
        return -1;
    }
    
    debug_log(DEBUG, "Setting WLAN BSSID to %s for interface %s", bssid, name);
    
    /* Parse MAC address */
    if (ether_aton_r(bssid, &mac) == NULL) {
        debug_log(ERROR, "Invalid BSSID format: %s", bssid);
        return -1;
    }
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for setting WLAN BSSID: %s", strerror(errno));
        return -1;
    }
    
    if (lib80211_set80211(sock, name, IEEE80211_IOC_BSSID, 0, sizeof(mac), &mac) < 0) {
        debug_log(ERROR, "Failed to set WLAN BSSID: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Set WLAN BSSID to %s for interface %s", bssid, name);
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
    
    if (!name) {
        debug_log(ERROR, "Invalid parameters for setting WLAN channel");
        return -1;
    }
    
    debug_log(DEBUG, "Setting WLAN channel to %d for interface %s", channel, name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for setting WLAN channel: %s", strerror(errno));
        return -1;
    }
    
    if (lib80211_set80211(sock, name, IEEE80211_IOC_CHANNEL, channel, 0, NULL) < 0) {
        debug_log(ERROR, "Failed to set WLAN channel: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Set WLAN channel to %d for interface %s", channel, name);
    return 0;
}

/**
 * Set WLAN security mode
 * @param name WLAN interface name
 * @param security Security mode string
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_set_security(const char *name, const char *security) {
    /* Security mode setting not implemented - requires kernel-specific structures */
    debug_log(WARN, "WLAN security mode setting not implemented for interface %s (security: %s)", name, security);
    return -1;
}

/**
 * Set WLAN key
 * @param name WLAN interface name
 * @param key Key string
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_set_key(const char *name, const char *key) {
    /* WLAN key setting not implemented - requires kernel-specific structures */
    debug_log(WARN, "WLAN key setting not implemented for interface %s (key: %s)", name, key);
    return -1;
}

/**
 * Delete a WLAN interface
 * @param name WLAN interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_wlan_delete(const char *name) {
    int sock;
    struct ifreq ifr;
    
    if (!name) {
        debug_log(ERROR, "Invalid parameters for WLAN interface deletion");
        return -1;
    }
    
    debug_log(DEBUG, "Deleting WLAN interface %s", name);
    
    sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        debug_log(ERROR, "Failed to create socket for WLAN interface deletion: %s", strerror(errno));
        return -1;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    
    if (ioctl(sock, SIOCIFDESTROY, &ifr) < 0) {
        debug_log(ERROR, "Failed to delete WLAN interface: %s", strerror(errno));
        close(sock);
        return -1;
    }
    
    close(sock);
    debug_log(INFO, "Deleted WLAN interface %s", name);
    return 0;
} 