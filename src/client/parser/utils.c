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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Global variables for parser */
command_t *current_command = NULL;
int parse_error = 0;

/**
 * Convert command type enum to string
 * @param type Command type
 * @return String representation
 */
const char *command_type_to_string(command_type_t type) {
  switch (type) {
  case CMD_SET:
    return "set";
  case CMD_SHOW:
    return "show";
  case CMD_DELETE:
    return "delete";
  case CMD_COMMIT:
    return "commit";
  case CMD_SAVE:
    return "save";
  default:
    return "unknown";
  }
}

/**
 * Convert string to command type enum
 * @param str Command type string
 * @return Command type enum
 */
command_type_t command_type_from_string(const char *str) {
  if (!str) {
    return CMD_UNKNOWN;
  }

  if (strcmp(str, "set") == 0) {
    return CMD_SET;
  } else if (strcmp(str, "show") == 0) {
    return CMD_SHOW;
  } else if (strcmp(str, "delete") == 0) {
    return CMD_DELETE;
  } else if (strcmp(str, "commit") == 0) {
    return CMD_COMMIT;
  } else if (strcmp(str, "save") == 0) {
    return CMD_SAVE;
  }

  return CMD_UNKNOWN;
}

/**
 * Convert object type enum to string
 * @param type Object type
 * @return String representation
 */
const char *object_type_to_string(object_type_t type) {
  switch (type) {
  case OBJ_VRF:
    return "vrf";
  case OBJ_INTERFACE:
    return "interface";
  case OBJ_ROUTE:
    return "route";
  default:
    return "unknown";
  }
}

/**
 * Convert string to object type enum
 * @param str Object type string
 * @return Object type enum
 */
object_type_t object_type_from_string(const char *str) {
  if (!str) {
    return OBJ_UNKNOWN;
  }

  if (strcmp(str, "vrf") == 0) {
    return OBJ_VRF;
  } else if (strcmp(str, "interface") == 0) {
    return OBJ_INTERFACE;
  } else if (strcmp(str, "route") == 0) {
    return OBJ_ROUTE;
  }

  return OBJ_UNKNOWN;
}

/**
 * Convert interface type enum to string
 * @param type Interface type
 * @return String representation
 */
const char *interface_type_to_string(interface_type_t type) {
  switch (type) {
  case IF_TYPE_ETHERNET:
    return "ethernetCsmacd";
  case IF_TYPE_WIRELESS:
    return "ieee80211";
  case IF_TYPE_EPAIR:
    return "epair";
  case IF_TYPE_GIF:
    return "gif";
  case IF_TYPE_GRE:
    return "gre";
  case IF_TYPE_LAGG:
    return "lagg";
  case IF_TYPE_LOOPBACK:
    return "softwareLoopback";
  case IF_TYPE_OVPN:
    return "ovpn";
  case IF_TYPE_TUN:
    return "tun";
  case IF_TYPE_TAP:
    return "tap";
  case IF_TYPE_VLAN:
    return "l2vlan";
  case IF_TYPE_VXLAN:
    return "vxlan";
  case IF_TYPE_BRIDGE:
    return "bridge";
  default:
    return "other";
  }
}

/**
 * Convert string to interface type enum
 * @param str Interface type string
 * @return Interface type enum
 */
interface_type_t interface_type_from_string(const char *str) {
  if (!str) {
    return IF_TYPE_UNKNOWN;
  }

  if (strcmp(str, "ethernet") == 0 || strcmp(str, "ethernetCsmacd") == 0) {
    return IF_TYPE_ETHERNET;
  } else if (strcmp(str, "wireless") == 0 ||
             strcmp(str, "wireless80211") == 0 ||
             strcmp(str, "ieee80211") == 0) {
    return IF_TYPE_WIRELESS;
  } else if (strcmp(str, "epair") == 0) {
    return IF_TYPE_EPAIR;
  } else if (strcmp(str, "gif") == 0) {
    return IF_TYPE_GIF;
  } else if (strcmp(str, "gre") == 0) {
    return IF_TYPE_GRE;
  } else if (strcmp(str, "lagg") == 0) {
    return IF_TYPE_LAGG;
  } else if (strcmp(str, "lo") == 0 || strcmp(str, "loopback") == 0 ||
             strcmp(str, "softwareLoopback") == 0) {
    return IF_TYPE_LOOPBACK;
  } else if (strcmp(str, "ovpn") == 0 || strcmp(str, "tunnel") == 0) {
    return IF_TYPE_OVPN;
  } else if (strcmp(str, "tun") == 0) {
    return IF_TYPE_TUN;
  } else if (strcmp(str, "tap") == 0) {
    return IF_TYPE_TAP;
  } else if (strcmp(str, "vlan") == 0 || strcmp(str, "l2vlan") == 0) {
    return IF_TYPE_VLAN;
  } else if (strcmp(str, "vxlan") == 0) {
    return IF_TYPE_VXLAN;
  } else if (strcmp(str, "bridge") == 0) {
    return IF_TYPE_BRIDGE;
  }

  return IF_TYPE_UNKNOWN;
}

/**
 * Parse command line into command structure
 * @param line Command line string
 * @param cmd Command structure to fill
 * @return 0 on success, -1 on failure
 */
int parse_command(const char *line, command_t *cmd) {
  char *line_copy, *token, *saveptr;
  int arg_count = 0;

  if (!line || !cmd) {
    return -1;
  }

  /* Initialize command structure */
  memset(cmd, 0, sizeof(*cmd));

  /* Make a copy of the line for parsing */
  line_copy = strdup(line);
  if (!line_copy) {
    return -1;
  }

  /* Parse first token (command type) */
  token = strtok_r(line_copy, " ", &saveptr);
  if (!token) {
    free(line_copy);
    return -1;
  }

  cmd->type = command_type_from_string(token);
  if (cmd->type == CMD_UNKNOWN) {
    fprintf(stderr, "Unknown command: %s\n", token);
    free(line_copy);
    return -1;
  }

  /* Parse remaining tokens */
  while ((token = strtok_r(NULL, " ", &saveptr)) != NULL && arg_count < 10) {
    strlcpy(cmd->args[arg_count], token, sizeof(cmd->args[arg_count]));
    arg_count++;
  }

  cmd->arg_count = arg_count;

  /* Determine object type based on arguments */
  if (arg_count > 0) {
    cmd->object = object_type_from_string(cmd->args[0]);
  }

  /* Special handling for set interface commands with complex structure */
  debug_log(
      DEBUG_DEBUG,
      "Checking for set interface command: type=%d, object=%d, arg_count=%d",
      cmd->type, cmd->object, arg_count);
  if (cmd->type == CMD_SET && cmd->object == OBJ_INTERFACE && arg_count >= 10) {
    /* Check if this is a set interface type <type> name <name> peer <peer> vrf
     * id <number> command */
    if (strcmp(cmd->args[1], "type") == 0 &&
        strcmp(cmd->args[3], "name") == 0 &&
        strcmp(cmd->args[5], "peer") == 0 && strcmp(cmd->args[7], "vrf") == 0 &&
        strcmp(cmd->args[8], "id") == 0) {

      /* Reorganize the arguments to match the expected format */
      char temp_args[10][64];
      for (int i = 0; i < arg_count; i++) {
        strlcpy(temp_args[i], cmd->args[i], sizeof(temp_args[i]));
      }

      /* args[0] = interface type (epair) */
      strlcpy(cmd->args[0], temp_args[2], sizeof(cmd->args[0]));
      /* args[1] = interface name (epair127) */
      strlcpy(cmd->args[1], temp_args[4], sizeof(cmd->args[1]));
      /* args[2] = property (peer) */
      strlcpy(cmd->args[2], temp_args[5], sizeof(cmd->args[2]));
      /* args[3] = value (b) */
      strlcpy(cmd->args[3], temp_args[6], sizeof(cmd->args[3]));
      /* args[4] = sub property (vrf) */
      strlcpy(cmd->args[4], temp_args[7], sizeof(cmd->args[4]));
      /* args[5] = sub value (18) */
      strlcpy(cmd->args[5], temp_args[9], sizeof(cmd->args[5]));

      cmd->arg_count = 6;
      debug_log(DEBUG_DEBUG,
                "Reorganized set interface command: type=%s, name=%s, "
                "property=%s, value=%s, sub_property=%s, sub_value=%s",
                cmd->args[0], cmd->args[1], cmd->args[2], cmd->args[3],
                cmd->args[4], cmd->args[5]);
    }
  }

  free(line_copy);
  return 0;
}

/**
 * Parse command using YACC parser
 * @param line Command line string
 * @param cmd Command structure to fill
 * @return 0 on success, -1 on failure
 */
int parse_command_yacc(const char *line, command_t *cmd) {
  extern int yyparse(void);
  extern void yyrestart(FILE *);
  extern FILE *yyin;
  extern command_t *current_command;
  extern int parse_error;

  if (!line || !cmd) {
    return -1;
  }

  /* Initialize command structure */
  memset(cmd, 0, sizeof(*cmd));

  /* Set up global variables for parser */
  current_command = cmd;
  parse_error = 0;

  /* Create a temporary file with the command line */
  FILE *temp_file = tmpfile();
  if (!temp_file) {
    debug_log(DEBUG_ERROR, "Failed to create temporary file for YACC parsing");
    return -1;
  }

  /* Write command line to temporary file */
  fprintf(temp_file, "%s\n", line);
  rewind(temp_file);

  /* Set up YACC input */
  yyin = temp_file;

  /* Parse the command */
  debug_log(DEBUG_DEBUG, "Starting YACC parse for command: %s", line);
  int result = yyparse();
  debug_log(DEBUG_DEBUG, "YACC parse result: %d, parse_error: %d", result,
            parse_error);

  /* Clean up */
  fclose(temp_file);
  yyin = NULL;
  current_command = NULL;

  if (parse_error || result != 0) {
    debug_log(DEBUG_ERROR, "YACC parsing failed for command: %s", line);
    return -1;
  }

  debug_log(DEBUG_DEBUG,
            "YACC parsing successful: type=%d, object=%d, arg_count=%d",
            cmd->type, cmd->object, cmd->arg_count);

  return 0;
} 