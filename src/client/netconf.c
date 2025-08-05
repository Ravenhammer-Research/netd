/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
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

#include "net.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Connect to netd server via Unix domain socket
 * @param client Client structure
 * @return 0 on success, -1 on failure
 */
int netconf_connect(net_client_t *client)
{
    struct sockaddr_un addr;

    if (!client) {
        return -1;
    }

    /* Create socket */
    client->socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client->socket_fd < 0) {
        print_error("Failed to create socket: %s", strerror(errno));
        return -1;
    }

    /* Set up address */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strlcpy(addr.sun_path, NETD_SOCKET_PATH, sizeof(addr.sun_path));

    /* Connect to server */
    debug_log(DEBUG_DEBUG, "Attempting to connect to server at %s", NETD_SOCKET_PATH);
    if (connect(client->socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        print_error("Failed to connect to netd server: %s", strerror(errno));
        close(client->socket_fd);
        client->socket_fd = -1;
        return -1;
    }

    debug_log(DEBUG_DEBUG, "Successfully connected to server");
    client->connected = true;
    return 0;
}

/**
 * Disconnect from netd server
 * @param client Client structure
 */
void netconf_disconnect(net_client_t *client)
{
    if (client && client->connected) {
        close(client->socket_fd);
        client->socket_fd = -1;
        client->connected = false;
    }
}

/**
 * Send NETCONF request to server
 * @param client Client structure
 * @param request Request XML string
 * @param response Response XML string (allocated)
 * @return 0 on success, -1 on failure
 */
int netconf_send_request(net_client_t *client, const char *request, char **response)
{
    ssize_t bytes_sent, bytes_received;
    char *response_buffer = NULL;
    size_t total_received = 0;
    size_t buffer_size = 8192; /* Start with 8KB */
    char temp_buffer[4096]; /* Temporary buffer for each recv() */

    debug_log(DEBUG_DEBUG, "netconf_send_request called");

    if (!client || !client->connected || !request || !response) {
        debug_log(DEBUG_ERROR, "Invalid parameters to netconf_send_request");
        return -1;
    }

    /* Validate request against YANG schema if YANG context is available */
    if (client->yang_ctx) {
        debug_log(DEBUG_DEBUG, "Validating NETCONF request against YANG schema");
        if (yang_validate_rpc_client(client, request) < 0) {
            debug_log(DEBUG_WARN, "NETCONF request failed YANG validation, but sending anyway");
            /* Don't fail the request, just log a warning for now */
        } else {
            debug_log(DEBUG_DEBUG, "NETCONF request validated successfully against YANG schema");
        }
    } else {
        debug_log(DEBUG_DEBUG, "No YANG context available, skipping request validation");
    }

    /* Send request */
    debug_log(DEBUG_DEBUG, "Sending request to server (%zu bytes)", strlen(request));
    debug_log(DEBUG_DEBUG, "Request content:\n%s", request);
    debug_log(DEBUG_DEBUG, "About to call send() on socket %d", client->socket_fd);
    
    /* Verify socket is still valid */
    if (client->socket_fd < 0) {
        debug_log(DEBUG_ERROR, "Invalid socket descriptor");
        client->connected = false;
        return -1;
    }
    
    bytes_sent = send(client->socket_fd, request, strlen(request), 0);
    if (bytes_sent < 0) {
        print_error("Failed to send request: %s", strerror(errno));
        client->connected = false;
        return -1;
    }
    debug_log(DEBUG_DEBUG, "Sent %zd bytes to server", bytes_sent);

    /* Allocate initial response buffer */
    response_buffer = malloc(buffer_size);
    if (!response_buffer) {
        print_error("Failed to allocate response buffer");
        return -1;
    }

    /* Receive response in chunks */
    debug_log(DEBUG_DEBUG, "Waiting for response from server...");
    
    /* Set socket to non-blocking */
    int flags = fcntl(client->socket_fd, F_GETFL, 0);
    fcntl(client->socket_fd, F_SETFL, flags | O_NONBLOCK);
    
    int retry_count = 0;
    const int max_retries = 10;
    
    while (1) {
        bytes_received = recv(client->socket_fd, temp_buffer, sizeof(temp_buffer), 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                /* Server closed connection, this is normal */
                debug_log(DEBUG_DEBUG, "Server closed connection after response");
                break;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                /* No data available, retry a few times before giving up */
                retry_count++;
                if (retry_count >= max_retries) {
                    debug_log(DEBUG_DEBUG, "No more data available after %d retries", max_retries);
                    break;
                }
                usleep(10000); /* 10ms delay */
                continue;
            } else {
                print_error("Failed to receive response: %s", strerror(errno));
                free(response_buffer);
                client->connected = false;
                return -1;
            }
        }

        /* Check if we need more buffer space */
        if (total_received + bytes_received >= buffer_size) {
            buffer_size *= 2;
            char *new_buffer = realloc(response_buffer, buffer_size);
            if (!new_buffer) {
                print_error("Failed to reallocate response buffer");
                free(response_buffer);
                return -1;
            }
            response_buffer = new_buffer;
        }

        /* Copy received data to response buffer */
        memcpy(response_buffer + total_received, temp_buffer, bytes_received);
        total_received += bytes_received;
        
        debug_log(DEBUG_DEBUG, "Received %zd bytes from server (total: %zu)", bytes_received, total_received);

        /* Continue reading until no more data */
        if (bytes_received == 0) {
            debug_log(DEBUG_DEBUG, "No more data from server");
            break;
        }
    }

    /* Null-terminate the response */
    response_buffer[total_received] = '\0';

    /* Allocate and copy response */
    *response = strdup(response_buffer);
    free(response_buffer);
    
    if (!*response) {
        print_error("Failed to allocate final response buffer");
        return -1;
    }

    /* Validate response against YANG schema if YANG context is available */
    if (client->yang_ctx) {
        debug_log(DEBUG_DEBUG, "Validating NETCONF response against YANG schema");
        if (yang_validate_response_client(client, *response) < 0) {
            debug_log(DEBUG_WARN, "NETCONF response failed YANG validation, but processing anyway");
            /* Don't fail the request, just log a warning for now */
        } else {
            debug_log(DEBUG_DEBUG, "NETCONF response validated successfully against YANG schema");
        }
    } else {
        debug_log(DEBUG_DEBUG, "No YANG context available, skipping response validation");
    }

    return 0;
}

/**
 * Get interfaces from server
 * @param client Client structure
 * @param response Response XML string (allocated)
 * @return 0 on success, -1 on failure
 */
int netconf_get_interfaces(net_client_t *client, char **response)
{
    const char *request = 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
        "  <get>\n"
        "    <filter type=\"subtree\">\n"
        "      <interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\"/>\n"
        "    </filter>\n"
        "  </get>\n"
        "</rpc>\n";

    return netconf_send_request(client, request, response);
}

/**
 * Get VRFs from server
 * @param client Client structure
 * @param response Response XML string (allocated)
 * @return 0 on success, -1 on failure
 */
int netconf_get_vrfs(net_client_t *client, char **response)
{
    const char *request = 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<rpc message-id=\"2\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
        "  <get>\n"
        "    <filter type=\"subtree\">\n"
        "      <network-instances xmlns=\"urn:ietf:params:xml:ns:yang:ietf-network-instance\"/>\n"
        "    </filter>\n"
        "  </get>\n"
        "</rpc>\n";

    return netconf_send_request(client, request, response);
}

/**
 * Get routes from server
 * @param client Client structure
 * @param fib FIB number
 * @param family Address family
 * @param response Response XML string (allocated)
 * @return 0 on success, -1 on failure
 */
int netconf_get_routes(net_client_t *client, uint32_t fib, int family, char **response)
{
    char request[1024];
    (void)family; /* Suppress unused parameter warning */

    if (fib == 0) {
        /* Default FIB - get all network instances */
        snprintf(request, sizeof(request),
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<rpc message-id=\"3\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
            "  <get>\n"
            "    <filter type=\"subtree\">\n"
            "      <network-instances xmlns=\"urn:ietf:params:xml:ns:yang:ietf-network-instance\"/>\n"
            "    </filter>\n"
            "  </get>\n"
            "</rpc>\n");
    } else {
        /* Specific FIB - include network instance name */
        snprintf(request, sizeof(request),
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<rpc message-id=\"3\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
            "  <get>\n"
            "    <filter type=\"subtree\">\n"
            "      <network-instances xmlns=\"urn:ietf:params:xml:ns:yang:ietf-network-instance\">\n"
            "        <network-instance>\n"
            "          <name>vrf%u</name>\n"
            "        </network-instance>\n"
            "      </network-instances>\n"
            "    </filter>\n"
            "  </get>\n"
            "</rpc>\n", fib);
    }

    return netconf_send_request(client, request, response);
}

/**
 * Get interface groups from server
 * @param client Client structure
 * @param response Response XML string (allocated)
 * @return 0 on success, -1 on failure
 */
int netconf_get_interface_groups(net_client_t *client, char **response)
{
    const char *request = 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<rpc message-id=\"4\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
        "  <get>\n"
        "    <filter type=\"subtree\">\n"
        "      <interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\"/>\n"
        "    </filter>\n"
        "  </get>\n"
        "</rpc>\n";

    return netconf_send_request(client, request, response);
}