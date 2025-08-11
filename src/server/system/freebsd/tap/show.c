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
 * Show TAP interface information
 * @param name TAP interface name
 * @param owner Owner UID (output)
 * @param group Group GID (output)
 * @param status Interface status (output)
 * @return 0 on success, -1 on failure
 */
int freebsd_tap_show(const char *name, int *owner, int *group, int *status) {
  int sock;
  struct ifreq ifr;

  if (!name || !owner || !group || !status) {
    debug_log(ERROR, "Invalid parameters for TAP show");
    return -1;
  }

  debug_log(DEBUG, "Showing TAP interface %s", name);

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(ERROR, "Failed to create socket for TAP show: %s", strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Get interface flags */
  if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
    debug_log(ERROR, "Failed to get TAP interface flags: %s", strerror(errno));
    close(sock);
    return -1;
  }

  /* Set status based on flags */
  *status = (ifr.ifr_flags & IFF_UP) ? 1 : 0;

  /* For now, set owner and group to unknown since these ioctls are not standard */
  *owner = -1;
  *group = -1;

  close(sock);
  debug_log(INFO, "Showed TAP interface %s (owner: %d, group: %d, status: %d)", 
            name, *owner, *group, *status);
  return 0;
} 