#include "net.h"
#include <sys/socket.h>
#include <sys/queue.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
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
    
    /* Debug logging to see command structure */
    debug_log(DEBUG_DEBUG, "SET command object: %d, arg_count: %d", cmd->object, cmd->arg_count);
    for (int i = 0; i < cmd->arg_count && i < 10; i++) {
        debug_log(DEBUG_DEBUG, "args[%d]: '%s'", i, cmd->args[i]);
    }
    
    /* Handle route setting commands */
    if (cmd->arg_count >= 10 && strcmp(cmd->args[0], "vrf") == 0 && strcmp(cmd->args[1], "id") == 0 &&
        strcmp(cmd->args[3], "protocol") == 0 && strcmp(cmd->args[4], "static") == 0 &&
        (strcmp(cmd->args[5], "inet") == 0 || strcmp(cmd->args[5], "inet6") == 0) && 
        strcmp(cmd->args[6], "network") == 0 && strcmp(cmd->args[8], "gateway") == 0) {
        /* Format: set vrf id <fib> protocol static inet network <network> gateway <gateway> [iface <interface>] */
        uint32_t fib = atoi(cmd->args[2]);  /* VRF ID/FIB */
        const char *network = cmd->args[7];  /* Network (e.g., 10.0.0.0/32) */
        const char *gateway = cmd->args[9];  /* Gateway (e.g., 192.168.32.129) */
        const char *interface = NULL;        /* Optional interface */
        
        /* Check for optional interface parameter */
        if (cmd->arg_count >= 12 && strcmp(cmd->args[10], "iface") == 0) {
            interface = cmd->args[11];
        }
        
        debug_log(DEBUG_INFO, "Setting route: FIB=%u, network=%s, gateway=%s, interface=%s", 
                  fib, network, gateway, interface ? interface : "none");
        
        /* Build route setting XML request */
        if (interface) {
            snprintf(request, sizeof(request),
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                "  <edit-config>\n"
                "    <target><running/></target>\n"
                "    <config>\n"
                "      <lib xmlns=\"http://frrouting.org/yang/vrf\">\n"
                "        <vrf>\n"
                "          <name>%s</name>\n"
                "          <route xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
                "            <destination>%s</destination>\n"
                "            <gateway>%s</gateway>\n"
                "            <interface>%s</interface>\n"
                "            <type>static</type>\n"
                "            <enabled>true</enabled>\n"
                "          </route>\n"
                "        </vrf>\n"
                "      </lib>\n"
                "    </config>\n"
                "  </edit-config>\n"
                "</rpc>\n",
                fib == 0 ? "default" : cmd->args[2], network, gateway, interface);
        } else {
            snprintf(request, sizeof(request),
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                "  <edit-config>\n"
                "    <target><running/></target>\n"
                "    <config>\n"
                "      <lib xmlns=\"http://frrouting.org/yang/vrf\">\n"
                "        <vrf>\n"
                "          <name>%s</name>\n"
                "          <route xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
                "            <destination>%s</destination>\n"
                "            <gateway>%s</gateway>\n"
                "            <type>static</type>\n"
                "            <enabled>true</enabled>\n"
                "          </route>\n"
                "        </vrf>\n"
                "      </lib>\n"
                "    </config>\n"
                "  </edit-config>\n"
                "</rpc>\n",
                fib == 0 ? "default" : cmd->args[2], network, gateway);
        }
    } else {
        /* Generic set command for other objects */
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
    }
    
    /* Send request to server */
    if (netconf_send_request(client, request, &response) == 0) {
        if (response) {
            printf("SET command successful\n");
            free(response);
            /* Mark that we have an active transaction on the server */
            client->transaction.active = true;
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
    
    /* Debug logging to see command structure */
    debug_log(DEBUG_DEBUG, "Command object: %d, arg_count: %d", cmd->object, cmd->arg_count);
    for (int i = 0; i < cmd->arg_count && i < 5; i++) {
        debug_log(DEBUG_DEBUG, "args[%d]: '%s'", i, cmd->args[i]);
    }
    
    /* Special handling for VRF ID route command - check pattern regardless of object type */
    if (cmd->arg_count >= 5 && 
        strcmp(cmd->args[0], "vrf") == 0 && strcmp(cmd->args[1], "id") == 0 &&
        strcmp(cmd->args[3], "protocol") == 0 && strcmp(cmd->args[4], "static") == 0) {
        debug_log(DEBUG_INFO, "Handling VRF ID route command");
        /* Extract FIB ID and get routes for that FIB */
        uint32_t fib_id = atoi(cmd->args[2]);
        ret = netconf_get_routes(client, fib_id, AF_UNSPEC, &response);
    } else {
        /* Check for VRF FIBID route command */
        if (cmd->object == OBJ_ROUTE && cmd->arg_count >= 5 && 
            strcmp(cmd->args[0], "vrf") == 0 && strcmp(cmd->args[1], "id") == 0 &&
            strcmp(cmd->args[3], "protocol") == 0 && strcmp(cmd->args[4], "static") == 0) {
            debug_log(DEBUG_INFO, "Handling VRF FIBID route command");
            /* Extract FIB ID and get routes for that FIB */
            uint32_t fib_id = atoi(cmd->args[2]);
            ret = netconf_get_routes(client, fib_id, AF_UNSPEC, &response);
        } else if (strcmp(cmd->args[0], "interface") == 0 || strcmp(cmd->args[0], "interfaces") == 0) {
            ret = netconf_get_interfaces(client, &response);
        } else if (strcmp(cmd->args[0], "vrf") == 0 || strcmp(cmd->args[0], "vrfs") == 0) {
            ret = netconf_get_vrfs(client, &response);
        } else if (strcmp(cmd->args[0], "route") == 0 || strcmp(cmd->args[0], "routes") == 0) {
            /* Check if this is a protocol-specific route request */
            if (cmd->arg_count >= 3 && strcmp(cmd->args[1], "protocol") == 0) {
                /* For now, just get all routes - protocol filtering can be added later */
                ret = netconf_get_routes(client, 0, AF_UNSPEC, &response);
            } else {
                ret = netconf_get_routes(client, 0, AF_UNSPEC, &response);
            }
        } else if (strcmp(cmd->args[0], "group") == 0) {
            ret = netconf_get_interface_groups(client, &response);
        } else {
            print_error("Unknown object type: %s", cmd->args[0]);
            return -1;
        }
    }
    
            if (ret == 0 && response) {
            /* Display formatted table instead of raw XML */
            debug_log(DEBUG_DEBUG, "Command args[0]='%s', args[1]='%s', arg_count=%d", 
                     cmd->args[0], cmd->args[1], cmd->arg_count);
            if (strcmp(cmd->args[0], "interface") == 0 || strcmp(cmd->args[0], "interfaces") == 0) {
                /* Check if this is a group-filtered request */
                if (cmd->arg_count >= 3 && strcmp(cmd->args[1], "group") == 0) {
                    /* Check if this is the wlan group */
                    if (strcmp(cmd->args[2], "wlan") == 0) {
                        debug_log(DEBUG_DEBUG, "Raw XML response for wlan group:\n%s", response);
                        print_wlan_interface_table(response);
                    } else {
                        print_interface_table_filtered(response, cmd->args[2]);
                    }
                } else {
                    print_interface_table(response);
                }
        } else if (cmd->arg_count >= 5 && 
                   strcmp(cmd->args[0], "vrf") == 0 && strcmp(cmd->args[1], "id") == 0 &&
                   strcmp(cmd->args[3], "protocol") == 0 && strcmp(cmd->args[4], "static") == 0) {
            debug_log(DEBUG_DEBUG, "Displaying VRF ID route table");
            print_route_table(response);
        } else if (cmd->object == OBJ_ROUTE && cmd->arg_count >= 5 && 
                   strcmp(cmd->args[0], "vrf") == 0 && strcmp(cmd->args[1], "id") == 0 &&
                   strcmp(cmd->args[3], "protocol") == 0 && strcmp(cmd->args[4], "static") == 0) {
            debug_log(DEBUG_DEBUG, "Displaying VRF FIBID route table");
            print_route_table(response);
        } else if (strcmp(cmd->args[0], "vrf") == 0 || strcmp(cmd->args[0], "vrfs") == 0) {
            debug_log(DEBUG_DEBUG, "Displaying VRF table");
            print_vrf_table(response);
        } else if (strcmp(cmd->args[0], "route") == 0 || strcmp(cmd->args[0], "routes") == 0) {
            debug_log(DEBUG_DEBUG, "Displaying route table");
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
    
    /* Debug logging to see command structure */
    debug_log(DEBUG_DEBUG, "DELETE command object: %d, arg_count: %d", cmd->object, cmd->arg_count);
    for (int i = 0; i < cmd->arg_count && i < 12; i++) {
        debug_log(DEBUG_DEBUG, "args[%d]: '%s'", i, cmd->args[i]);
    }
    
    /* Handle route deletion commands */
    if (cmd->arg_count >= 10 && strcmp(cmd->args[0], "vrf") == 0 && strcmp(cmd->args[1], "id") == 0 &&
        strcmp(cmd->args[3], "protocol") == 0 && strcmp(cmd->args[4], "static") == 0 &&
        (strcmp(cmd->args[5], "inet") == 0 || strcmp(cmd->args[5], "inet6") == 0) && 
        strcmp(cmd->args[6], "network") == 0 && strcmp(cmd->args[8], "gateway") == 0) {
        /* Format: delete vrf id <fib> protocol static inet network <network> gateway <gateway> */
        uint32_t fib = atoi(cmd->args[2]);  /* VRF ID/FIB */
        const char *network = cmd->args[7];  /* Network (e.g., 10.0.0.0/32) */
        const char *gateway = cmd->args[9];  /* Gateway (e.g., 192.168.32.129) */
        
        debug_log(DEBUG_INFO, "Deleting route: FIB=%u, network=%s, gateway=%s", fib, network, gateway);
        
        /* Build route deletion XML request */
        snprintf(request, sizeof(request),
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
            "  <edit-config>\n"
            "    <target><running/></target>\n"
            "    <config>\n"
            "      <lib xmlns=\"http://frrouting.org/yang/vrf\">\n"
            "        <vrf>\n"
            "          <name>%s</name>\n"
            "          <route xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
            "            <destination>%s</destination>\n"
            "            <gateway>%s</gateway>\n"
            "            <type>static</type>\n"
            "            <enabled>false</enabled>\n"
            "          </route>\n"
            "        </vrf>\n"
            "      </lib>\n"
            "    </config>\n"
            "  </edit-config>\n"
            "</rpc>\n",
            fib == 0 ? "default" : cmd->args[2], network, gateway);
    } else {
        /* Generic delete command for other objects */
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
    }
    
    /* Send request to server */
    if (netconf_send_request(client, request, &response) == 0) {
        if (response) {
            printf("DELETE command successful\n");
            free(response);
            /* Mark that we have an active transaction on the server */
            client->transaction.active = true;
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
    char *response = NULL;
    char request[1024];
    
    if (!client || !client->transaction.active) {
        print_error("No active transaction to commit");
        return -1;
    }
    
    /* Build commit XML request */
    snprintf(request, sizeof(request),
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
        "  <commit xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
        "  </commit>\n"
        "</rpc>\n");
    
    /* Send request to server */
    if (netconf_send_request(client, request, &response) == 0) {
        if (response) {
            /* Check if the response contains an error */
            if (strstr(response, "<rpc-error>") != NULL) {
                print_error("COMMIT command failed - server error");
                free(response);
                return -1;
            } else {
                printf("COMMIT command successful\n");
                free(response);
                /* Clear the transaction */
                transaction_rollback(client);
                return 0;
            }
        }
        return 0;
    } else {
        print_error("COMMIT command failed");
        return -1;
    }
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