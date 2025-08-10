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

#ifndef FREEBSD_80211_H
#define FREEBSD_80211_H

#include <stddef.h>  /* for size_t */
#include <netd.h>    /* for debug functions and types */

/* IEEE 802.11 wireless interface operations */

/* Wireless interface operations */
int freebsd_wireless_create(const char *name, const char *parent_name);
int freebsd_wireless_set_regdomain(const char *name, const char *regdomain);
int freebsd_wireless_set_country(const char *name, const char *country);
int freebsd_wireless_set_authmode(const char *name, const char *authmode);
int freebsd_wireless_set_privacy(const char *name, const char *privacy);
int freebsd_wireless_set_txpower(const char *name, int txpower);
int freebsd_wireless_delete(const char *name);
int freebsd_wireless_show(const char *name, char *regdomain, size_t regdomain_size,
                          char *country, size_t country_size, char *authmode,
                          size_t authmode_size, char *privacy, size_t privacy_size,
                          int *txpower, int *bmiss, int *scanvalid, char *features,
                          size_t features_size, int *bintval, char *parent, size_t parent_size);

/* WLAN interface operations */
int freebsd_wlan_create(const char *name, const char *parent);
int freebsd_wlan_set_ssid(const char *name, const char *ssid);
int freebsd_wlan_set_bssid(const char *name, const char *bssid);
int freebsd_wlan_set_channel(const char *name, int channel);
int freebsd_wlan_set_security(const char *name, const char *security);
int freebsd_wlan_set_key(const char *name, const char *key);
int freebsd_wlan_delete(const char *name);
int freebsd_wlan_show(const char *name, char *ssid, size_t ssid_size,
                      char *bssid, size_t bssid_size, int *channel,
                      char *security, size_t security_size);
int freebsd_wlan_scan(const char *name);
int freebsd_wlan_connect(const char *name, const char *ssid, const char *security, const char *key);

#endif /* FREEBSD_80211_H */ 