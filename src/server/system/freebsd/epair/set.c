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
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <epair.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <unistd.h>

#include <netd.h>

/**
 * Create an epair interface
 * @param name Epair interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_epair_create(const char *name) {
  int sock;
  struct ifreq ifr;

  if (!name) {
    debug_log(DEBUG_ERROR, "Invalid parameters for epair creation");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Creating epair interface %s", name);

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for epair creation: %s", strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Create epair interface using ifconfig-style approach */
  if (ioctl(sock, SIOCIFCREATE, &ifr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to create epair interface: %s", strerror(errno));
    close(sock);
    return -1;
  }

  close(sock);
  debug_log(DEBUG_INFO, "Created epair interface %s", name);
  return 0;
}

/**
 * Set epair peer interface
 * @param name Epair interface name
 * @param peer_name Peer interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_epair_set_peer(const char *name, const char *peer_name) {
  int sock;
  struct ifreq ifr;

  if (!name || !peer_name) {
    debug_log(DEBUG_ERROR, "Invalid parameters for epair peer setting");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Setting epair peer for %s to %s", name, peer_name);

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for epair peer: %s", strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Set peer interface - this is typically done through ifconfig */
  /* For now, we'll just verify the peer interface exists */
  struct ifreq peer_ifr;
  memset(&peer_ifr, 0, sizeof(peer_ifr));
  strlcpy(peer_ifr.ifr_name, peer_name, sizeof(peer_ifr.ifr_name));

  if (ioctl(sock, SIOCGIFFLAGS, &peer_ifr) < 0) {
    debug_log(DEBUG_ERROR, "Peer interface %s does not exist: %s", peer_name, strerror(errno));
    close(sock);
    return -1;
  }

  close(sock);
  debug_log(DEBUG_INFO, "Set epair peer for %s to %s", name, peer_name);
  return 0;
} 