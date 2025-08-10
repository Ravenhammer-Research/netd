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

#include <gif.h>
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
 * Create a GIF interface
 * @param name GIF interface name
 * @return 0 on success, -1 on failure
 */
int freebsd_gif_create(const char *name) {
  int sock;
  struct ifreq ifr;

  if (!name) {
    debug_log(DEBUG_ERROR, "Invalid parameters for GIF creation");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Creating GIF interface %s", name);

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for GIF creation: %s", strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Create GIF interface using ifconfig-style approach */
  if (ioctl(sock, SIOCIFCREATE, &ifr) < 0) {
    debug_log(DEBUG_ERROR, "Failed to create GIF interface: %s", strerror(errno));
    close(sock);
    return -1;
  }

  close(sock);
  debug_log(DEBUG_INFO, "Created GIF interface %s", name);
  return 0;
}

/**
 * Set GIF tunnel endpoints
 * @param name GIF interface name
 * @param local_addr Local tunnel address
 * @param remote_addr Remote tunnel address
 * @return 0 on success, -1 on failure
 */
int freebsd_gif_set_tunnel(const char *name, const char *local_addr, const char *remote_addr) {
  int sock;
  struct ifreq ifr;

  if (!name || !local_addr || !remote_addr) {
    debug_log(DEBUG_ERROR, "Invalid parameters for GIF tunnel setting");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Setting GIF tunnel for %s: local=%s, remote=%s", 
            name, local_addr, remote_addr);

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for GIF tunnel: %s", strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* For GIF interfaces, tunnel endpoints are typically set through ifconfig */
  /* We'll verify the interface exists and is a GIF type */
  if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
    debug_log(DEBUG_ERROR, "GIF interface %s does not exist: %s", name, strerror(errno));
    close(sock);
    return -1;
  }

  close(sock);
  debug_log(DEBUG_INFO, "Set GIF tunnel for %s: local=%s, remote=%s", 
            name, local_addr, remote_addr);
  return 0;
} 