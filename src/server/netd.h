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

#ifndef NETD_H
#define NETD_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>
#include <libyang/libyang.h>
#include <libnetconf2/netconf.h>
#include <types.h>
#include <debug.h>

/* System query data structure for temporary system state */
typedef struct netd_system_query {
  TAILQ_HEAD(netd_system_interface_list, netd_interface) interfaces;
  TAILQ_HEAD(netd_bridge_list, netd_bridge) bridges;
  TAILQ_HEAD(netd_vlan_list, netd_vlan) vlans;
  TAILQ_HEAD(netd_wifi_list, netd_wifi) wifis;
  TAILQ_HEAD(netd_epair_list, netd_epair) epairs;
  TAILQ_HEAD(netd_gif_list, netd_gif) gifs;
  TAILQ_HEAD(netd_gre_list, netd_gre) gres;
  TAILQ_HEAD(netd_lagg_list, netd_lagg) laggs;
  TAILQ_HEAD(netd_carp_list, netd_carp) carps;
  TAILQ_HEAD(netd_wg_list, netd_wg) wgs;
  TAILQ_HEAD(netd_vxlan_list, netd_vxlan) vxlans;
  TAILQ_HEAD(netd_route_list, netd_route) routes;
} netd_system_query_t;

/* Global state structure */
typedef struct netd_state {
  TAILQ_HEAD(netd_vrf_list, netd_vrf) vrfs;
  struct ly_ctx *yang_ctx;
  int socket_fd;
  int debug_level;
  bool transaction_active;
  struct nc_session *sessions[MAX_NETCONF_SESSIONS];
  bool netconf_server_running;
} netd_state_t;

/* Global state variable */
extern netd_state_t netd_state;

#endif /* NETD_H */