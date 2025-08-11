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
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system/freebsd/80211/80211.h>

/**
 * Create a wireless interface
 * @param state Server state
 * @param name Wireless interface name
 * @param parent_name Parent interface name (if any)
 * @return 0 on success, -1 on failure
 */
int wireless_interface_create(netd_state_t *state, const char *name,
                              const char *parent_name) {
  interface_t *wireless_iface;

  if (!state || !name) {
    debug_log(ERROR,
              "Invalid parameters for wireless creation: state=%p, name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Creating wireless interface '%s'", name);

  /* Create the wireless interface using the general interface creation */
  int result = interface_create(state, name, IF_TYPE_WIRELESS);
  if (result < 0) {
    debug_log(ERROR, "Failed to create wireless interface %s", name);
    return -1;
  }

  /* Find the created interface and set wireless-specific properties */
  wireless_iface = interface_find(state, name);
  if (!wireless_iface) {
    debug_log(ERROR, "Failed to find created wireless interface %s", name);
    return -1;
  }

  /* Set wireless properties */
  if (parent_name) {
    strlcpy(wireless_iface->wifi_parent, parent_name,
            sizeof(wireless_iface->wifi_parent));
  }

  /* Set default wireless parameters */
  strlcpy(wireless_iface->wifi_regdomain, "US",
          sizeof(wireless_iface->wifi_regdomain));
  strlcpy(wireless_iface->wifi_country, "US",
          sizeof(wireless_iface->wifi_country));
  strlcpy(wireless_iface->wifi_authmode, "open",
          sizeof(wireless_iface->wifi_authmode));
  strlcpy(wireless_iface->wifi_privacy, "off",
          sizeof(wireless_iface->wifi_privacy));
  wireless_iface->wifi_txpower = 30; /* Default 30 dBm */
  wireless_iface->wifi_bmiss = 10;   /* Default 10 seconds */
  wireless_iface->wifi_scanvalid = 300; /* Default 5 minutes */
  strlcpy(wireless_iface->wifi_features, "none",
          sizeof(wireless_iface->wifi_features));
  wireless_iface->wifi_bintval = 100; /* Default 100ms */

  /* Create wireless interface in FreeBSD */
  if (freebsd_wireless_create(name, parent_name) < 0) {
    debug_log(ERROR,
              "Failed to create wireless interface %s in FreeBSD", name);
    /* Clean up the interface from state */
    interface_delete(state, name);
    return -1;
  }

  debug_log(INFO, "Created wireless interface %s", name);
  return 0;
}

/**
 * Set wireless regulatory domain
 * @param state Server state
 * @param name Wireless interface name
 * @param regdomain Regulatory domain string
 * @return 0 on success, -1 on failure
 */
int wireless_set_regdomain(netd_state_t *state, const char *name,
                           const char *regdomain) {
  interface_t *wireless_iface;

  if (!state || !name || !regdomain) {
    debug_log(ERROR,
              "Invalid parameters for wireless regdomain setting: state=%p, "
              "name=%s, regdomain=%s",
              state, name ? name : "NULL", regdomain ? regdomain : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Setting regdomain '%s' for wireless interface '%s'",
            regdomain, name);

  /* Find wireless interface */
  wireless_iface = interface_find(state, name);
  if (!wireless_iface) {
    debug_log(ERROR, "Wireless interface %s not found", name);
    return -1;
  }

  if (wireless_iface->type != IF_TYPE_WIRELESS) {
    debug_log(ERROR, "Interface %s is not wireless", name);
    return -1;
  }

  /* Set regdomain in FreeBSD */
  if (freebsd_wireless_set_regdomain(name, regdomain) < 0) {
    debug_log(ERROR,
              "Failed to set regdomain %s for wireless %s in FreeBSD",
              regdomain, name);
    return -1;
  }

  /* Update interface state */
  strlcpy(wireless_iface->wifi_regdomain, regdomain,
          sizeof(wireless_iface->wifi_regdomain));

  debug_log(INFO, "Set regdomain %s for wireless interface %s",
            regdomain, name);
  return 0;
}

/**
 * Set wireless country code
 * @param state Server state
 * @param name Wireless interface name
 * @param country Country code string
 * @return 0 on success, -1 on failure
 */
int wireless_set_country(netd_state_t *state, const char *name,
                         const char *country) {
  interface_t *wireless_iface;

  if (!state || !name || !country) {
    debug_log(ERROR,
              "Invalid parameters for wireless country setting: state=%p, "
              "name=%s, country=%s",
              state, name ? name : "NULL", country ? country : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Setting country '%s' for wireless interface '%s'",
            country, name);

  /* Find wireless interface */
  wireless_iface = interface_find(state, name);
  if (!wireless_iface) {
    debug_log(ERROR, "Wireless interface %s not found", name);
    return -1;
  }

  if (wireless_iface->type != IF_TYPE_WIRELESS) {
    debug_log(ERROR, "Interface %s is not wireless", name);
    return -1;
  }

  /* Set country in FreeBSD */
  if (freebsd_wireless_set_country(name, country) < 0) {
    debug_log(ERROR,
              "Failed to set country %s for wireless %s in FreeBSD", country,
              name);
    return -1;
  }

  /* Update interface state */
  strlcpy(wireless_iface->wifi_country, country,
          sizeof(wireless_iface->wifi_country));

  debug_log(INFO, "Set country %s for wireless interface %s", country,
            name);
  return 0;
}

/**
 * Set wireless authentication mode
 * @param state Server state
 * @param name Wireless interface name
 * @param authmode Authentication mode string
 * @return 0 on success, -1 on failure
 */
int wireless_set_authmode(netd_state_t *state, const char *name,
                          const char *authmode) {
  interface_t *wireless_iface;

  if (!state || !name || !authmode) {
    debug_log(ERROR,
              "Invalid parameters for wireless authmode setting: state=%p, "
              "name=%s, authmode=%s",
              state, name ? name : "NULL", authmode ? authmode : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Setting authmode '%s' for wireless interface '%s'",
            authmode, name);

  /* Find wireless interface */
  wireless_iface = interface_find(state, name);
  if (!wireless_iface) {
    debug_log(ERROR, "Wireless interface %s not found", name);
    return -1;
  }

  if (wireless_iface->type != IF_TYPE_WIRELESS) {
    debug_log(ERROR, "Interface %s is not wireless", name);
    return -1;
  }

  /* Set authmode in FreeBSD */
  if (freebsd_wireless_set_authmode(name, authmode) < 0) {
    debug_log(ERROR,
              "Failed to set authmode %s for wireless %s in FreeBSD", authmode,
              name);
    return -1;
  }

  /* Update interface state */
  strlcpy(wireless_iface->wifi_authmode, authmode,
          sizeof(wireless_iface->wifi_authmode));

  debug_log(INFO, "Set authmode %s for wireless interface %s", authmode,
            name);
  return 0;
}

/**
 * Set wireless privacy mode
 * @param state Server state
 * @param name Wireless interface name
 * @param privacy Privacy mode string ("on" or "off")
 * @return 0 on success, -1 on failure
 */
int wireless_set_privacy(netd_state_t *state, const char *name,
                         const char *privacy) {
  interface_t *wireless_iface;

  if (!state || !name || !privacy) {
    debug_log(ERROR,
              "Invalid parameters for wireless privacy setting: state=%p, "
              "name=%s, privacy=%s",
              state, name ? name : "NULL", privacy ? privacy : "NULL");
    return -1;
  }

  debug_log(DEBUG, "Setting privacy '%s' for wireless interface '%s'",
            privacy, name);

  /* Find wireless interface */
  wireless_iface = interface_find(state, name);
  if (!wireless_iface) {
    debug_log(ERROR, "Wireless interface %s not found", name);
    return -1;
  }

  if (wireless_iface->type != IF_TYPE_WIRELESS) {
    debug_log(ERROR, "Interface %s is not wireless", name);
    return -1;
  }

  /* Set privacy in FreeBSD */
  if (freebsd_wireless_set_privacy(name, privacy) < 0) {
    debug_log(ERROR,
              "Failed to set privacy %s for wireless %s in FreeBSD", privacy,
              name);
    return -1;
  }

  /* Update interface state */
  strlcpy(wireless_iface->wifi_privacy, privacy,
          sizeof(wireless_iface->wifi_privacy));

  debug_log(INFO, "Set privacy %s for wireless interface %s", privacy,
            name);
  return 0;
}

/**
 * Set wireless transmit power
 * @param state Server state
 * @param name Wireless interface name
 * @param txpower Transmit power in dBm
 * @return 0 on success, -1 on failure
 */
int wireless_set_txpower(netd_state_t *state, const char *name, int txpower) {
  interface_t *wireless_iface;

  if (!state || !name) {
    debug_log(ERROR,
              "Invalid parameters for wireless txpower setting: state=%p, "
              "name=%s",
              state, name ? name : "NULL");
    return -1;
  }

  if (txpower < 0 || txpower > 30) {
    debug_log(ERROR, "Invalid txpower %d (must be 0-30 dBm)", txpower);
    return -1;
  }

  debug_log(DEBUG, "Setting txpower %d for wireless interface '%s'",
            txpower, name);

  /* Find wireless interface */
  wireless_iface = interface_find(state, name);
  if (!wireless_iface) {
    debug_log(ERROR, "Wireless interface %s not found", name);
    return -1;
  }

  if (wireless_iface->type != IF_TYPE_WIRELESS) {
    debug_log(ERROR, "Interface %s is not wireless", name);
    return -1;
  }

  /* Set txpower in FreeBSD */
  if (freebsd_wireless_set_txpower(name, txpower) < 0) {
    debug_log(ERROR,
              "Failed to set txpower %d for wireless %s in FreeBSD", txpower,
              name);
    return -1;
  }

  /* Update interface state */
  wireless_iface->wifi_txpower = txpower;

  debug_log(INFO, "Set txpower %d for wireless interface %s", txpower,
            name);
  return 0;
} 