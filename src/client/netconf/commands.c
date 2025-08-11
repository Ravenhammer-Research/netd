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

#include <net.h>
#include <netconf.h>
#include <table/table.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/**
 * Execute a command
 */
int execute_command(net_client_t *client, const command_t *cmd) {
  if (!client || !cmd) {
    return -1;
  }

  /* Ensure we're connected to the server */
  if (!client->connected) {
    debug_log(INFO, "Connecting to server...");
    if (netconf_connect(client) < 0) {
      fprintf(stderr, "Failed to connect to server\n");
      return -1;
    }
    debug_log(INFO, "Connected to server");
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
    fprintf(stderr, "Unknown command type: %d\n", cmd->type);
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
    fprintf(stderr, "SET command requires object, name, and property\n");
    return -1;
  }

  /* Debug logging to see command structure */
  debug_log(DEBUG, "SET command object: %d, arg_count: %d", cmd->object,
            cmd->arg_count);
  for (int i = 0; i < cmd->arg_count && i < 10; i++) {
    debug_log(DEBUG, "args[%d]: '%s'", i, cmd->args[i]);
  }

  /* Handle route setting commands */
  if (cmd->arg_count >= 6 && strcmp(cmd->args[0], "vrf") == 0 &&
      strcmp(cmd->args[1], "id") == 0 &&
      strcmp(cmd->args[3], "protocol") == 0 &&
      strcmp(cmd->args[4], "static") == 0 &&
      (strcmp(cmd->args[5], "inet") == 0 ||
       strcmp(cmd->args[5], "inet6") == 0)) {

    uint32_t fib = atoi(cmd->args[2]); /* VRF ID/FIB */
    const char *family = cmd->args[5]; /* Address family */
    const char *route_type = NULL;     /* host or network */
    const char *destination = NULL;    /* host address or network */
    const char *next_hop_type = NULL;  /* gateway or iface */
    const char *next_hop = NULL;       /* gateway address or interface name */

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
            debug_log(INFO,
                      "Route with gateway %s and interface constraint %s",
                      next_hop, interface);
          }
        } else if (strcmp(cmd->args[8], "iface") == 0) {
          next_hop_type = "iface";
          next_hop = cmd->args[9];
        }
      }

      /* Build NETCONF request for route */
      if (route_type && destination) {
        if (strcmp(family, "inet") == 0) {
          snprintf(request, sizeof(request),
                   "<rpc message-id=\"1\">"
                   "<edit-config>"
                   "<target><running/></target>"
                   "<config>"
                   "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">"
                   "<interface>"
                   "<name>%s</name>"
                   "<ipv4 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">"
                   "<address>"
                   "<ip>%s</ip>"
                   "<prefix-length>%d</prefix-length>"
                   "</address>"
                   "</ipv4>"
                   "</interface>"
                   "</interfaces>"
                   "</config>"
                   "</edit-config>"
                   "</rpc>",
                   next_hop, destination, 24); /* Default /24 for now */
        } else if (strcmp(family, "inet6") == 0) {
          snprintf(request, sizeof(request),
                   "<rpc message-id=\"1\">"
                   "<edit-config>"
                   "<target><running/></target>"
                   "<config>"
                   "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">"
                   "<interface>"
                   "<name>%s</name>"
                   "<ipv6 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">"
                   "<address>"
                   "<ip>%s</ip>"
                   "<prefix-length>%d</prefix-length>"
                   "</address>"
                   "</ipv6>"
                   "</interface>"
                   "</interfaces>"
                   "</config>"
                   "</edit-config>"
                   "</rpc>",
                   next_hop, destination, 64); /* Default /64 for IPv6 */
        }

        debug_log(INFO, "Sending route SET request: %s", request);
        if (netconf_send_request(client, request, &response) < 0) {
          fprintf(stderr, "Failed to send route SET request\n");
          return -1;
        }

        /* Log the route details for debugging */
        debug_log(INFO, "Route SET: type=%s, dest=%s, family=%s, fib=%u", 
                 route_type, destination, family, fib);
        if (next_hop_type && next_hop) {
          debug_log(INFO, "Route SET: next_hop_type=%s, next_hop=%s", 
                   next_hop_type, next_hop);
        }
      }
    }
  }

  /* Handle interface setting commands */
  if (cmd->arg_count >= 4 && strcmp(cmd->args[0], "interface") == 0) {
    const char *ifname = cmd->args[1];
    const char *property = cmd->args[2];
    const char *value = cmd->args[3];

    if (strcmp(property, "description") == 0) {
      snprintf(request, sizeof(request),
               "<rpc message-id=\"1\">"
               "<edit-config>"
               "<target><running/></target>"
               "<config>"
               "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">"
               "<interface>"
               "<name>%s</name>"
               "<description>%s</description>"
               "</interface>"
               "</interfaces>"
               "</config>"
               "</edit-config>"
               "</rpc>",
               ifname, value);
    } else if (strcmp(property, "mtu") == 0) {
      int mtu = atoi(value);
      if (mtu <= 0 || mtu > 9000) {
        fprintf(stderr, "Invalid MTU value: %s\n", value);
        return -1;
      }
      snprintf(request, sizeof(request),
               "<rpc message-id=\"1\">"
               "<edit-config>"
               "<target><running/></target>"
               "<config>"
               "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">"
               "<interface>"
               "<name>%s</name>"
               "<ipv4 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">"
               "<mtu>%d</mtu>"
               "</ipv4>"
               "</interface>"
               "</interfaces>"
               "</config>"
               "</edit-config>"
               "</rpc>",
               ifname, mtu);
    } else if (strcmp(property, "enabled") == 0) {
      bool enabled = (strcmp(value, "true") == 0);
      snprintf(request, sizeof(request),
               "<rpc message-id=\"1\">"
               "<edit-config>"
               "<target><running/></target>"
               "<config>"
               "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">"
               "<interface>"
               "<name>%s</name>"
               "<enabled>%s</enabled>"
               "</interface>"
               "</interfaces>"
               "</config>"
               "</edit-config>"
               "</rpc>",
               ifname, enabled ? "true" : "false");
    } else {
      fprintf(stderr, "Unknown interface property: %s\n", property);
      return -1;
    }

    debug_log(INFO, "Sending interface SET request: %s", request);
    if (netconf_send_request(client, request, &response) < 0) {
      fprintf(stderr, "Failed to send interface SET request\n");
      return -1;
    }

    printf("Interface %s %s set to %s successfully\n", ifname, property, value);
    free(response);
    return 0;
  }

  /* Handle VRF setting commands */
  if (cmd->arg_count >= 4 && strcmp(cmd->args[0], "vrf") == 0) {
    if (strcmp(cmd->args[1], "id") == 0) {
      uint32_t fib = atoi(cmd->args[2]);
      const char *property = cmd->args[3];
      const char *value = cmd->args[4];

      if (strcmp(property, "description") == 0) {
        snprintf(request, sizeof(request),
                 "<rpc message-id=\"1\">"
                 "<edit-config>"
                 "<target><running/></target>"
                 "<config>"
                 "<vrf xmlns=\"urn:ietf:params:xml:ns:yang:ietf-routing\">"
                 "<routing-instance>"
                 "<name>%d</name>"
                 "<description>%s</description>"
                 "</routing-instance>"
                 "</vrf>"
                 "</config>"
                 "</edit-config>"
                 "</rpc>",
                 fib, value);
      } else {
        fprintf(stderr, "Unknown VRF property: %s\n", property);
        return -1;
      }

      debug_log(INFO, "Sending VRF SET request: %s", request);
      if (netconf_send_request(client, request, &response) < 0) {
        fprintf(stderr, "Failed to send VRF SET request\n");
        return -1;
      }

      printf("VRF %d %s set to %s successfully\n", fib, property, value);
      free(response);
      return 0;
    }
  }

  fprintf(stderr, "Unsupported SET command format\n");
  return -1;
}

/**
 * Execute a SHOW command
 */
int execute_show_command(net_client_t *client, const command_t *cmd) {
  char *response = NULL;
  int ret = 0;

  if (!client || !cmd) {
    return -1;
  }

  if (cmd->arg_count < 1) {
    fprintf(stderr, "SHOW command requires an object to display\n");
    return -1;
  }

  const char *object = cmd->args[0];

  if (strcmp(object, "interfaces") == 0 || (strcmp(object, "interface") == 0 && cmd->arg_count == 1)) {
    if (netconf_get_interfaces(client, &response, NULL) < 0) {
      fprintf(stderr, "Failed to get interfaces\n");
      return -1;
    }

      print_interface_table(response);

    free(response);
  } else if (strcmp(object, "routes") == 0 || strcmp(object, "route") == 0) {
    uint32_t fib = 0; /* Default VRF */
    int family = AF_INET; /* Default to IPv4 */

    if (cmd->arg_count >= 2) {
      if (strcmp(cmd->args[1], "inet") == 0) {
        family = AF_INET;
      } else if (strcmp(cmd->args[1], "inet6") == 0) {
        family = AF_INET6;
      } else {
        fib = atoi(cmd->args[1]);
      }
    }

    if (cmd->arg_count >= 3 && strcmp(cmd->args[2], "inet") == 0) {
      family = AF_INET;
    } else if (cmd->arg_count >= 3 && strcmp(cmd->args[2], "inet6") == 0) {
      family = AF_INET6;
    }

    if (netconf_get_routes(client, fib, family, &response) < 0) {
      fprintf(stderr, "Failed to get routes\n");
      return -1;
    }

    print_routes_table(response);
    free(response);
  } else if (strcmp(object, "vrfs") == 0 || strcmp(object, "vrf") == 0) {
    /* Handle VRF protocol static routes */
    if (cmd->arg_count >= 4 && strcmp(cmd->args[1], "id") == 0 && 
        strcmp(cmd->args[3], "protocol") == 0 && strcmp(cmd->args[4], "static") == 0) {
      
      uint32_t fib = atoi(cmd->args[2]); /* VRF ID/FIB */
      int family = AF_UNSPEC; /* Default to unspecified (both IPv4 and IPv6) */
      
      /* Check for address family specification */
      if (cmd->arg_count >= 6) {
        if (strcmp(cmd->args[5], "inet") == 0) {
          family = AF_INET;
        } else if (strcmp(cmd->args[5], "inet6") == 0) {
          family = AF_INET6;
        }
      }
      
      debug_log(INFO, "SHOW VRF protocol static: fib=%u, family=%d", fib, family);
      
      if (netconf_get_routes(client, fib, family, &response) < 0) {
        fprintf(stderr, "Failed to get VRF routes\n");
        return -1;
      }
      
      print_routes_table(response);
      free(response);
    } else {
      /* Handle basic VRF listing */
    if (netconf_get_vrfs(client, &response) < 0) {
      fprintf(stderr, "Failed to get VRFs\n");
      return -1;
    }

    print_vrf_table(response);
    free(response);
    }
  } else if (strcmp(object, "interface") == 0 && cmd->arg_count >= 2) {
    const char *type = NULL;

    debug_log(DEBUG, "SHOW interface command: arg_count=%d", cmd->arg_count);
    for (int i = 0; i < cmd->arg_count; i++) {
      debug_log(DEBUG, "SHOW interface args[%d]: '%s'", i, cmd->args[i]);
    }

    /* Check if this is "show interface type <type>" */
    if (cmd->arg_count == 3 && strcmp(cmd->args[1], "type") == 0) {
      type = cmd->args[2];
      debug_log(DEBUG, "Found type filter: %s", type);
    } else {
      debug_log(DEBUG, "No type filter found");
    }

    debug_log(INFO, "SHOW interface type: %s", type ? type : "all");

    if (netconf_get_interfaces(client, &response, type) < 0) {
      fprintf(stderr, "Failed to get interfaces%s\n", type ? " of specified type" : "");
      return -1;
    }

    /* Display using appropriate table */
    if (type && strcmp(type, "bridge") == 0) {
        print_bridge_table(response);
    } else {
      print_interface_table(response);
    }

    free(response);
  } else {
    fprintf(stderr, "Unknown SHOW object: %s\n", object);
    ret = -1;
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

  if (cmd->arg_count < 3) {
    fprintf(stderr, "DELETE command requires object, name, and property\n");
    return -1;
  }

  const char *object = cmd->args[0];
  const char *name = cmd->args[1];
  const char *property = cmd->args[2];

  if (strcmp(object, "interface") == 0) {
    if (strcmp(property, "description") == 0) {
      snprintf(request, sizeof(request),
               "<rpc message-id=\"1\">"
               "<edit-config>"
               "<target><running/></target>"
               "<config>"
               "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">"
               "<interface>"
               "<name>%s</name>"
               "<description operation=\"delete\"/>"
               "</interface>"
               "</interfaces>"
               "</config>"
               "</edit-config>"
               "</rpc>",
               name);
    } else if (strcmp(property, "mtu") == 0) {
      snprintf(request, sizeof(request),
               "<rpc message-id=\"1\">"
               "<edit-config>"
               "<target><running/></target>"
               "<config>"
               "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">"
               "<interface>"
               "<name>%s</name>"
               "<ipv4 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">"
               "<mtu operation=\"delete\"/>"
               "</ipv4>"
               "</interface>"
               "</interfaces>"
               "</config>"
               "</edit-config>"
               "</rpc>",
               name);
    } else {
      fprintf(stderr, "Unknown interface property to delete: %s\n", property);
      return -1;
    }

    debug_log(INFO, "Sending interface DELETE request: %s", request);
    if (netconf_send_request(client, request, &response) < 0) {
      fprintf(stderr, "Failed to send interface DELETE request\n");
      return -1;
    }

    printf("Interface %s %s deleted successfully\n", name, property);
    free(response);
    return 0;
  } else if (strcmp(object, "vrf") == 0) {
    if (strcmp(name, "id") == 0 && cmd->arg_count >= 4) {
      uint32_t fib = atoi(cmd->args[2]);
      const char *vrf_property = cmd->args[3];

      if (strcmp(vrf_property, "description") == 0) {
        snprintf(request, sizeof(request),
                 "<rpc message-id=\"1\">"
                 "<edit-config>"
                 "<target><running/></target>"
                 "<config>"
                 "<vrf xmlns=\"urn:ietf:params:xml:ns:yang:ietf-routing\">"
                 "<routing-instance>"
                 "<name>%d</name>"
                 "<description operation=\"delete\"/>"
                 "</routing-instance>"
                 "</vrf>"
                 "</config>"
                 "</edit-config>"
                 "</rpc>",
                 fib);
      } else {
        fprintf(stderr, "Unknown VRF property to delete: %s\n", vrf_property);
        return -1;
      }

      debug_log(INFO, "Sending VRF DELETE request: %s", request);
      if (netconf_send_request(client, request, &response) < 0) {
        fprintf(stderr, "Failed to send VRF DELETE request\n");
        return -1;
      }

      printf("VRF %d %s deleted successfully\n", fib, vrf_property);
      free(response);
      return 0;
    }
  }

  fprintf(stderr, "Unsupported DELETE command format\n");
  return -1;
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

  /* Build save request */
  snprintf(request, sizeof(request),
           "<rpc message-id=\"1\">"
           "<copy-config>"
           "<target><startup/></target>"
           "<source><running/></source>"
           "</copy-config>"
           "</rpc>");

  debug_log(INFO, "Sending save request: %s", request);
  if (netconf_send_request(client, request, &response) < 0) {
    fprintf(stderr, "Failed to save configuration\n");
    return -1;
  }

  printf("Configuration saved successfully\n");
  free(response);
  return 0;
} 