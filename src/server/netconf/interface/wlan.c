#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <net/if.h>

#include "netd.h"
#include "../../system/freebsd/proto.h"

/**
 * Create a WLAN interface
 */
int wlan_interface_create(netd_state_t *state, const char *name, const char *parent)
{
    int ret;
    
    if (!state || !name || !parent) {
        return -EINVAL;
    }
    
    // Create the base interface
    ret = interface_create(state, name, IF_TYPE_WLAN);
    if (ret != 0) {
        syslog(LOG_ERR, "Failed to create WLAN interface %s: %d", name, ret);
        return ret;
    }
    
    // Set WLAN-specific properties in state
    interface_t *iface = interface_find(state, name);
    if (iface) {
        iface->type = IF_TYPE_WLAN;
        // Note: parent field not implemented in interface struct
        // Could be added later if needed
        iface->flags |= IFF_BROADCAST;
        iface->flags |= IFF_MULTICAST;
    }
    
    // Create the WLAN interface in the system
    ret = freebsd_wlan_create(name, parent);
    if (ret != 0) {
        syslog(LOG_ERR, "Failed to create system WLAN interface %s: %d", name, ret);
        // Clean up the interface from state
        interface_delete(state, name);
        return ret;
    }
    
    syslog(LOG_INFO, "Created WLAN interface %s on parent %s", name, parent);
    return 0;
}

/**
 * Set WLAN interface properties
 */
int wlan_set_properties(netd_state_t *state, const char *name,
                       const char *ssid, const char *bssid, int channel,
                       const char *security, const char *key)
{
    interface_t *iface;
    int ret = 0;
    
    if (!state || !name) {
        return -EINVAL;
    }
    
    iface = interface_find(state, name);
    if (!iface) {
        return -ENOENT;
    }
    
    if (iface->type != INTERFACE_TYPE_WLAN) {
        return -EINVAL;
    }
    
    // Set SSID if provided
    if (ssid) {
        ret = freebsd_wlan_set_ssid(name, ssid);
        if (ret != 0) {
            syslog(LOG_ERR, "Failed to set SSID for WLAN interface %s: %d", name, ret);
        }
    }
    
    // Set BSSID if provided
    if (bssid) {
        ret = freebsd_wlan_set_bssid(name, bssid);
        if (ret != 0) {
            syslog(LOG_ERR, "Failed to set BSSID for WLAN interface %s: %d", name, ret);
        }
    }
    
    // Set channel if provided
    if (channel > 0) {
        ret = freebsd_wlan_set_channel(name, channel);
        if (ret != 0) {
            syslog(LOG_ERR, "Failed to set channel for WLAN interface %s: %d", name, ret);
        }
    }
    
    // Set security if provided
    if (security) {
        ret = freebsd_wlan_set_security(name, security);
        if (ret != 0) {
            syslog(LOG_ERR, "Failed to set security for WLAN interface %s: %d", name, ret);
        }
    }
    
    // Set key if provided
    if (key) {
        ret = freebsd_wlan_set_key(name, key);
        if (ret != 0) {
            syslog(LOG_ERR, "Failed to set key for WLAN interface %s: %d", name, ret);
        }
    }
    
    return ret;
}

/**
 * Get WLAN interface information
 */
int wlan_get_info(netd_state_t *state, const char *name,
                 char **ssid, char **bssid, int *channel,
                 char **security, char **parent)
{
    interface_t *iface;
    
    if (!state || !name) {
        return -EINVAL;
    }
    
    iface = interface_find(state, name);
    if (!iface) {
        return -ENOENT;
    }
    
    if (iface->type != IF_TYPE_WLAN) {
        return -EINVAL;
    }
    
    if (ssid) {
        *ssid = NULL;
        // Get SSID from system
        int ret = freebsd_wlan_get_ssid(name, ssid);
        if (ret != 0) {
            syslog(LOG_ERR, "Failed to get SSID for WLAN interface %s: %d", name, ret);
        }
    }
    
    if (bssid) {
        *bssid = NULL;
        // Get BSSID from system
        int ret = freebsd_wlan_get_bssid(name, bssid);
        if (ret != 0) {
            syslog(LOG_ERR, "Failed to get BSSID for WLAN interface %s: %d", name, ret);
        }
    }
    
    if (channel) {
        *channel = 0;
        // Get channel from system
        int ret = freebsd_wlan_get_channel(name, channel);
        if (ret != 0) {
            syslog(LOG_ERR, "Failed to get channel for WLAN interface %s: %d", name, ret);
        }
    }
    
    if (security) {
        *security = NULL;
        // Get security from system
        int ret = freebsd_wlan_get_security(name, security);
        if (ret != 0) {
            syslog(LOG_ERR, "Failed to get security for WLAN interface %s: %d", name, ret);
        }
    }
    
    if (parent) {
        *parent = NULL;  // Parent field not implemented in interface struct
    }
    
    return 0;
}

/**
 * Scan for available networks
 */
int wlan_scan_networks(netd_state_t *state, const char *name)
{
    interface_t *iface;
    
    if (!state || !name) {
        return -EINVAL;
    }
    
    iface = interface_find(state, name);
    if (!iface) {
        return -ENOENT;
    }
    
    if (iface->type != IF_TYPE_WLAN) {
        return -EINVAL;
    }
    
    // Trigger network scan
    int ret = freebsd_wlan_scan(name);
    if (ret != 0) {
        syslog(LOG_ERR, "Failed to scan networks for WLAN interface %s: %d", name, ret);
    }
    
    return ret;
}

/**
 * Connect to a network
 */
int wlan_connect(netd_state_t *state, const char *name, const char *ssid,
                const char *security, const char *key)
{
    interface_t *iface;
    
    if (!state || !name || !ssid) {
        return -EINVAL;
    }
    
    iface = interface_find(state, name);
    if (!iface) {
        return -ENOENT;
    }
    
    if (iface->type != IF_TYPE_WLAN) {
        return -EINVAL;
    }
    
    // Connect to the specified network
    int ret = freebsd_wlan_connect(name, ssid, security, key);
    if (ret != 0) {
        syslog(LOG_ERR, "Failed to connect WLAN interface %s to %s: %d", name, ssid, ret);
    }
    
    return ret;
} 