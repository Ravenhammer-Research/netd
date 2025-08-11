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

 #include <netd.h>
#include <lagg.h>
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

/**
 * Show LAGG interface information
 * @param name LAGG interface name
 * @param protocol Buffer to store protocol name
 * @param protocol_size Size of protocol buffer
 * @param members Array to store member names
 * @param max_members Maximum number of members
 * @param member_count Pointer to store actual member count
 * @return 0 on success, -1 on failure
 */
int freebsd_lagg_show(const char *name, char *protocol, size_t protocol_size, 
                      char (*members)[MAX_IFNAME_LEN], int max_members, int *member_count) {
  (void)max_members; /* Unused in this simplified implementation */
  int sock;
  struct ifreq ifr;

  if (!name || !protocol || !members || !member_count) {
    debug_log(ERROR, "Invalid parameters for LAGG show");
    return -1;
  }

  debug_log(DEBUG, "Showing LAGG interface %s", name);

  /* Create socket for ioctl */
  sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sock < 0) {
    debug_log(ERROR, "Failed to create socket for LAGG show: %s", strerror(errno));
    return -1;
  }

  /* Set up interface request */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  /* Get interface flags to check if it exists */
  if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
    debug_log(ERROR, "LAGG interface %s does not exist: %s", name, strerror(errno));
    close(sock);
    return -1;
  }

  /* For LAGG interfaces, we can try to get protocol and member info */
  /* This is a simplified implementation - actual info may require more complex logic */
  strlcpy(protocol, "unknown", protocol_size);
  *member_count = 0; /* No members found in this simplified implementation */

  close(sock);
  debug_log(INFO, "Showed LAGG interface %s", name);
  return 0;
} 