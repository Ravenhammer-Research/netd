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
#include <string.h>

/**
 * Initialize client
 * @param client Client structure
 * @param interactive Whether running in interactive mode
 * @return 0 on success, -1 on failure
 */
int client_init(net_client_t *client, bool interactive)
{
    if (!client) {
        return -1;
    }

    /* Initialize client structure */
    memset(client, 0, sizeof(*client));
    client->socket_fd = -1;
    client->connected = false;
    client->completion_count = 0;
    client->transaction.active = false;
    client->transaction.command_count = 0;

    /* Initialize YANG context */
    if (yang_init_client(client) < 0) {
        print_error("Failed to initialize YANG context");
        return -1;
    }

    /* Connect to server */
    if (netconf_connect(client) < 0) {
        print_error("Failed to connect to netd server");
        yang_cleanup_client(client);
        return -1;
    }

    /* Initialize readline if interactive */
    if (interactive) {
        initialize_readline();
    }

    return 0;
}

/**
 * Cleanup client
 * @param client Client structure
 */
void client_cleanup(net_client_t *client)
{
    if (client) {
        /* Rollback any active transaction */
        if (client->transaction.active) {
            transaction_rollback(client);
        }
        
        netconf_disconnect(client);
        yang_cleanup_client(client);
    }
} 