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
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

#include <net.h>
#include <netconf.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * Connect to the NETCONF server
 */
int netconf_connect(net_client_t *client) {
    struct sockaddr_un addr;
    
    if (!client) {
        return -1;
    }
    
    /* Create socket */
    client->socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client->socket_fd < 0) {
        fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
        return -1;
    }
    
    /* Set socket address */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, NETCONF_SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    /* Connect to server */
    if (connect(client->socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "Failed to connect to server: %s\n", strerror(errno));
        close(client->socket_fd);
        client->socket_fd = -1;
        return -1;
    }
    
    /* Set socket to non-blocking mode */
    int flags = fcntl(client->socket_fd, F_GETFL, 0);
    fcntl(client->socket_fd, F_SETFL, flags | O_NONBLOCK);
    
    client->connected = true;
    debug_log(INFO, "Connected to NETCONF server");
    
    return 0;
}

/**
 * Disconnect from the NETCONF server
 */
void netconf_disconnect(net_client_t *client) {
    if (!client) {
        return;
    }
    
    if (client->socket_fd >= 0) {
        close(client->socket_fd);
        client->socket_fd = -1;
    }
    
    client->connected = false;
    debug_log(INFO, "Disconnected from NETCONF server");
}

/**
 * Send a NETCONF request and receive response
 */
int netconf_send_request(net_client_t *client, const char *request, char **response) {
    ssize_t sent, received;
    char *buffer = malloc(128 * 1024 * 1024); /* 128MB - same as server NETCONF_RESPONSE_BUFFER_SIZE */
    char *full_response = NULL;
    size_t response_size = 0;
    
    if (!client || !request || !response) {
        free(buffer);
        return -1;
    }
    
    if (!buffer) {
        fprintf(stderr, "Failed to allocate receive buffer\n");
        return -1;
    }
    
    if (!client->connected) {
        fprintf(stderr, "Not connected to server\n");
        free(buffer);
        return -1;
    }
    
    /* Send request */
    sent = send(client->socket_fd, request, strlen(request), 0);
    if (sent < 0) {
        fprintf(stderr, "Failed to send request: %s\n", strerror(errno));
        free(buffer);
        return -1;
    }
    
    if (sent != (ssize_t)strlen(request)) {
        fprintf(stderr, "Incomplete send: %zd of %zu bytes\n", sent, strlen(request));
        free(buffer);
        return -1;
    }
    
    debug_log(DEBUG, "Sent request (%zd bytes): %s", sent, request);
    
    /* Validate request against YANG schema */
    if (yang_validate_rpc_client(client, request) < 0) {
        debug_log(ERROR, "Request failed YANG validation");
        free(buffer);
        return -1;
    }
    
    /* Receive response */
    while (1) {
        received = recv(client->socket_fd, buffer, 128 * 1024 * 1024 - 1, 0);
        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                /* Non-blocking socket, no data available */
                usleep(10000); /* 10ms */
                continue;
            }
            fprintf(stderr, "Failed to receive response: %s\n", strerror(errno));
            free(buffer);
            return -1;
        }
        
        if (received == 0) {
            /* Connection closed by server */
            break;
        }
        
        /* Null-terminate received data */
        buffer[received] = '\0';
        
        /* Append to full response */
        char *new_response = realloc(full_response, response_size + received + 1);
        if (!new_response) {
            fprintf(stderr, "Failed to allocate memory for response\n");
            free(full_response);
            free(buffer);
            return -1;
        }
        
        full_response = new_response;
        strcpy(full_response + response_size, buffer);
        response_size += received;
        
        /* Check if we have a complete response */
        if (strstr(full_response, "</rpc-reply>") || strstr(full_response, "</hello>")) {
            break;
        }
    }
    
    free(buffer);
    
    if (!full_response) {
        fprintf(stderr, "No response received\n");
        return -1;
    }
    
    *response = full_response;
    debug_log(DEBUG, "Received response (%zu bytes)", response_size);
    
    /* Validate response against YANG schema */
    if (yang_validate_response_client(client, full_response) < 0) {
        debug_log(WARN, "Response validation failed, but continuing");
    }
    
    return 0;
} 