#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <net/if.h>

#include "netd.h"
#include "../../system/freebsd/proto.h"

/**
 * Create a TAP interface
 */
int tap_interface_create(netd_state_t *state, const char *name)
{
    int ret;
    
    if (!state || !name) {
        return -EINVAL;
    }
    
    // Create the base interface
    ret = interface_create(state, name, IF_TYPE_TAP);
    if (ret != 0) {
        syslog(LOG_ERR, "Failed to create TAP interface %s: %d", name, ret);
        return ret;
    }
    
    // Set TAP-specific properties in state
    interface_t *iface = interface_find(state, name);
    if (iface) {
        iface->type = IF_TYPE_TAP;
        iface->flags |= IFF_POINTOPOINT;
        iface->flags |= IFF_MULTICAST;
    }
    
    // Create the TAP interface in the system
    ret = freebsd_tap_create(name);
    if (ret != 0) {
        syslog(LOG_ERR, "Failed to create system TAP interface %s: %d", name, ret);
        // Clean up the interface from state
        interface_delete(state, name);
        return ret;
    }
    
    syslog(LOG_INFO, "Created TAP interface %s", name);
    return 0;
}

/**
 * Set TAP interface properties
 */
int tap_set_properties(netd_state_t *state, const char *name,
                      int mtu, int owner, int group)
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
    
    if (iface->type != IF_TYPE_TAP) {
        return -EINVAL;
    }
    
    // Note: description field not implemented in interface struct
    // Could be added later if needed
    
    // Set MTU if provided
    if (mtu > 0) {
        ret = interface_set_mtu(state, name, mtu);
        if (ret != 0) {
            syslog(LOG_ERR, "Failed to set MTU for TAP interface %s: %d", name, ret);
        }
    }
    
    // Set owner/group if provided
    if (owner > 0 || group > 0) {
        ret = freebsd_tap_set_permissions(name, owner, group);
        if (ret != 0) {
            syslog(LOG_ERR, "Failed to set permissions for TAP interface %s: %d", name, ret);
        }
    }
    
    return ret;
}

/**
 * Get TAP interface information
 */
int tap_get_info(netd_state_t *state, const char *name,
                char **description, int *mtu, int *owner, int *group)
{
    interface_t *iface;
    
    if (!state || !name) {
        return -EINVAL;
    }
    
    iface = interface_find(state, name);
    if (!iface) {
        return -ENOENT;
    }
    
    if (iface->type != IF_TYPE_TAP) {
        return -EINVAL;
    }
    
    if (description) {
        *description = NULL;  // Description field not implemented in interface struct
    }
    
    if (mtu) {
        *mtu = iface->mtu;
    }
    
    // Get owner/group from system if requested
    if (owner || group) {
        int ret = freebsd_tap_get_permissions(name, owner, group);
        if (ret != 0) {
            syslog(LOG_ERR, "Failed to get permissions for TAP interface %s: %d", name, ret);
        }
    }
    
    return 0;
}

/**
 * Set TAP interface up/down
 */
int tap_set_updown(netd_state_t *state, const char *name, int up)
{
    interface_t *iface;
    int ret;
    
    if (!state || !name) {
        return -EINVAL;
    }
    
    iface = interface_find(state, name);
    if (!iface) {
        return -ENOENT;
    }
    
    if (iface->type != IF_TYPE_TAP) {
        return -EINVAL;
    }
    
    if (up) {
        ret = freebsd_tap_up(name);
        if (ret == 0) {
            iface->flags |= IFF_UP;
        }
    } else {
        ret = freebsd_tap_down(name);
        if (ret == 0) {
            iface->flags &= ~IFF_UP;
        }
    }
    
    if (ret != 0) {
        syslog(LOG_ERR, "Failed to set TAP interface %s %s: %d", 
               name, up ? "up" : "down", ret);
    }
    
    return ret;
} 