/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the the following disclaimer.
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
#include <netconf/netconf.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>

static netd_state_t state;
static bool running = true;

/**
 * Signal handler for graceful shutdown
 * @param sig Signal number
 */
static void signal_handler(int sig) {
  debug_log(INFO, "Received shutdown signal %d (%s)", sig,
            sig == SIGINT    ? "SIGINT"
            : sig == SIGTERM ? "SIGTERM"
                             : "UNKNOWN");
  running = false;
}

/**
 * Setup Unix domain socket
 * @return Socket file descriptor on success, -1 on failure
 */
static int setup_socket(void) {
  int sock;
  struct sockaddr_un addr;

  debug_log(INFO, "Setting up Unix domain socket at %s",
            NETD_SOCKET_PATH);

  /* Create socket */
  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock < 0) {
    debug_log(ERROR, "Failed to create socket: %s", strerror(errno));
    return -1;
  }
  debug_log(DEBUG, "Created socket with file descriptor %d", sock);

  /* Set socket options */
  int opt = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    debug_log(ERROR, "Failed to set socket options: %s", strerror(errno));
    close(sock);
    return -1;
  }
  debug_log(DEBUG, "Set socket reuse address option");

  /* Make socket non-blocking for signal handling */
  int flags = fcntl(sock, F_GETFL, 0);
  if (flags < 0) {
    debug_log(ERROR, "Failed to get socket flags: %s", strerror(errno));
    close(sock);
    return -1;
  }
  if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
    debug_log(ERROR, "Failed to set socket non-blocking: %s",
              strerror(errno));
    close(sock);
    return -1;
  }
  debug_log(DEBUG, "Set socket to non-blocking mode");

  /* Remove existing socket file */
  if (unlink(NETD_SOCKET_PATH) == 0) {
    debug_log(DEBUG, "Removed existing socket file %s", NETD_SOCKET_PATH);
  } else if (errno != ENOENT) {
    debug_log(WARN, "Failed to remove existing socket file %s: %s",
              NETD_SOCKET_PATH, strerror(errno));
  }

  /* Bind socket */
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strlcpy(addr.sun_path, NETD_SOCKET_PATH, sizeof(addr.sun_path));

  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    debug_log(ERROR, "Failed to bind socket %s: %s", NETD_SOCKET_PATH,
              strerror(errno));
    close(sock);
    return -1;
  }
  debug_log(DEBUG, "Bound socket to %s", NETD_SOCKET_PATH);

  /* Listen for connections */
  if (listen(sock, 5) < 0) {
    debug_log(ERROR, "Failed to listen on socket: %s", strerror(errno));
    close(sock);
    return -1;
  }
  debug_log(DEBUG, "Socket listening with backlog of 5");

  /* Set socket permissions */
  if (chmod(NETD_SOCKET_PATH, 0666) < 0) {
    debug_log(WARN, "Failed to set socket permissions: %s",
              strerror(errno));
  } else {
    debug_log(DEBUG, "Set socket permissions to 0666");
  }

  debug_log(INFO, "Unix domain socket setup complete: %s (fd: %d)",
            NETD_SOCKET_PATH, sock);
  return sock;
}

/**
 * Handle client connection
 * @param client_sock Client socket file descriptor
 */
static void handle_client(int client_sock) {
  char buffer[8192];
  char *response = NULL;
  ssize_t bytes_read, bytes_written;
  int request_count = 0;

  debug_log(INFO, "Handling client connection on socket %d", client_sock);

  /* Handle multiple requests on the same connection */
  while (1) {
    request_count++;
    debug_log(DEBUG1, "Waiting for request #%d from client...",
              request_count);

    /* Read request */
    bytes_read = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
      if (bytes_read == 0) {
        debug_log(INFO, "Client closed connection after %d requests",
                  request_count - 1);
      } else {
        debug_log(ERROR, "Failed to read from client socket %d: %s",
                  client_sock, strerror(errno));
      }
      close(client_sock);
      return;
    }

    buffer[bytes_read] = '\0';
    debug_log(DEBUG1, "Received %zd bytes from client (request #%d)",
              bytes_read, request_count);

      if (state.debug_level >= DEBUG2) {
  debug_log(DEBUG2, "Request content: %s", buffer);
    }

    /* Handle NETCONF request */
    debug_log(DEBUG1, "Processing NETCONF request #%d", request_count);
    if (netconf_handle_request(&state, buffer, &response) < 0) {
      debug_log(ERROR, "Failed to handle NETCONF request #%d",
                request_count);
      close(client_sock);
      return;
    }

    /* Send response */
    if (response) {
      debug_log(DEBUG1, "Sending response for request #%d (%zu bytes)",
                request_count, strlen(response));
      bytes_written = send(client_sock, response, strlen(response), 0);
      if (bytes_written < 0) {
          debug_log(ERROR,
            "Failed to send response to client socket %d: %s",
                  client_sock, strerror(errno));
        free(response);
        close(client_sock);
        return;
      } else {
          debug_log(DEBUG1,
            "Successfully sent response (%zd bytes) for request #%d",
                  bytes_written, request_count);
      }
      free(response);
      response = NULL;
    } else {
      debug_log(WARN, "No response generated for request #%d",
                request_count);
    }
  }
}

/**
 * Print usage information
 * @param progname Program name
 */
static void print_usage(const char *progname) {
  fprintf(stderr, "Usage: %s [OPTIONS]\n", progname);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -d, --debug LEVEL    Set debug level (0-5)\n");
  fprintf(stderr, "  -h, --help           Show this help message\n");
  fprintf(stderr, "Debug levels:\n");
  fprintf(stderr, "  0 = None\n");
  fprintf(stderr, "  1 = Error\n");
  fprintf(stderr, "  2 = Warning\n");
  fprintf(stderr, "  3 = Info\n");
  fprintf(stderr, "  4 = Debug\n");
  fprintf(stderr, "  5 = Trace\n");
}

/**
 * Main function
 * @param argc Argument count
 * @param argv Argument vector
 * @return 0 on success, 1 on failure
 */
int main(int argc, char *argv[]) {
  int server_sock, client_sock;
  struct sockaddr_un client_addr;
  socklen_t client_len;
  debug_level_t debug_level = NONE;
  int opt;

  debug_log(INFO, "netd server starting (version: %s)", "1.0.0");

  /* Parse command line options */
  while ((opt = getopt(argc, argv, "d:h")) != -1) {
    switch (opt) {
    case 'd':
      debug_level = atoi(optarg);
      if (debug_level < NONE || debug_level > DEBUG2) {
        fprintf(stderr, "Invalid debug level: %s\n", optarg);
        return 1;
      }
      debug_log(INFO, "Debug level set to %d via command line",
                debug_level);
      break;
    case 'h':
      print_usage(argv[0]);
      return 0;
    default:
      print_usage(argv[0]);
      return 1;
    }
  }

  /* Initialize debug logging */
  debug_log(INFO, "Initializing debug logging system");
  debug_init(debug_level);

  /* Initialize syslog if not in debug mode */
  if (debug_level == NONE) {
    debug_log(INFO, "Initializing syslog for production logging");
    openlog("netd", LOG_PID | LOG_CONS, LOG_DAEMON);
  }

  /* Set up signal handlers */
  debug_log(DEBUG, "Setting up signal handlers for SIGINT and SIGTERM");
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  /* Initialize server state */
  debug_log(DEBUG, "Initializing server state");
  memset(&state, 0, sizeof(state));
  TAILQ_INIT(&state.vrfs);
  TAILQ_INIT(&state.interfaces);
  TAILQ_INIT(&state.bridges);
  TAILQ_INIT(&state.vlans);
  TAILQ_INIT(&state.wifis);
  TAILQ_INIT(&state.epairs);
  TAILQ_INIT(&state.gifs);
  TAILQ_INIT(&state.laggs);
  TAILQ_INIT(&state.routes);
  TAILQ_INIT(&state.pending_changes);
  state.debug_level = debug_level;
  state.transaction_active = false;
  debug_log(DEBUG, "Server state initialized successfully");

  /* Initialize YANG context */
  debug_log(INFO, "Initializing YANG context");
  if (yang_init(&state) < 0) {
    debug_log(ERROR, "Failed to initialize YANG context");
    return 1;
  }

  /* Load configuration */
  debug_log(INFO, "Loading configuration");
  if (config_load(&state) < 0) {
    debug_log(ERROR, "Failed to load configuration");
    yang_cleanup(&state);
    return 1;
  }
  debug_log(INFO, "Configuration loaded successfully");

  /* Setup Unix domain socket */
  debug_log(INFO, "Setting up Unix domain socket");
  server_sock = setup_socket();
  if (server_sock < 0) {
    debug_log(ERROR, "Failed to setup socket");
    yang_cleanup(&state);
    return 1;
  }
  state.socket_fd = server_sock;

  debug_log(INFO,
            "netd server started successfully and ready to accept connections");

  /* Main server loop */
  int loop_iteration = 0;
  while (running) {
    loop_iteration++;
    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(server_sock, &readfds);

    /* Set timeout to allow checking running flag */
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

      debug_log(DEBUG2,
            "Server loop iteration %d: waiting for connections...",
              loop_iteration);
    int result = select(server_sock + 1, &readfds, NULL, NULL, &timeout);

    if (result < 0) {
      if (errno == EINTR) {
        /* Interrupted by signal, check running flag */
          debug_log(DEBUG1,
            "select() interrupted by signal, checking running flag");
        continue;
      }
      debug_log(ERROR, "select() failed on iteration %d: %s",
                loop_iteration, strerror(errno));
      break;
    }

    if (result == 0) {
      /* Timeout, check running flag and continue */
        debug_log(DEBUG2,
            "select() timeout on iteration %d, checking running flag",
                loop_iteration);
      continue;
    }

    debug_log(DEBUG1, "select() returned %d on iteration %d", result,
              loop_iteration);

    if (FD_ISSET(server_sock, &readfds)) {
      client_len = sizeof(client_addr);
      client_sock =
          accept(server_sock, (struct sockaddr *)&client_addr, &client_len);

      if (client_sock < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
          debug_log(ERROR,
                    "Failed to accept connection on iteration %d: %s",
                    loop_iteration, strerror(errno));
        } else {
          debug_log(DEBUG1, "Accept would block on iteration %d",
                    loop_iteration);
        }
        continue;
      }

      /* Set client socket to blocking mode */
      int flags = fcntl(client_sock, F_GETFL, 0);
      if (flags >= 0) {
        fcntl(client_sock, F_SETFL, flags & ~O_NONBLOCK);
        debug_log(DEBUG, "Set client socket %d to blocking mode",
                  client_sock);
      }

      debug_log(INFO, "Accepted client connection from %s on socket %d",
                client_addr.sun_path[0] ? client_addr.sun_path : "unknown",
                client_sock);
      handle_client(client_sock);
    }
  }

  /* Cleanup */
  debug_log(INFO, "Shutting down netd server");

  if (server_sock >= 0) {
    debug_log(DEBUG, "Closing server socket %d", server_sock);
    close(server_sock);
  }

  if (unlink(NETD_SOCKET_PATH) == 0) {
    debug_log(DEBUG, "Removed socket file %s", NETD_SOCKET_PATH);
  } else if (errno != ENOENT) {
    debug_log(WARN, "Failed to remove socket file %s: %s",
              NETD_SOCKET_PATH, strerror(errno));
  }

  /* Save configuration */
  debug_log(INFO, "Saving final configuration");
  if (config_save(&state) < 0) {
    debug_log(ERROR, "Failed to save configuration during shutdown");
  } else {
    debug_log(INFO, "Configuration saved successfully");
  }

  /* Cleanup YANG context */
  debug_log(INFO, "Cleaning up YANG context");
  yang_cleanup(&state);

  /* Close syslog */
    if (debug_level == NONE) {
  debug_log(DEBUG, "Closing syslog");
    closelog();
  }

  debug_log(INFO, "netd server stopped successfully");
  return 0;
}