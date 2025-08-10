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
 * Show GIF interface information
 * @param name GIF interface name
 * @param local_addr Buffer to store local tunnel address
 * @param local_addr_size Size of local_addr buffer
 * @param remote_addr Buffer to store remote tunnel address
 * @param remote_addr_size Size of remote_addr buffer
 * @return 0 on success, -1 on failure
 */
int freebsd_gif_show(const char *name, char *local_addr, size_t local_addr_size, 
                     char *remote_addr, size_t remote_addr_size) {
  int sock;
  struct ifreq ifr;

  if (!name || !local_addr || !remote_addr) {
    debug_log(DEBUG_ERROR, "Invalid parameters for GIF show");
    return -1;
  }

  debug_log(DEBUG_DEBUG, "Showing GIF interface %s", name);

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(DEBUG_ERROR, "Failed to create socket for GIF show: %s", strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Get interface flags to check if it exists */
  if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
    debug_log(DEBUG_ERROR, "GIF interface %s does not exist: %s", name, strerror(errno));
    close(sock);
    return -1;
  }

  /* For GIF interfaces, we can try to get tunnel endpoint info */
  /* This is a simplified implementation - actual tunnel info may require more complex logic */
  strlcpy(local_addr, "unknown", local_addr_size);
  strlcpy(remote_addr, "unknown", remote_addr_size);

  close(sock);
  debug_log(DEBUG_INFO, "Showed GIF interface %s", name);
  return 0;
} 