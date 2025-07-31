#include "net.h"
#include <sys/queue.h>
#include <unistd.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>



/**
 * Initialize readline for interactive mode
 */
void initialize_readline(void) {
    /* Set up readline completion */
    rl_attempted_completion_function = command_completion;
    
    /* Load history from file */
    read_history(".net_history");
}

/**
 * Command generator for tab completion
 */
char *command_generator(const char *text, int state) {
    static int list_index, len;
    const char *commands[] = {
        "set", "show", "delete", "commit", "save", "quit", "exit", "help"
    };
    
    if (!state) {
        list_index = 0;
        len = strlen(text);
    }
    
    while (list_index < (int)(sizeof(commands) / sizeof(commands[0]))) {
        const char *name = commands[list_index++];
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }
    
    return NULL;
}

/**
 * Command completion function for readline
 */
char **command_completion(const char *text, int start, int end) {
    (void)end; /* Suppress unused parameter warning */
    char **matches = NULL;
    
    if (start == 0) {
        matches = rl_completion_matches(text, command_generator);
    }
    
    return matches;
}

/**
 * Initialize YANG context for client
 */
int yang_init_client(net_client_t *client) {
    if (!client) {
        return -1;
    }
    
    /* Initialize libyang context with search paths */
    if (ly_ctx_new("./yang", 0, &client->yang_ctx) != LY_SUCCESS) {
        fprintf(stderr, "Failed to initialize YANG context\n");
        return -1;
    }
    
    if (!client->yang_ctx) {
        fprintf(stderr, "Failed to create YANG context\n");
        return -1;
    }
    
    return 0;
}

/**
 * Cleanup YANG context for client
 */
void yang_cleanup_client(net_client_t *client) {
    if (client && client->yang_ctx) {
        ly_ctx_destroy(client->yang_ctx);
        client->yang_ctx = NULL;
    }
}



/**
 * Execute a command
 */
int execute_command(net_client_t *client, const command_t *cmd) {
    if (!client || !cmd) {
        return -1;
    }
    
    /* Ensure we're connected to the server */
    if (!client->connected) {
        debug_log(DEBUG_INFO, "Connecting to server...");
        if (netconf_connect(client) < 0) {
            print_error("Failed to connect to server");
            return -1;
        }
        debug_log(DEBUG_INFO, "Connected to server");
    }
    
    /* Execute based on command type */
    switch (cmd->type) {
        case CMD_SET:
            return execute_set_command(client, cmd);
        case CMD_SHOW:
            return execute_show_command(client, cmd);
        case CMD_DELETE:
            return execute_delete_command(client, cmd);
        case CMD_COMMIT:
            return transaction_commit(client);
        case CMD_SAVE:
            return execute_save_command(client, cmd);
        default:
            print_error("Unknown command type: %d", cmd->type);
            return -1;
    }
}

/**
 * Execute a SET command
 */
int execute_set_command(net_client_t *client, const command_t *cmd) {
    char *response = NULL;
    char request[1024];
    
    if (!client || !cmd) {
        return -1;
    }
    
    if (cmd->arg_count < 3) {
        print_error("SET command requires object, name, and property");
        return -1;
    }
    
    /* Build a simple XML request for now */
    snprintf(request, sizeof(request),
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
        "  <edit-config>\n"
        "    <target><running/></target>\n"
        "    <config>\n"
        "      <set xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
        "        <object>%s</object>\n"
        "        <name>%s</name>\n"
        "        <property>%s</property>\n"
        "        <value>%s</value>\n"
        "      </set>\n"
        "    </config>\n"
        "  </edit-config>\n"
        "</rpc>\n",
        cmd->args[0], cmd->args[1], cmd->args[2], 
        cmd->arg_count > 3 ? cmd->args[3] : "");
    
    /* Send request to server */
    if (netconf_send_request(client, request, &response) == 0) {
        if (response) {
            printf("SET command successful\n");
            free(response);
        }
        return 0;
    } else {
        print_error("SET command failed");
        return -1;
    }
}

/**
 * Execute a SHOW command
 */
int execute_show_command(net_client_t *client, const command_t *cmd) {
    char *response = NULL;
    int ret = -1;
    
    if (!client || !cmd) {
        return -1;
    }
    
    if (cmd->arg_count < 1) {
        print_error("SHOW command requires an object type");
        return -1;
    }
    
    /* Determine what to show based on first argument */
    if (strcmp(cmd->args[0], "interface") == 0 || strcmp(cmd->args[0], "interfaces") == 0) {
        ret = netconf_get_interfaces(client, &response);
    } else if (strcmp(cmd->args[0], "vrf") == 0 || strcmp(cmd->args[0], "vrfs") == 0) {
        ret = netconf_get_vrfs(client, &response);
    } else if (strcmp(cmd->args[0], "route") == 0 || strcmp(cmd->args[0], "routes") == 0) {
        ret = netconf_get_routes(client, 0, AF_UNSPEC, &response);
    } else {
        print_error("Unknown object type: %s", cmd->args[0]);
        return -1;
    }
    
    if (ret == 0 && response) {
        /* Display formatted table instead of raw XML */
        if (strcmp(cmd->args[0], "interface") == 0 || strcmp(cmd->args[0], "interfaces") == 0) {
            /* Check if this is a group-filtered request */
            if (cmd->arg_count >= 3 && strcmp(cmd->args[1], "group") == 0) {
                print_interface_table_filtered(response, cmd->args[2]);
            } else {
            print_interface_table(response);
            }
        } else if (strcmp(cmd->args[0], "vrf") == 0 || strcmp(cmd->args[0], "vrfs") == 0) {
            print_vrf_table(response);
        } else if (strcmp(cmd->args[0], "route") == 0 || strcmp(cmd->args[0], "routes") == 0) {
            print_route_table(response);
        }
        free(response);
    } else {
        print_error("Failed to get %s information", cmd->args[0]);
    }
    
    return ret;
}

/**
 * Execute a DELETE command
 */
int execute_delete_command(net_client_t *client, const command_t *cmd) {
    char *response = NULL;
    char request[1024];
    
    if (!client || !cmd) {
        return -1;
    }
    
    if (cmd->arg_count < 2) {
        print_error("DELETE command requires object and name");
        return -1;
    }
    
    /* Build a simple XML request for now */
    snprintf(request, sizeof(request),
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
        "  <edit-config>\n"
        "    <target><running/></target>\n"
        "    <config>\n"
        "      <delete xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
        "        <object>%s</object>\n"
        "        <name>%s</name>\n"
        "        <property>%s</property>\n"
        "      </delete>\n"
        "    </config>\n"
        "  </edit-config>\n"
        "</rpc>\n",
        cmd->args[0], cmd->args[1], 
        cmd->arg_count > 2 ? cmd->args[2] : "");
    
    /* Send request to server */
    if (netconf_send_request(client, request, &response) == 0) {
        if (response) {
            printf("DELETE command successful\n");
            free(response);
        }
        return 0;
    } else {
        print_error("DELETE command failed");
        return -1;
    }
}

/**
 * Execute a SAVE command
 */
int execute_save_command(net_client_t *client, const command_t *cmd) {
    char *response = NULL;
    char request[1024];
    
    if (!client || !cmd) {
        return -1;
    }
    
    /* Use default filename if none provided */
    const char *filename = (cmd->arg_count >= 1) ? cmd->args[0] : "/etc/netd.conf";
    
    /* Build a simple XML request for now */
    snprintf(request, sizeof(request),
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
        "  <save xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
        "    <filename>%s</filename>\n"
        "  </save>\n"
        "</rpc>\n",
        filename);
    
    /* Send request to server */
    if (netconf_send_request(client, request, &response) == 0) {
        if (response) {
            printf("SAVE command successful\n");
            free(response);
        }
        return 0;
    } else {
        print_error("SAVE command failed");
        return -1;
    }
}

/**
 * Begin a transaction
 */
int transaction_begin(net_client_t *client) {
    if (!client) {
        return -1;
    }
    
    client->transaction.active = 1;
    client->transaction.command_count = 0;
    return 0;
}

/**
 * Commit a transaction
 */
int transaction_commit(net_client_t *client) {
    if (!client || !client->transaction.active) {
        return -1;
    }
    
    /* Process all pending commands */
    for (int i = 0; i < client->transaction.command_count; i++) {
        printf("Executing transaction command %d\n", i);
        /* TODO: Actually execute the command */
    }
    
    /* Clear the transaction */
    transaction_rollback(client);
    return 0;
}

/**
 * Rollback a transaction
 */
int transaction_rollback(net_client_t *client) {
    if (!client) {
        return -1;
    }
    
    client->transaction.command_count = 0;
    client->transaction.active = 0;
    return 0;
}

/**
 * Add a command to a transaction
 */
int transaction_add_command(net_client_t *client, const command_t *cmd) {
    if (!client || !client->transaction.active || !cmd) {
        return -1;
    }
    
    if (client->transaction.command_count >= 100) {
        return -1; /* Transaction full */
    }
    
    client->transaction.commands[client->transaction.command_count] = *cmd;
    client->transaction.command_count++;
    return 0;
}

/**
 * Interactive mode
 */
int interactive_mode(net_client_t *client) {
    char *line;
    
    printf("Network Configuration Tool\n");
    printf("Type 'help' for commands, 'quit' to exit\n");
    
    while ((line = readline("net> ")) != NULL) {
        if (strlen(line) > 0) {
            add_history(line);
            
            if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
                free(line);
                break;
            }
            
            if (strcmp(line, "help") == 0) {
                printf("Available commands:\n");
                printf("  set <object> <name> <property> <value>\n");
                printf("  show <object> [name]\n");
                printf("  delete <object> <name> [property]\n");
                printf("  commit\n");
                printf("  save <filename>\n");
                printf("  quit/exit\n");
            } else {
                command_t cmd;
                if (parse_command(line, &cmd) == 0) {
                    execute_command(client, &cmd);
                }
            }
        }
        
        free(line);
    }
    
    return 0;
} 