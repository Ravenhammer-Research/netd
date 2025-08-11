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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netconf/netconf.h>
#include <netconf/vrf/vrf.h>



/**
 * Create interfaces XML response
 * @param state Server state
 * @param message_id Message ID for the response
 * @return Interfaces XML response string (allocated)
 */
char *create_interfaces_xml_response(netd_state_t *state, const char *message_id) {
  char *response;
  interface_t *iface;

  if (!state || !message_id) {
    return NULL;
  }

  /* Allocate buffer for response */
  response = malloc(NETCONF_RESPONSE_BUFFER_SIZE);
  if (!response) {
    debug_log(ERROR, "Failed to allocate memory for interfaces response");
    return NULL;
  }

  if (prepare_response(response,
                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<rpc-reply xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" "
                 "message-id=\"%s\">\n"
                 "  <data>\n"
                 "    <interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">\n",
                 message_id) == -1)
    goto response_error_size;

  /* Add interface entries */
  int interface_count = 0;
  TAILQ_FOREACH(iface, &state->interfaces, entries) {
    interface_count++;
    debug_log(DEBUG, "Processing interface %d: %s", interface_count, iface->name);
    /* Build groups string */
    char groups_str[256] = "";
    if (iface->group_count > 0) {
      for (int i = 0; i < iface->group_count; i++) {
        if (i > 0) strcat(groups_str, ",");
        strcat(groups_str, iface->groups[i]);
      }
    }
    
    /* Find VRF name for this FIB */
    char vrf_name[64] = "default";
    if (iface->fib > 0) {
      vrf_t *vrf = vrf_find_by_fib(state, iface->fib);
      if (vrf) {
        strncpy(vrf_name, vrf->name, sizeof(vrf_name) - 1);
        vrf_name[sizeof(vrf_name) - 1] = '\0';
      } else {
        snprintf(vrf_name, sizeof(vrf_name), "fib%d", iface->fib);
      }
    }
    
    if (prepare_response(response,
                   "      <interface>\n"
                   "        <name>%s</name>\n"
                   "        <type xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\">"
                   "ianaift:%s</type>\n"
                   "        <enabled>%s</enabled>\n"
                   "        <bind-ni-name>%s</bind-ni-name>\n"
                   "        <mtu>%d</mtu>\n"
                   "        <flags>%d</flags>\n",
                   iface->name,
                   interface_type_to_string(iface->type),
                   iface->enabled ? "true" : "false",
                   vrf_name,
                   iface->mtu,
                   iface->flags) == -1)
      goto response_error_size;

    /* Add IPv4 addresses */
    if (iface->address_count > 0) {
      if (prepare_response(response,
                     "        <ipv4 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">\n") == -1)
        goto response_error_size;
      for (int i = 0; i < iface->address_count; i++) {
        if (prepare_response(response,
                       "          <address>\n"
                       "            <ip>%s</ip>\n"
                       "            <prefix-length>%d</prefix-length>\n"
                       "          </address>\n",
                       iface->addresses[i].addr,
                       iface->addresses[i].prefixlen) == -1)
          goto response_error_size;
      }
      if (prepare_response(response, "        </ipv4>\n") == -1)
        goto response_error_size;
    }

    /* Add IPv6 addresses */
    if (iface->address_count6 > 0) {
      if (prepare_response(response,
                     "        <ipv6 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">\n") == -1)
        goto response_error_size;
      for (int i = 0; i < iface->address_count6; i++) {
        if (prepare_response(response,
                       "          <address>\n"
                       "            <ip>%s</ip>\n"
                       "            <prefix-length>%d</prefix-length>\n"
                       "          </address>\n",
                       iface->addresses6[i].addr,
                       iface->addresses6[i].prefixlen) == -1)
          goto response_error_size;
      }
      if (prepare_response(response, "        </ipv6>\n") == -1)
        goto response_error_size;
    }

    /* Add bridge members for bridge interfaces */
    if (iface->type == IF_TYPE_BRIDGE) {
      /* Find corresponding bridge struct */
      bridge_t *bridge = NULL;
      TAILQ_FOREACH(bridge, &state->bridges, entries) {
        if (strcmp(bridge->name, iface->name) == 0) {
          break;
        }
      }
      
      if (bridge && bridge->member_count > 0) {
        debug_log(DEBUG, "Bridge interface %s has %d members", iface->name, bridge->member_count);
        if (prepare_response(response, "        <bridge-members xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n") == -1)
          goto response_error_size;
        
        for (int i = 0; i < bridge->member_count; i++) {
          if (prepare_response(response, "          <member>%s</member>\n", bridge->members[i]) == -1)
            goto response_error_size;
        }
        
        if (prepare_response(response, "        </bridge-members>\n") == -1)
          goto response_error_size;
      }
    }

    /* Add VLAN information for VLAN interfaces */
    if (iface->type == IF_TYPE_VLAN) {
      /* Find corresponding VLAN struct */
      vlan_t *vlan = NULL;
      TAILQ_FOREACH(vlan, &state->vlans, entries) {
        if (strcmp(vlan->name, iface->name) == 0) {
          break;
        }
      }
      
      if (vlan) {
        debug_log(DEBUG, "VLAN interface %s: id=%d, parent=%s", iface->name, vlan->vlan_id, vlan->vlan_parent);
        if (prepare_response(response, "        <vlan xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n") == -1)
          goto response_error_size;
        
        if (prepare_response(response, "          <vlan-id>%d</vlan-id>\n", vlan->vlan_id) == -1)
          goto response_error_size;
        
        if (strlen(vlan->vlan_parent) > 0) {
          if (prepare_response(response, "          <parent-interface>%s</parent-interface>\n", vlan->vlan_parent) == -1)
            goto response_error_size;
        }
        
        if (prepare_response(response, "          <protocol>%s</protocol>\n", vlan->vlan_proto) == -1)
          goto response_error_size;
        
        if (prepare_response(response, "          <priority>%d</priority>\n", vlan->vlan_pcp) == -1)
          goto response_error_size;
        
        if (prepare_response(response, "        </vlan>\n") == -1)
          goto response_error_size;
      }
    }

    /* Add epair information for epair interfaces */
    if (iface->type == IF_TYPE_EPAIR) {
      /* Find corresponding epair struct */
      epair_t *epair = NULL;
      TAILQ_FOREACH(epair, &state->epairs, entries) {
        if (strcmp(epair->name, iface->name) == 0) {
          break;
        }
      }
      
      if (epair) {
        debug_log(DEBUG, "Epair interface %s: peer=%s", iface->name, epair->peer_name);
        if (prepare_response(response, "        <epair xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n") == -1)
          goto response_error_size;
        
        if (strlen(epair->peer_name) > 0) {
          if (prepare_response(response, "          <peer-interface>%s</peer-interface>\n", epair->peer_name) == -1)
            goto response_error_size;
        }
        
        if (prepare_response(response, "        </epair>\n") == -1)
          goto response_error_size;
      }
    }

    /* Add gif information for gif interfaces */
    if (iface->type == IF_TYPE_GIF) {
      /* Find corresponding gif struct */
      gif_t *gif = NULL;
      TAILQ_FOREACH(gif, &state->gifs, entries) {
        if (strcmp(gif->name, iface->name) == 0) {
          break;
        }
      }
      
      if (gif) {
        debug_log(DEBUG, "Gif interface %s: local=%s, remote=%s", iface->name, gif->tunnel_local, gif->tunnel_remote);
        if (prepare_response(response, "        <gif xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n") == -1)
          goto response_error_size;
        
        if (strlen(gif->tunnel_local) > 0) {
          if (prepare_response(response, "          <tunnel-local>%s</tunnel-local>\n", gif->tunnel_local) == -1)
            goto response_error_size;
        }
        
        if (strlen(gif->tunnel_remote) > 0) {
          if (prepare_response(response, "          <tunnel-remote>%s</tunnel-remote>\n", gif->tunnel_remote) == -1)
            goto response_error_size;
        }
        
        if (prepare_response(response, "        </gif>\n") == -1)
          goto response_error_size;
      }
    }

    /* Add lagg information for lagg interfaces */
    if (iface->type == IF_TYPE_LAGG) {
      /* Find corresponding lagg struct */
      lagg_t *lagg = NULL;
      TAILQ_FOREACH(lagg, &state->laggs, entries) {
        if (strcmp(lagg->name, iface->name) == 0) {
          break;
        }
      }
      
      if (lagg) {
        debug_log(DEBUG, "Lagg interface %s: proto=%s, members=%d", iface->name, lagg->lagg_proto, lagg->member_count);
        if (prepare_response(response, "        <lagg xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n") == -1)
          goto response_error_size;
        
        if (strlen(lagg->lagg_proto) > 0) {
          if (prepare_response(response, "          <protocol>%s</protocol>\n", lagg->lagg_proto) == -1)
            goto response_error_size;
        }
        
        if (lagg->member_count > 0) {
          if (prepare_response(response, "          <members>\n") == -1)
            goto response_error_size;
          
          for (int i = 0; i < lagg->member_count; i++) {
            if (prepare_response(response, "            <member>%s</member>\n", lagg->members[i]) == -1)
              goto response_error_size;
          }
          
          if (prepare_response(response, "          </members>\n") == -1)
            goto response_error_size;
        }
        
        if (prepare_response(response, "        </lagg>\n") == -1)
          goto response_error_size;
      }
    }

    /* Add wifi information for wireless interfaces */
    if (iface->type == IF_TYPE_WIRELESS) {
      /* Find corresponding wifi struct */
      wifi_t *wifi = NULL;
      TAILQ_FOREACH(wifi, &state->wifis, entries) {
        if (strcmp(wifi->name, iface->name) == 0) {
          break;
        }
      }
      
      if (wifi) {
        debug_log(DEBUG, "Wifi interface %s: regdomain=%s, country=%s, authmode=%s", 
                  iface->name, wifi->regdomain, wifi->country, wifi->authmode);
        if (prepare_response(response, "        <wifi xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n") == -1)
          goto response_error_size;
        
        if (strlen(wifi->regdomain) > 0) {
          if (prepare_response(response, "          <regdomain>%s</regdomain>\n", wifi->regdomain) == -1)
            goto response_error_size;
        }
        
        if (strlen(wifi->country) > 0) {
          if (prepare_response(response, "          <country>%s</country>\n", wifi->country) == -1)
            goto response_error_size;
        }
        
        if (strlen(wifi->authmode) > 0) {
          if (prepare_response(response, "          <authmode>%s</authmode>\n", wifi->authmode) == -1)
            goto response_error_size;
        }
        
        if (strlen(wifi->privacy) > 0) {
          if (prepare_response(response, "          <privacy>%s</privacy>\n", wifi->privacy) == -1)
            goto response_error_size;
        }
        
        if (wifi->txpower > 0) {
          if (prepare_response(response, "          <txpower>%d</txpower>\n", wifi->txpower) == -1)
            goto response_error_size;
        }
        
        if (strlen(wifi->parent) > 0) {
          if (prepare_response(response, "          <parent>%s</parent>\n", wifi->parent) == -1)
            goto response_error_size;
        }
        
        if (prepare_response(response, "        </wifi>\n") == -1)
          goto response_error_size;
      }
    }

    if (prepare_response(response,
                   "        <groups>%s</groups>\n"
                   "      </interface>\n",
                   groups_str) == -1)
      goto response_error_size;
    debug_log(DEBUG, "Completed interface %d: %s", interface_count, iface->name);
  }

    if (prepare_response(response,
                 "    </interfaces>\n"
                 "  </data>\n"
                 "</rpc-reply>") == -1)
    goto response_error_size;

  debug_log(DEBUG, "Generated interfaces XML response");
  return response;

response_error_size:
  debug_log(ERROR, "Response too big");
  free(response);
  return NULL;
} 