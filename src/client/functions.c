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
    if (cmd->arg_count >= 6 && strcmp(cmd->args[0], "vrf") == 0 && strcmp(cmd->args[1], "id") == 0 &&
        strcmp(cmd->args[3], "protocol") == 0 && strcmp(cmd->args[4], "static") == 0 &&
        (strcmp(cmd->args[5], "inet") == 0 || strcmp(cmd->args[5], "inet6") == 0)) {
        
        uint32_t fib = atoi(cmd->args[2]);  /* VRF ID/FIB */
        const char *family = cmd->args[5];   /* Address family */
        const char *route_type = NULL;       /* host or network */
        const char *destination = NULL;      /* host address or network */
        const char *next_hop_type = NULL;    /* gateway or iface */
        const char *next_hop = NULL;         /* gateway address or interface name */
        
        /* Parse the flexible route syntax */
        if (cmd->arg_count >= 8) {
            if (strcmp(cmd->args[6], "host") == 0) {
                route_type = "host";
                destination = cmd->args[7];
            } else if (strcmp(cmd->args[6], "network") == 0) {
                route_type = "network";
                destination = cmd->args[7];
            } else {
                /* Legacy format: network <network> gateway <gateway> */
                route_type = "network";
                destination = cmd->args[6];
                if (cmd->arg_count >= 10 && strcmp(cmd->args[8], "gateway") == 0) {
                    next_hop_type = "gateway";
                    next_hop = cmd->args[9];
                }
            }
            
            /* Check for gateway or iface */
            if (cmd->arg_count >= 10) {
                if (strcmp(cmd->args[8], "gateway") == 0) {
                    next_hop_type = "gateway";
                    next_hop = cmd->args[9];
                    
                    /* Check for optional interface constraint after gateway */
                    if (cmd->arg_count >= 12 && strcmp(cmd->args[10], "iface") == 0) {
                        const char *interface = cmd->args[11];
                        debug_log(DEBUG_INFO, "Route with gateway %s and interface constraint %s", next_hop, interface);
                    }
                } else if (strcmp(cmd->args[8], "iface") == 0) {
                    next_hop_type = "iface";
                    next_hop = cmd->args[9];
                }
            }
        }
        
        debug_log(DEBUG_INFO, "Setting route: FIB=%u, family=%s, type=%s, dest=%s, next_hop_type=%s, next_hop=%s", 
                  fib, family, route_type ? route_type : "unknown", destination ? destination : "none", 
                  next_hop_type ? next_hop_type : "none", next_hop ? next_hop : "none");
        
        /* Build route setting XML request */
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
            fib == 0 ? "default" : cmd->args[2], 
            destination ? destination : "",
            next_hop ? next_hop : "");
    } else if (cmd->object == OBJ_VRF) {
        /* Handle different VRF command formats */
        if (cmd->arg_count == 5 && strcmp(cmd->args[0], "vrf") == 0 && strcmp(cmd->args[1], "id") == 0 && strcmp(cmd->args[3], "name") == 0) {
            /* Format: set vrf id <fib> name <name> */
            const char *vrf_id = cmd->args[2];  /* FIB number */
            const char *vrf_name = cmd->args[4]; /* VRF name */
            
            debug_log(DEBUG_INFO, "Setting VRF: id=%s, name=%s", vrf_id, vrf_name);
            
            /* VRF name configuration */
            debug_log(DEBUG_INFO, "Generating VRF name XML: vrf_name='%s', vrf_id='%s'", vrf_name, vrf_id);
            snprintf(request, sizeof(request),
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                "  <edit-config>\n"
                "    <target><running/></target>\n"
                "    <config>\n"
                "      <lib xmlns=\"http://frrouting.org/yang/vrf\">\n"
                "        <vrf>\n"
                "          <name>%s</name>\n"
                "          <table>%s</table>\n"
                "        </vrf>\n"
                "      </lib>\n"
                "    </config>\n"
                "  </edit-config>\n"
                "</rpc>\n",
                vrf_name, vrf_id);  // vrf_name="servers", vrf_id="18"
            debug_log(DEBUG_INFO, "Generated VRF XML request: %s", request);
        } else {
            /* Legacy format: set vrf <id> <property> <value> */
            const char *vrf_id = cmd->args[0];
            const char *property = cmd->args[1];
            const char *value = cmd->args[2];
            
            debug_log(DEBUG_INFO, "Setting VRF (legacy): id=%s, property=%s, value=%s", vrf_id, property, value);
            
            if (strcmp(property, "name") == 0) {
            /* VRF name configuration */
            debug_log(DEBUG_INFO, "Generating VRF name XML: value='%s', vrf_id='%s'", value, vrf_id);
            snprintf(request, sizeof(request),
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                "  <edit-config>\n"
                "    <target><running/></target>\n"
                "    <config>\n"
                "      <lib xmlns=\"http://frrouting.org/yang/vrf\">\n"
                "        <vrf>\n"
                "          <name>%s</name>\n"
                "          <table>%s</table>\n"
                "        </vrf>\n"
                "      </lib>\n"
                "    </config>\n"
                "  </edit-config>\n"
                "</rpc>\n",
                value, vrf_id);  // value="servers" (name), vrf_id="18" (table)
            debug_log(DEBUG_INFO, "Generated VRF XML request: %s", request);
        } else {
            /* Legacy VRF table assignment */
            snprintf(request, sizeof(request),
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                "  <edit-config>\n"
                "    <target><running/></target>\n"
                "    <config>\n"
                "      <lib xmlns=\"http://frrouting.org/yang/vrf\">\n"
                "        <vrf>\n"
                "          <name>%s</name>\n"
                "          <table>%s</table>\n"
                "        </vrf>\n"
                "      </lib>\n"
                "    </config>\n"
                "  </edit-config>\n"
                "</rpc>\n",
                vrf_id, value);
            }
        }
    } else {
        /* Handle interface commands - parser has already extracted the components */
        debug_log(DEBUG_INFO, "Processing interface command: object=%d, arg_count=%d", cmd->object, cmd->arg_count);
        for (int i = 0; i < cmd->arg_count && i < 10; i++) {
            debug_log(DEBUG_INFO, "args[%d]: '%s'", i, cmd->args[i]);
        }
        
        if (cmd->object == OBJ_INTERFACE && cmd->arg_count >= 4) {
            /* Handle the case where args are: [interface, type, epair, name, epair127, peer, b, vrf, id, 18] */
            const char *interface_type = NULL;
            const char *interface_name = NULL;
            const char *property = NULL;
            const char *value = NULL;
            
            if (cmd->arg_count >= 10 && strcmp(cmd->args[1], "type") == 0 && strcmp(cmd->args[3], "name") == 0) {
                /* Special case: set interface type <type> name <name> peer <peer> vrf id <number> */
                interface_type = cmd->args[2];  /* epair */
                interface_name = cmd->args[4];  /* epair127 */
                property = cmd->args[5];        /* peer */
                value = cmd->args[6];          /* b */
            } else {
                /* Standard case: args[0]=type, args[1]=name, args[2]=property, args[3]=value */
                interface_type = cmd->args[0];
                interface_name = cmd->args[1];
                property = cmd->args[2];
                value = cmd->args[3];
            }
            
            /* Validate type-specific properties */
            if (strcmp(property, "member") == 0 && strcmp(interface_type, "bridge") != 0) {
                fprintf(stderr, "Error: member operations only valid for bridge interfaces\n");
                return -1;
            }
            if (strcmp(property, "laggproto") == 0 && strcmp(interface_type, "lagg") != 0) {
                fprintf(stderr, "Error: laggproto operations only valid for lagg interfaces\n");
                return -1;
            }
            if (strcmp(property, "laggport") == 0 && strcmp(interface_type, "lagg") != 0) {
                fprintf(stderr, "Error: laggport operations only valid for lagg interfaces\n");
                return -1;
            }
            if (strcmp(property, "peer") == 0 && strcmp(interface_type, "epair") != 0) {
                fprintf(stderr, "Error: peer operations only valid for epair interfaces\n");
                return -1;
            }
            if (strcmp(property, "vlandev") == 0 && strcmp(interface_type, "vlan") != 0) {
                fprintf(stderr, "Error: vlandev operations only valid for vlan interfaces\n");
                return -1;
            }
            if (strcmp(property, "vlanproto") == 0 && strcmp(interface_type, "vlan") != 0) {
                fprintf(stderr, "Error: vlanproto operations only valid for vlan interfaces\n");
                return -1;
            }
            if (strcmp(property, "tunnel") == 0 && strcmp(interface_type, "gif") != 0 && strcmp(interface_type, "vxlan") != 0) {
                fprintf(stderr, "Error: tunnel operations only valid for gif and vxlan interfaces\n");
                return -1;
            }
            if (strcmp(property, "tunnelvrf") == 0 && strcmp(interface_type, "gif") != 0) {
                fprintf(stderr, "Error: tunnelvrf operations only valid for gif interfaces\n");
                return -1;
            }
            if (strcmp(property, "layer2") == 0 && strcmp(interface_type, "vlan") != 0) {
                fprintf(stderr, "Error: layer2 operations only valid for vlan interfaces\n");
                return -1;
            }
            if (strcmp(property, "vxlanid") == 0 && strcmp(interface_type, "vxlan") != 0) {
                fprintf(stderr, "Error: vxlanid operations only valid for vxlan interfaces\n");
                return -1;
            }
            if (strcmp(property, "vxlandev") == 0 && strcmp(interface_type, "vxlan") != 0) {
                fprintf(stderr, "Error: vxlandev operations only valid for vxlan interfaces\n");
                return -1;
            }
            
            /* Handle extended epair syntax with vrf and address properties */
            debug_log(DEBUG_INFO, "Checking peer property: property='%s', arg_count=%d", property, cmd->arg_count);
            debug_log(DEBUG_INFO, "Peer condition check: property='%s', arg_count=%d, condition=%s", 
                      property, cmd->arg_count, (strcmp(property, "peer") == 0 && cmd->arg_count >= 6) ? "TRUE" : "FALSE");
            if (strcmp(property, "peer") == 0 && cmd->arg_count >= 6) {
                const char *peer = value;
                const char *sub_property = cmd->arg_count >= 6 ? cmd->args[4] : NULL;
                const char *sub_value = cmd->arg_count >= 6 ? cmd->args[5] : NULL;
                
                debug_log(DEBUG_INFO, "Setting epair peer: type=%s, name=%s, peer=%s, sub_property=%s, sub_value=%s", 
                          interface_type, interface_name, peer, sub_property ? sub_property : "none", sub_value ? sub_value : "none");
                
                snprintf(request, sizeof(request),
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                    "  <edit-config>\n"
                    "    <target><running/></target>\n"
                    "    <config>\n"
                    "      <set xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
                    "        <object>interface</object>\n"
                    "        <type>%s</type>\n"
                    "        <name>%s</name>\n"
                    "        <property>peer</property>\n"
                    "        <value>%s</value>\n"
                    "        <sub_property>%s</sub_property>\n"
                    "        <sub_value>%s</sub_value>\n"
                    "      </set>\n"
                    "    </config>\n"
                    "  </edit-config>\n"
                    "</rpc>\n",
                    interface_type, interface_name, peer, 
                    sub_property ? sub_property : "", sub_value ? sub_value : "");
                
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
            } else {
                /* Standard interface property setting */
                debug_log(DEBUG_INFO, "Using standard interface property setting branch");
                snprintf(request, sizeof(request),
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                    "  <edit-config>\n"
                    "    <target><running/></target>\n"
                    "    <config>\n"
                    "      <set xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
                    "        <object>interface</object>\n"
                    "        <type>%s</type>\n"
                    "        <name>%s</name>\n"
                    "        <property>%s</property>\n"
                    "        <value>%s</value>\n"
                    "      </set>\n"
                    "    </config>\n"
                    "  </edit-config>\n"
                    "</rpc>\n",
                    interface_type, interface_name, property, value);
            }
            
            /* Handle tunnel operations with local/remote addresses */
            if (strcmp(property, "tunnel") == 0) {
                /* For tunnel operations, we need to extract family, local, and remote from the value */
                if (cmd->arg_count >= 9) {
                    const char *family = cmd->args[7];
                    const char *local = cmd->args[9];
                    const char *remote = cmd->args[11];
                    
                    snprintf(request, sizeof(request),
                        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                        "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                        "  <edit-config>\n"
                        "    <target><running/></target>\n"
                        "    <config>\n"
                        "      <interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">\n"
                        "        <interface>\n"
                        "          <name>%s</name>\n"
                        "          <type xmlns:x=\"urn:ietf:params:xml:ns:yang:ietf-if-extensions\">x:ethernetCsmacd</type>\n"
                        "          <enabled>true</enabled>\n"
                        "          <property xmlns=\"urn:ietf:params:xml:ns:yang:netd\">%s</property>\n"
                        "          <family>%s</family>\n"
                        "          <local>%s</local>\n"
                        "          <remote>%s</remote>\n"
                        "        </interface>\n"
                        "      </interfaces>\n"
                        "    </config>\n"
                        "  </edit-config>\n"
                        "</rpc>\n",
                        interface_name, property, family, local, remote);
                }
            } else {
                /* Standard property assignment */
                debug_log(DEBUG_INFO, "Using standard property assignment branch - this is wrong for epair commands!");
                snprintf(request, sizeof(request),
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                    "  <edit-config>\n"
                    "    <target><running/></target>\n"
                    "    <config>\n"
                    "      <interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">\n"
                    "        <interface>\n"
                    "          <name>%s</name>\n"
                    "          <type xmlns:x=\"urn:ietf:params:xml:ns:yang:ietf-if-extensions\">x:ethernetCsmacd</type>\n"
                    "          <enabled>true</enabled>\n"
                    "          <property xmlns=\"urn:ietf:params:xml:ns:yang:netd\">%s</property>\n"
                    "          <value>%s</value>\n"
                    "        </interface>\n"
                    "      </interfaces>\n"
                    "    </config>\n"
                    "  </edit-config>\n"
                    "</rpc>\n",
                    interface_name, property, value);
            }
            
            debug_log(DEBUG_INFO, "Generated XML request: %s", request);
            
        } else {
            /* Generic set command for other objects */
            debug_log(DEBUG_INFO, "Using generic set command branch");
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
        
        debug_log(DEBUG_INFO, "Generated XML request (generic): %s", request);
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
    char request[4096];
    char *response = NULL;
    int ret = 0;
    
    debug_log(DEBUG_INFO, "Executing SHOW command");
    
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
        int family = AF_UNSPEC;
        
        /* Check if address family is specified */
        if (cmd->arg_count >= 6) {
            if (strcmp(cmd->args[5], "inet") == 0) {
                family = AF_INET;
                debug_log(DEBUG_INFO, "Filtering for IPv4 routes only");
            } else if (strcmp(cmd->args[5], "inet6") == 0) {
                family = AF_INET6;
                debug_log(DEBUG_INFO, "Filtering for IPv6 routes only");
            }
        }
        
        ret = netconf_get_routes(client, fib_id, family, &response);
    } else {
        /* Check for VRF FIBID route command */
        if (cmd->object == OBJ_ROUTE && cmd->arg_count >= 5 && 
            strcmp(cmd->args[0], "vrf") == 0 && strcmp(cmd->args[1], "id") == 0 &&
            strcmp(cmd->args[3], "protocol") == 0 && strcmp(cmd->args[4], "static") == 0) {
            debug_log(DEBUG_INFO, "Handling VRF FIBID route command");
            /* Extract FIB ID and get routes for that FIB */
            uint32_t fib_id = atoi(cmd->args[2]);
            int family = AF_UNSPEC;
            
            /* Check if address family is specified */
            if (cmd->arg_count >= 6) {
                if (strcmp(cmd->args[5], "inet") == 0) {
                    family = AF_INET;
                    debug_log(DEBUG_INFO, "Filtering for IPv4 routes only");
                } else if (strcmp(cmd->args[5], "inet6") == 0) {
                    family = AF_INET6;
                    debug_log(DEBUG_INFO, "Filtering for IPv6 routes only");
                }
            }
            
            ret = netconf_get_routes(client, fib_id, family, &response);
        } else if (cmd->object == OBJ_INTERFACE) {
            debug_log(DEBUG_INFO, "Processing interface command: arg_count=%d, args[0]='%s', args[1]='%s'", 
                      cmd->arg_count, cmd->args[0], cmd->args[1]);
            
            if (cmd->arg_count >= 2 && strcmp(cmd->args[0], "group") == 0) {
                /* Show interfaces filtered by group - use default table view */
                const char *group = cmd->args[1];
                debug_log(DEBUG_INFO, "Showing interfaces in group: %s", group);
                
                snprintf(request, sizeof(request),
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                    "  <get>\n"
                    "    <filter type=\"subtree\">\n"
                    "      <interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">\n"
                    "        <interface>\n"
                    "          <name/>\n"
                    "          <type/>\n"
                    "          <enabled/>\n"
                    "          <group xmlns=\"urn:ietf:params:xml:ns:yang:netd\">%s</group>\n"
                    "        </interface>\n"
                    "      </interfaces>\n"
                    "    </filter>\n"
                    "  </get>\n"
                    "</rpc>\n",
                    group);
            } else if (cmd->arg_count >= 2 && strcmp(cmd->args[0], "type") == 0) {
                /* Show interfaces by type - use custom table view */
                const char *interface_type = cmd->args[1];
                debug_log(DEBUG_INFO, "Showing interfaces of type: %s", interface_type);
                
                snprintf(request, sizeof(request),
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                    "  <get>\n"
                    "    <filter type=\"subtree\">\n"
                    "      <interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">\n"
                    "        <interface>\n"
                    "          <name/>\n"
                    "          <type/>\n"
                    "          <enabled/>\n"
                    "          <status/>\n"
                    "          <vrf/>\n"
                    "          <members/>\n"
                    "          <mtu/>\n"
                    "          <flags/>\n"
                    "          <ipv4/>\n"
                    "          <ipv6/>\n"
                    "          <groups/>\n"
                    "        </interface>\n"
                    "      </interfaces>\n"
                    "    </filter>\n"
                    "  </get>\n"
                    "</rpc>\n");
                
                debug_log(DEBUG_INFO, "Generated XML request: %s", request);
            } else if (cmd->arg_count >= 2) {
                /* Show specific interface */
                const char *interface_type = cmd->args[0];
                const char *interface_name = cmd->args[1];
                debug_log(DEBUG_INFO, "Showing interface: %s %s", interface_type, interface_name);
                
                snprintf(request, sizeof(request),
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                    "  <get>\n"
                    "    <filter type=\"subtree\">\n"
                    "      <interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">\n"
                    "        <interface>\n"
                    "          <name>%s</name>\n"
                    "          <type>%s</type>\n"
                    "          <enabled/>\n"
                    "          <status/>\n"
                    "          <vrf/>\n"
                    "          <members/>\n"
                    "          <mtu/>\n"
                    "          <flags/>\n"
                    "          <ipv4/>\n"
                    "          <ipv6/>\n"
                    "          <groups/>\n"
                    "        </interface>\n"
                    "      </interfaces>\n"
                    "    </filter>\n"
                    "  </get>\n"
                    "</rpc>\n",
                    interface_name, interface_type);
            } else {
                /* Show all interfaces - default table view */
                debug_log(DEBUG_INFO, "Showing all interfaces");
                
                snprintf(request, sizeof(request),
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                    "  <get>\n"
                    "    <filter type=\"subtree\">\n"
                    "      <interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">\n"
                    "        <interface>\n"
                    "          <name/>\n"
                    "          <type/>\n"
                    "          <enabled/>\n"
                    "          <status/>\n"
                    "          <vrf/>\n"
                    "          <members/>\n"
                    "          <mtu/>\n"
                    "          <flags/>\n"
                    "          <ipv4/>\n"
                    "          <ipv6/>\n"
                    "          <groups/>\n"
                    "        </interface>\n"
                    "      </interfaces>\n"
                    "    </filter>\n"
                    "  </get>\n"
                    "</rpc>\n");
            }
            
            /* Send the NETCONF request and handle response */
            ret = netconf_send_request(client, request, &response);
            if (ret == 0 && response) {
                debug_log(DEBUG_INFO, "Received XML response: %s", response);
                /* Parse XML and display formatted interface table */
                if (cmd->arg_count >= 3 && strcmp(cmd->args[1], "type") == 0) {
                    const char *interface_type = cmd->args[2];
                    if (strcmp(interface_type, "bridge") == 0) {
                        print_iftype_bridge_table(response);
                    } else if (strcmp(interface_type, "vlan") == 0) {
                        print_iftype_vlan_table(response);
                    } else if (strcmp(interface_type, "lagg") == 0) {
                        print_iftype_lagg_table(response);
                    } else if (strcmp(interface_type, "ethernet") == 0) {
                        print_iftype_ethernet_table(response);
                    } else if (strcmp(interface_type, "tap") == 0) {
                        print_iftype_tap_table(response);
                    } else if (strcmp(interface_type, "gif") == 0) {
                        print_iftype_gif_table(response);
                    } else if (strcmp(interface_type, "epair") == 0) {
                        print_iftype_epair_table(response);
                    } else if (strcmp(interface_type, "vxlan") == 0) {
                        print_iftype_vxlan_table(response);
                    } else if (strcmp(interface_type, "loopback") == 0) {
                        print_iftype_loopback_table(response);
                    } else if (strcmp(interface_type, "wlan") == 0) {
                        print_iftype_wlan_table(response);
                    } else {
                        print_interface_table(response);
                    }
                } else {
                    print_interface_table(response);
                }
                free(response);
            } else {
                print_error("Failed to get interface information");
            }
            return ret;
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
                        print_iftype_wlan_table(response);
                    } else {
                        print_interface_table(response);
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
        if (response) {
            free(response);
        }
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
        /* Handle interface delete commands */
        if (cmd->object == OBJ_INTERFACE && cmd->arg_count >= 2 && 
            strcmp(cmd->args[0], "name") == 0) {
            /* Format: delete interface name <name> [property [value]] */
            const char *interface_name = cmd->args[1];
            const char *property = cmd->arg_count >= 3 ? cmd->args[2] : NULL;
            const char *value = cmd->arg_count >= 4 ? cmd->args[3] : NULL;
            
            debug_log(DEBUG_INFO, "Deleting interface: name=%s, property=%s, value=%s", 
                      interface_name, property ? property : "all", value ? value : "none");
            
            if (property && strcmp(property, "address") == 0 && value) {
                /* Delete specific address */
                snprintf(request, sizeof(request),
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                    "  <edit-config>\n"
                    "    <target><running/></target>\n"
                    "    <config>\n"
                    "      <delete xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
                    "        <object>interface</object>\n"
                    "        <name>%s</name>\n"
                    "        <property>address</property>\n"
                    "        <value>%s</value>\n"
                    "      </delete>\n"
                    "    </config>\n"
                    "  </edit-config>\n"
                    "</rpc>\n",
                    interface_name, value);
            } else if (property && strcmp(property, "member") == 0) {
                if (value) {
                    /* Delete specific member */
                    snprintf(request, sizeof(request),
                        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                        "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                        "  <edit-config>\n"
                        "    <target><running/></target>\n"
                        "    <config>\n"
                        "      <delete xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
                        "        <object>interface</object>\n"
                        "        <name>%s</name>\n"
                        "        <property>member</property>\n"
                        "        <value>%s</value>\n"
                        "      </delete>\n"
                        "    </config>\n"
                        "  </edit-config>\n"
                        "</rpc>\n",
                        interface_name, value);
                } else {
                    /* Delete all members */
                    snprintf(request, sizeof(request),
                        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                        "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                        "  <edit-config>\n"
                        "    <target><running/></target>\n"
                        "    <config>\n"
                        "      <delete xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
                        "        <object>interface</object>\n"
                        "        <name>%s</name>\n"
                        "        <property>member</property>\n"
                        "      </delete>\n"
                        "    </config>\n"
                        "  </edit-config>\n"
                        "</rpc>\n",
                        interface_name);
                }
            } else if (property && strcmp(property, "laggport") == 0) {
                if (value) {
                    /* Delete specific laggport */
                    snprintf(request, sizeof(request),
                        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                        "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                        "  <edit-config>\n"
                        "    <target><running/></target>\n"
                        "    <config>\n"
                        "      <delete xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
                        "        <object>interface</object>\n"
                        "        <name>%s</name>\n"
                        "        <property>laggport</property>\n"
                        "        <value>%s</value>\n"
                        "      </delete>\n"
                        "    </config>\n"
                        "  </edit-config>\n"
                        "</rpc>\n",
                        interface_name, value);
                } else {
                    /* Delete all laggports */
                    snprintf(request, sizeof(request),
                        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                        "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                        "  <edit-config>\n"
                        "    <target><running/></target>\n"
                        "    <config>\n"
                        "      <delete xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
                        "        <object>interface</object>\n"
                        "        <name>%s</name>\n"
                        "        <property>laggport</property>\n"
                        "      </delete>\n"
                        "    </config>\n"
                        "  </edit-config>\n"
                        "</rpc>\n",
                        interface_name);
                }
            } else {
                /* Delete entire interface */
                snprintf(request, sizeof(request),
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<rpc message-id=\"1\" xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\">\n"
                    "  <edit-config>\n"
                    "    <target><running/></target>\n"
                    "    <config>\n"
                    "      <delete xmlns=\"urn:ietf:params:xml:ns:yang:netd\">\n"
                    "        <object>interface</object>\n"
                    "        <name>%s</name>\n"
                    "      </delete>\n"
                    "    </config>\n"
                    "  </edit-config>\n"
                    "</rpc>\n",
                    interface_name);
            }
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
                if (parse_command_yacc(line, &cmd) == 0) {
                    execute_command(client, &cmd);
                } else {
                    /* Fall back to simple parser if YACC fails */
                    debug_log(DEBUG_DEBUG, "YACC parsing failed in interactive mode, trying simple parser");
                    if (parse_command(line, &cmd) == 0) {
                        /* Suppress the YACC error message since the fallback parser succeeded */
                        /* The command is valid, just not recognized by YACC */
                        execute_command(client, &cmd);
                    } else {
                        /* Only show error if both parsers fail */
                        print_error("Invalid command syntax");
                    }
                }
            }
        }
        
        free(line);
    }
    
    return 0;
} 