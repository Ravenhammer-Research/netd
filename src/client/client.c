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

#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

/**
 * Initialize client
 * @param client Client structure
 * @param interactive Whether running in interactive mode
 * @return 0 on success, -1 on failure
 */
int client_init(net_client_t *client, bool interactive) {
  if (!client) {
    return -1;
  }

  debug_log(DEBUG_INFO, "Initializing client (interactive: %s)",
            interactive ? "yes" : "no");

  /* Initialize client structure */
  memset(client, 0, sizeof(*client));
  client->socket_fd = -1;
  client->connected = false;
  client->transaction.active = false;
  client->transaction.command_count = 0;

  /* Initialize YANG context */
  debug_log(DEBUG_DEBUG, "Initializing YANG context");
  if (yang_init_client(client) < 0) {
    print_error("Failed to initialize YANG context");
    return -1;
  }

  /* Connect to server */
  debug_log(DEBUG_DEBUG, "Connecting to netd server");
  if (netconf_connect(client) < 0) {
    print_error("Failed to connect to netd server");
    yang_cleanup_client(client);
    return -1;
  }

  /* Initialize readline if interactive */
  if (interactive) {
    debug_log(DEBUG_DEBUG, "Initializing readline for interactive mode");
    initialize_readline();
  }

  debug_log(DEBUG_INFO, "Client initialization completed successfully");
  return 0;
}

/**
 * Cleanup client
 * @param client Client structure
 */
void client_cleanup(net_client_t *client) {
  if (client) {
    debug_log(DEBUG_DEBUG, "Cleaning up client");

    /* Rollback any active transaction */
    if (client->transaction.active) {
      debug_log(DEBUG_DEBUG, "Rolling back active transaction");
      transaction_rollback();
    }

    debug_log(DEBUG_DEBUG, "Disconnecting from server");
    netconf_disconnect(client);

    debug_log(DEBUG_DEBUG, "Cleaning up YANG context");
    yang_cleanup_client(client);

    debug_log(DEBUG_INFO, "Client cleanup completed");
  }
}

/**
 * Initialize readline
 */
void initialize_readline(void) {
    /* Set readline completion function */
    rl_completion_entry_function = command_completion;
    
    /* Set readline generator function */
    rl_attempted_completion_function = command_generator;
    
    /* Set readline prompt */
    rl_prompt = "net> ";
}

/**
 * Command completion function for readline
 */
char *command_completion(const char *text, int state) {
    static int list_index, len;
    static const char *commands[] = {
        "show", "set", "delete", "commit", "save", "quit", "help",
        "interfaces", "vrf", "routes", "bridge", "vlan", "ethernet",
        "lagg", "tap", "gif", "epair", "vxlan", "loopback", "wlan",
        NULL
    };
    char *name;
    
    /* If this is a new word to complete, initialize */
    if (!state) {
        list_index = 0;
        len = strlen(text);
    }
    
    /* Return next name that matches */
    while ((name = (char *)commands[list_index++])) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }
    
    return NULL;
}

/**
 * Command generator function for readline
 */
char **command_generator(const char *text, int start, int end) {
    char **matches = NULL;
    
    /* Don't complete if we're not at the beginning of the line */
    if (start == 0) {
        matches = rl_completion_matches(text, command_completion);
    }
    
    /* end parameter is not used in this implementation but required by readline API */
    (void)end; /* Suppress unused parameter warning */
    
    return matches;
}

/**
 * Interactive mode - main command loop
 */
int interactive_mode(net_client_t *client) {
    char *line;
    command_t cmd;
    int ret = 0;
    
    printf("Welcome to net client interactive mode\n");
    printf("Type 'help' for available commands, 'quit' to exit\n\n");
    
    /* Initialize readline */
    initialize_readline();
    
    while (1) {
        line = readline(rl_prompt);
        if (!line) {
            break;
        }
        
        /* Skip empty lines */
        if (strlen(line) == 0) {
            free(line);
            continue;
        }
        
        /* Add to history */
        add_history(line);
        
        /* Parse command */
        if (parse_command(line, &cmd) < 0) {
            print_error("Invalid command syntax");
            free(line);
            continue;
        }
        
        /* Handle quit command */
        if (cmd.type == CMD_QUIT) {
            free(line);
            break;
        }
        
        /* Execute command */
        ret = execute_command(client, &cmd);
        if (ret < 0) {
            printf("Command failed\n");
        }
        
        free(line);
    }
    
    printf("Goodbye!\n");
    return ret;
}