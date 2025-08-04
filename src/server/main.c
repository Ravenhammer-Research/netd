/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the the following disclaimer.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "netd.h"
#include <sys/un.h>
#include <signal.h>
#include <syslog.h>
#include <getopt.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>

static netd_state_t state;
static bool running = true;

/**
 * Signal handler for graceful shutdown
 * @param sig Signal number
 */
static void signal_handler(int sig)
{
    (void)sig;
    debug_log(DEBUG_INFO, "Received shutdown signal");
    running = false;
}

/**
 * Setup Unix domain socket
 * @return Socket file descriptor on success, -1 on failure
 */
static int setup_socket(void)
{
    int sock;
    struct sockaddr_un addr;

    /* Create socket */
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to create socket: %s", strerror(errno));
        return -1;
    }

    /* Set socket options */
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set socket options: %s", strerror(errno));
        close(sock);
        return -1;
    }

    /* Make socket non-blocking for signal handling */
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
        debug_log(DEBUG_ERROR, "Failed to get socket flags: %s", strerror(errno));
        close(sock);
        return -1;
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
        debug_log(DEBUG_ERROR, "Failed to set socket non-blocking: %s", strerror(errno));
        close(sock);
        return -1;
    }

    /* Remove existing socket file */
    unlink(NETD_SOCKET_PATH);

    /* Bind socket */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strlcpy(addr.sun_path, NETD_SOCKET_PATH, sizeof(addr.sun_path));

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        debug_log(DEBUG_ERROR, "Failed to bind socket %s: %s", NETD_SOCKET_PATH, strerror(errno));
        close(sock);
        return -1;
    }

    /* Listen for connections */
    if (listen(sock, 5) < 0) {
        debug_log(DEBUG_ERROR, "Failed to listen on socket: %s", strerror(errno));
        close(sock);
        return -1;
    }

    /* Set socket permissions */
    chmod(NETD_SOCKET_PATH, 0666);

    debug_log(DEBUG_INFO, "Unix domain socket setup complete: %s", NETD_SOCKET_PATH);
    return sock;
}

/**
 * Handle client connection
 * @param client_sock Client socket file descriptor
 */
static void handle_client(int client_sock)
{
    char buffer[8192];
    char *response = NULL;
    ssize_t bytes_read, bytes_written;

    debug_log(DEBUG_DEBUG, "Handling client connection");

    /* Handle multiple requests on the same connection */
    while (1) {
        /* Read request */
        debug_log(DEBUG_DEBUG, "Waiting for data from client...");
        bytes_read = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                debug_log(DEBUG_INFO, "Client closed connection");
            } else {
                debug_log(DEBUG_ERROR, "Failed to read from client: %s", strerror(errno));
            }
            close(client_sock);
            return;
        }
        debug_log(DEBUG_DEBUG, "Received %zd bytes from client", bytes_read);
        buffer[bytes_read] = '\0';

        debug_log(DEBUG_DEBUG, "Received request: %s", buffer);

        /* Handle NETCONF request */
        if (netconf_handle_request(&state, buffer, &response) < 0) {
            debug_log(DEBUG_ERROR, "Failed to handle NETCONF request");
            close(client_sock);
            return;
        }

        /* Send response */
        if (response) {
            bytes_written = send(client_sock, response, strlen(response), 0);
            if (bytes_written < 0) {
                debug_log(DEBUG_ERROR, "Failed to send response: %s", strerror(errno));
                close(client_sock);
                return;
            } else {
                debug_log(DEBUG_DEBUG, "Sent response (%zd bytes)", bytes_written);
            }
            free(response);
        }
    }
}

/**
 * Print usage information
 * @param progname Program name
 */
static void print_usage(const char *progname)
{
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
int main(int argc, char *argv[])
{
    int server_sock, client_sock;
    struct sockaddr_un client_addr;
    socklen_t client_len;
    debug_level_t debug_level = DEBUG_NONE;
    int opt;

    /* Parse command line options */
    while ((opt = getopt(argc, argv, "d:h")) != -1) {
        switch (opt) {
            case 'd':
                debug_level = atoi(optarg);
                if (debug_level < DEBUG_NONE || debug_level > DEBUG_TRACE) {
                    fprintf(stderr, "Invalid debug level: %s\n", optarg);
                    return 1;
                }
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
    debug_init(debug_level);

    /* Initialize syslog if not in debug mode */
    if (debug_level == DEBUG_NONE) {
        openlog("netd", LOG_PID | LOG_CONS, LOG_DAEMON);
    }

    /* Set up signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Initialize server state */
    memset(&state, 0, sizeof(state));
    TAILQ_INIT(&state.vrfs);
    TAILQ_INIT(&state.interfaces);
    TAILQ_INIT(&state.routes);
    TAILQ_INIT(&state.pending_changes);
    state.debug_level = debug_level;
    state.transaction_active = false;

    /* Initialize YANG context */
    if (yang_init(&state) < 0) {
        debug_log(DEBUG_ERROR, "Failed to initialize YANG context");
        return 1;
    }

    /* Load configuration */
    if (config_load(&state) < 0) {
        debug_log(DEBUG_ERROR, "Failed to load configuration");
        yang_cleanup(&state);
        return 1;
    }

    /* Setup Unix domain socket */
    server_sock = setup_socket();
    if (server_sock < 0) {
        debug_log(DEBUG_ERROR, "Failed to setup socket");
        yang_cleanup(&state);
        return 1;
    }
    state.socket_fd = server_sock;

    debug_log(DEBUG_INFO, "netd server started");

    /* Main server loop */
    while (running) {
        fd_set readfds;
        struct timeval timeout;
        
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);
        
        /* Set timeout to allow checking running flag */
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int result = select(server_sock + 1, &readfds, NULL, NULL, &timeout);
        
        if (result < 0) {
            if (errno == EINTR) {
                /* Interrupted by signal, check running flag */
                continue;
            }
            debug_log(DEBUG_ERROR, "select() failed: %s", strerror(errno));
            break;
        }
        
        if (result == 0) {
            /* Timeout, check running flag and continue */
            continue;
        }
        
        debug_log(DEBUG_DEBUG, "select() returned %d", result);
        
        if (FD_ISSET(server_sock, &readfds)) {
            client_len = sizeof(client_addr);
            client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
            
            if (client_sock < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    debug_log(DEBUG_ERROR, "Failed to accept connection: %s", strerror(errno));
                }
                continue;
            }

            /* Set client socket to blocking mode */
            int flags = fcntl(client_sock, F_GETFL, 0);
            if (flags >= 0) {
                fcntl(client_sock, F_SETFL, flags & ~O_NONBLOCK);
            }

            debug_log(DEBUG_INFO, "Accepted client connection from %s", client_addr.sun_path);
            handle_client(client_sock);
        }
    }

    /* Cleanup */
    debug_log(DEBUG_INFO, "Shutting down netd server");

    close(server_sock);
    unlink(NETD_SOCKET_PATH);

    /* Save configuration */
    if (config_save(&state) < 0) {
        debug_log(DEBUG_ERROR, "Failed to save configuration");
    }

    /* Cleanup YANG context */
    yang_cleanup(&state);

    /* Close syslog */
    if (debug_level == DEBUG_NONE) {
        closelog();
    }

    debug_log(DEBUG_INFO, "netd server stopped");
    return 0;
} 