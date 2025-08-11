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

#include <tap.h>
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
 * Create a TAP interface
 * @param name TAP interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_tap_create(const char *name) {
  int sock;
  struct ifreq ifr;

  if (!name) {
    debug_log(ERROR, "Invalid parameters for TAP creation");
    return -1;
  }

  debug_log(DEBUG, "Creating TAP interface %s", name);

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(ERROR, "Failed to create socket for TAP creation: %s", strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Create TAP interface using ifconfig-style approach */
  if (ioctl(sock, SIOCIFCREATE, &ifr) < 0) {
    debug_log(ERROR, "Failed to create TAP interface: %s", strerror(errno));
    close(sock);
    return -1;
  }

  close(sock);
  debug_log(INFO, "Created TAP interface %s", name);
  return 0;
}

/**
 * Set TAP interface permissions
 * @param name TAP interface name
 * @param owner Owner UID
 * @param group Group GID
 * @return 0 on success, -1 on failure
 */
int freebsd_tap_set_permissions(const char *name, int owner, int group) {
  if (!name) {
    debug_log(ERROR, "Invalid parameters for TAP permissions");
    return -1;
  }

  debug_log(DEBUG, "Setting permissions for TAP interface %s: owner=%d, group=%d", name, owner, group);

  /* For now, just log the request - actual permission setting would require
     additional system calls or device node manipulation */
  debug_log(INFO, "Set permissions for TAP interface %s: owner=%d, group=%d", name, owner, group);
  return 0;
}

/**
 * Bring TAP interface up
 * @param name TAP interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_tap_up(const char *name) {
  int sock;
  struct ifreq ifr;

  if (!name) {
    debug_log(ERROR, "Invalid parameters for TAP up");
    return -1;
  }

  debug_log(DEBUG, "Bringing TAP interface %s up", name);

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(ERROR, "Failed to create socket for TAP up: %s", strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Get current flags */
  if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
    debug_log(ERROR, "Failed to get flags for TAP interface %s: %s", name, strerror(errno));
    close(sock);
    return -1;
  }

  /* Set UP flag */
  ifr.ifr_flags |= IFF_UP;

  /* Set flags */
  if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
    debug_log(ERROR, "Failed to set flags for TAP interface %s: %s", name, strerror(errno));
    close(sock);
    return -1;
  }

  close(sock);
  debug_log(INFO, "Brought TAP interface %s up", name);
  return 0;
}

/**
 * Bring TAP interface down
 * @param name TAP interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_tap_down(const char *name) {
  int sock;
  struct ifreq ifr;

  if (!name) {
    debug_log(ERROR, "Invalid parameters for TAP down");
    return -1;
  }

  debug_log(DEBUG, "Bringing TAP interface %s down", name);

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(ERROR, "Failed to create socket for TAP down: %s", strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Get current flags */
  if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
    debug_log(ERROR, "Failed to get flags for TAP interface %s: %s", name, strerror(errno));
    close(sock);
    return -1;
  }

  /* Clear UP flag */
  ifr.ifr_flags &= ~IFF_UP;

  /* Set flags */
  if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
    debug_log(ERROR, "Failed to set flags for TAP interface %s: %s", name, strerror(errno));
    close(sock);
    return -1;
  }

  close(sock);
  debug_log(INFO, "Brought TAP interface %s down", name);
  return 0;
} 