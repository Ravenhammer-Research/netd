#include <netd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <net/if.h>
#include <interface.h>
#include <system/freebsd/loopback/loopback.h>

/**
 * Create a loopback interface
 */
int loopback_interface_create(netd_state_t *state, const char *name)
{
    int ret;
    
    if (!state || !name) {
        return -EINVAL;
    }
    
    // Create the base interface
    ret = interface_create(state, name, IF_TYPE_LOOPBACK);
    if (ret != 0) {
        syslog(LOG_ERR, "Failed to create loopback interface %s: %d", name, ret);
        return ret;
    }
    
    // Set loopback-specific properties in state
    interface_t *iface = interface_find(state, name);
    if (iface) {
        iface->type = IF_TYPE_LOOPBACK;
        iface->flags |= IFF_LOOPBACK;
    }
    
    // Create the loopback interface in the system
    ret = freebsd_loopback_create(name);
    if (ret != 0) {
        syslog(LOG_ERR, "Failed to create system loopback interface %s: %d", name, ret);
        // Clean up the interface from state
        interface_delete(state, name);
        return ret;
    }
    
    syslog(LOG_INFO, "Created loopback interface %s", name);
    return 0;
}

/**
 * Set loopback interface properties
 */
int loopback_set_properties(netd_state_t *state, const char *name, 
                           int mtu)
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
    
    if (iface->type != IF_TYPE_LOOPBACK) {
        return -EINVAL;
    }
    
    // Note: description field not implemented in interface struct
    // Could be added later if needed
    
    // Set MTU if provided
    if (mtu > 0) {
        ret = interface_set_mtu(state, name, mtu);
        if (ret != 0) {
            syslog(LOG_ERR, "Failed to set MTU for loopback interface %s: %d", name, ret);
        }
    }
    
    return ret;
}

/**
 * Get loopback interface information
 */
int loopback_get_info(netd_state_t *state, const char *name,
                     char **description, int *mtu)
{
    interface_t *iface;
    
    if (!state || !name) {
        return -EINVAL;
    }
    
    iface = interface_find(state, name);
    if (!iface) {
        return -ENOENT;
    }
    
    if (iface->type != IF_TYPE_LOOPBACK) {
        return -EINVAL;
    }
    
    if (description) {
        // Description field not implemented in interface struct
        *description = NULL;
    }
    
    if (mtu) {
        *mtu = iface->mtu;
    }
    
    return 0;
} 