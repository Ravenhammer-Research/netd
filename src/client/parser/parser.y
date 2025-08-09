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

%{
#include "net.h"

#include "grammar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex(void);
extern int yyparse(void);
extern FILE *yyin;
extern char *yytext;
extern int yylineno;

%}

%union {
    char *string;
    int integer;
    command_type_t cmd_type;
    object_type_t obj_type;
    interface_type_t if_type;
    int family;
}

%token <string> IDENTIFIER
%token <string> IPV4_ADDRESS
%token <string> IPV6_ADDRESS
%token <string> CIDR_ADDRESS
%token <string> NUMBER
%token <string> STRING

/* Token declarations */
%token PROTOCOL

%token <cmd_type> SET
%token <cmd_type> SHOW
%token <cmd_type> DELETE
%token <cmd_type> COMMIT
%token <cmd_type> SAVE

%token <string> VRF
%token <obj_type> INTERFACE
%token <obj_type> ROUTE

%token <if_type> ETHERNET
%token <if_type> WIRELESS80211
%token <if_type> EPAIR
%token <if_type> GIF
%token <if_type> GRE
%token <if_type> LAGG
%token <if_type> LO
%token <if_type> OVPN
%token <if_type> TUN
%token <if_type> TAP
%token <if_type> VLAN
%token <if_type> VXLAN

%token <family> INET
%token <family> INET6

%token <string> TABLE
%token <string> STATIC
%token <string> ADDRESS
%token <string> MTU
%token <string> GROUP
%token <string> REJECT_TOKEN
%token <string> BLACKHOLE
%token <string> ID_TOKEN
%token <string> TYPE
%token <string> NAME
%token <string> MEMBER
%token <string> LAGGPROTO
%token <string> LAGGPORT
%token <string> PEER
%token <string> VLANDEV
%token <string> VLANPROTO
%token <string> TUNNEL
%token <string> LOCAL
%token <string> REMOTE
%token <string> TUNNELVRF
%token <string> LAYER2
%token <string> HOST
%token <string> GATEWAY
%token <string> IFACE
%token <string> VXLANID
%token <string> VXLANLOCAL
%token <string> VXLANREMOTE
%token <string> VXLANDEV
%token <string> EXIT
%token <string> QUIT
%token <string> HELP
%token <string> QUESTION_MARK
%token <string> CLEAR
%token <string> ROLLBACK
%token <string> LOOPBACK
%token <string> BRIDGE
%token <string> WLAN

/* Non-terminal type declarations */
%type <string> vrf_assignment
%type <string> address_assignment
%type <string> mtu_assignment
%type <string> group_assignment
%type <string> address_family
%type <string> interface_type
%type <string> route_action

/* Command grammar rules will be added here */

%%

/* Top level rule */
command
    : set_command
    | show_command
    | delete_command
    | exit_command
    | help_command
    | clear_command
    | commit_command
    | rollback_command
    | /* empty */
    ;

/* Set commands */
set_command
    : SET VRF IDENTIFIER TABLE NUMBER {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_VRF;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $5, sizeof(current_command->args[1]));
            current_command->arg_count = 2;
        }
    }
    | SET VRF ID_TOKEN NUMBER NAME IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_VRF;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], "name", sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], $5, sizeof(current_command->args[2]));
            current_command->arg_count = 3;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER vrf_assignment {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "vrf", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER address_assignment {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "address", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER mtu_assignment {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "mtu", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER group_assignment {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "group", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER MEMBER IDENTIFIER {
        if (current_command) {
            /* Only allow member operations for bridge interfaces */
            if (strcmp($4, "bridge") != 0) {
                fprintf(stderr, "Error: member operations only valid for bridge interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "member", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $8, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER LAGGPROTO IDENTIFIER {
        if (current_command) {
            /* Only allow laggproto operations for lagg interfaces */
            if (strcmp($4, "lagg") != 0) {
                fprintf(stderr, "Error: laggproto operations only valid for lagg interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "laggproto", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $8, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER LAGGPORT IDENTIFIER {
        if (current_command) {
            /* Only allow laggport operations for lagg interfaces */
            if (strcmp($4, "lagg") != 0) {
                fprintf(stderr, "Error: laggport operations only valid for lagg interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "laggport", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $8, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER PEER IDENTIFIER {
        if (current_command) {
            /* Only allow peer operations for epair interfaces */
            if (strcmp($4, "epair") != 0) {
                fprintf(stderr, "Error: peer operations only valid for epair interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "peer", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $8, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER PEER IDENTIFIER vrf_assignment {
        if (current_command) {
            /* Only allow peer operations for epair interfaces */
            if (strcmp($4, "epair") != 0) {
                fprintf(stderr, "Error: peer operations only valid for epair interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "peer", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $8, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], "vrf", sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $9, sizeof(current_command->args[5]));
            current_command->arg_count = 6;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER PEER IDENTIFIER address_assignment {
        if (current_command) {
            /* Only allow peer operations for epair interfaces */
            if (strcmp($4, "epair") != 0) {
                fprintf(stderr, "Error: peer operations only valid for epair interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "peer", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $8, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], "address", sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $9, sizeof(current_command->args[5]));
            current_command->arg_count = 6;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER VLANDEV IDENTIFIER {
        if (current_command) {
            /* Only allow vlandev operations for vlan interfaces */
            if (strcmp($4, "vlan") != 0) {
                fprintf(stderr, "Error: vlandev operations only valid for vlan interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "vlandev", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $8, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER VLANPROTO IDENTIFIER {
        if (current_command) {
            /* Only allow vlanproto operations for vlan interfaces */
            if (strcmp($4, "vlan") != 0) {
                fprintf(stderr, "Error: vlanproto operations only valid for vlan interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "vlanproto", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $8, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER TUNNEL address_family LOCAL IPV4_ADDRESS REMOTE IPV4_ADDRESS {
        if (current_command) {
            /* Only allow tunnel operations for gif interfaces */
            if (strcmp($4, "gif") != 0) {
                fprintf(stderr, "Error: tunnel operations only valid for gif interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "tunnel", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], $9, sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $11, sizeof(current_command->args[5]));
            current_command->arg_count = 6;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER TUNNEL address_family LOCAL IPV6_ADDRESS REMOTE IPV6_ADDRESS {
        if (current_command) {
            /* Only allow tunnel operations for gif interfaces */
            if (strcmp($4, "gif") != 0) {
                fprintf(stderr, "Error: tunnel operations only valid for gif interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "tunnel", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], $9, sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $11, sizeof(current_command->args[5]));
            current_command->arg_count = 6;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER TUNNELVRF ID_TOKEN NUMBER {
        if (current_command) {
            /* Only allow tunnelvrf operations for gif interfaces */
            if (strcmp($4, "gif") != 0) {
                fprintf(stderr, "Error: tunnelvrf operations only valid for gif interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "tunnelvrf", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $8, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER LAYER2 VLAN ID_TOKEN NUMBER {
        if (current_command) {
            /* Only allow layer2 vlan operations for vlan interfaces */
            if (strcmp($4, "vlan") != 0) {
                fprintf(stderr, "Error: layer2 vlan operations only valid for vlan interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "layer2", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $9, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER VXLANID ID_TOKEN NUMBER {
        if (current_command) {
            /* Only allow vxlanid operations for vxlan interfaces */
            if (strcmp($4, "vxlan") != 0) {
                fprintf(stderr, "Error: vxlanid operations only valid for vxlan interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "vxlanid", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $8, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER TUNNEL address_family VXLANLOCAL IPV4_ADDRESS VXLANREMOTE IPV4_ADDRESS {
        if (current_command) {
            /* Only allow tunnel operations for vxlan interfaces */
            if (strcmp($4, "vxlan") != 0) {
                fprintf(stderr, "Error: tunnel operations only valid for vxlan interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "tunnel", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], $9, sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $11, sizeof(current_command->args[5]));
            current_command->arg_count = 6;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER TUNNEL address_family VXLANLOCAL IPV6_ADDRESS VXLANREMOTE IPV6_ADDRESS {
        if (current_command) {
            /* Only allow tunnel operations for vxlan interfaces */
            if (strcmp($4, "vxlan") != 0) {
                fprintf(stderr, "Error: tunnel operations only valid for vxlan interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "tunnel", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], $9, sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $11, sizeof(current_command->args[5]));
            current_command->arg_count = 6;
        }
    }
    | SET INTERFACE TYPE interface_type NAME IDENTIFIER VXLANDEV IDENTIFIER {
        if (current_command) {
            /* Only allow vxlandev operations for vxlan interfaces */
            if (strcmp($4, "vxlan") != 0) {
                fprintf(stderr, "Error: vxlandev operations only valid for vxlan interfaces\n");
                return -1;
            }
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "vxlandev", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $8, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET VRF IDENTIFIER address_family STATIC CIDR_ADDRESS route_action {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], $6, sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET VRF IDENTIFIER address_family STATIC HOST IPV4_ADDRESS GATEWAY IPV4_ADDRESS {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "host", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], "gateway", sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $9, sizeof(current_command->args[5]));
            current_command->arg_count = 6;
        }
    }
    | SET VRF IDENTIFIER address_family STATIC HOST IPV4_ADDRESS GATEWAY IPV4_ADDRESS IFACE IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "host", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], "gateway", sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $9, sizeof(current_command->args[5]));
            strlcpy(current_command->args[6], "iface", sizeof(current_command->args[6]));
            strlcpy(current_command->args[7], $11, sizeof(current_command->args[7]));
            current_command->arg_count = 8;
        }
    }
    | SET VRF IDENTIFIER address_family STATIC HOST IPV6_ADDRESS GATEWAY IPV6_ADDRESS {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "host", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], "gateway", sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $9, sizeof(current_command->args[5]));
            current_command->arg_count = 6;
        }
    }
    | SET VRF IDENTIFIER address_family STATIC HOST IPV6_ADDRESS GATEWAY IPV6_ADDRESS IFACE IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "host", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], "gateway", sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $9, sizeof(current_command->args[5]));
            strlcpy(current_command->args[6], "iface", sizeof(current_command->args[6]));
            strlcpy(current_command->args[7], $11, sizeof(current_command->args[7]));
            current_command->arg_count = 8;
        }
    }
    | SET VRF IDENTIFIER address_family STATIC CIDR_ADDRESS IFACE IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "network", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $6, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], "iface", sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $8, sizeof(current_command->args[5]));
            current_command->arg_count = 6;
        }
    }
    | SET VRF IDENTIFIER address_family STATIC CIDR_ADDRESS GATEWAY IPV4_ADDRESS {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "network", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $6, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], "gateway", sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $8, sizeof(current_command->args[5]));
            current_command->arg_count = 6;
        }
    }
    | SET VRF IDENTIFIER address_family STATIC CIDR_ADDRESS GATEWAY IPV4_ADDRESS IFACE IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "network", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $6, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], "gateway", sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $8, sizeof(current_command->args[5]));
            strlcpy(current_command->args[6], "iface", sizeof(current_command->args[6]));
            strlcpy(current_command->args[7], $10, sizeof(current_command->args[7]));
            current_command->arg_count = 8;
        }
    }
    | SET VRF IDENTIFIER address_family STATIC CIDR_ADDRESS GATEWAY IPV6_ADDRESS {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "network", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $6, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], "gateway", sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $8, sizeof(current_command->args[5]));
            current_command->arg_count = 6;
        }
    }
    | SET VRF IDENTIFIER address_family STATIC CIDR_ADDRESS GATEWAY IPV6_ADDRESS IFACE IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "network", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $6, sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], "gateway", sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $8, sizeof(current_command->args[5]));
            strlcpy(current_command->args[6], "iface", sizeof(current_command->args[6]));
            strlcpy(current_command->args[7], $10, sizeof(current_command->args[7]));
            current_command->arg_count = 8;
        }
    }
    ;

/* Show commands */
show_command
    : SHOW VRF ID_TOKEN NUMBER PROTOCOL STATIC {
        if (current_command) {
            current_command->type = CMD_SHOW;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], "vrf", sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], "id", sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], $4, sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], "protocol", sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], "static", sizeof(current_command->args[4]));
            current_command->arg_count = 5;
        }
    }
    | SHOW VRF ID_TOKEN NUMBER PROTOCOL STATIC address_family {
        if (current_command) {
            current_command->type = CMD_SHOW;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], "vrf", sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], "id", sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], $4, sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], "protocol", sizeof(current_command->args[3]));
            strlcpy(current_command->args[4], "static", sizeof(current_command->args[4]));
            strlcpy(current_command->args[5], $7, sizeof(current_command->args[5]));
            current_command->arg_count = 6;
        }
    }
    | SHOW VRF {
        if (current_command) {
            current_command->type = CMD_SHOW;
            current_command->object = OBJ_VRF;
            current_command->arg_count = 0;
        }
    }
    | SHOW VRF IDENTIFIER STATIC address_family {
        if (current_command) {
            current_command->type = CMD_SHOW;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $5, sizeof(current_command->args[1]));
            current_command->arg_count = 2;
        }
    }
    | SHOW ROUTE PROTOCOL IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_SHOW;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], "route", sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], "protocol", sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], $4, sizeof(current_command->args[2]));
            current_command->arg_count = 3;
        }
    }
    | SHOW INTERFACE GROUP IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_SHOW;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], "group", sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            current_command->arg_count = 2;
        }
    }
    | SHOW INTERFACE NAME IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_SHOW;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], "name", sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            current_command->arg_count = 2;
        }
    }
    | SHOW INTERFACE TYPE interface_type {
        if (current_command) {
            current_command->type = CMD_SHOW;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], "type", sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            current_command->arg_count = 2;
        }
    }
    | SHOW INTERFACE {
        if (current_command) {
            current_command->type = CMD_SHOW;
            current_command->object = OBJ_INTERFACE;
            current_command->arg_count = 0;
        }
    }
    ;

/* Delete commands */
delete_command
    : DELETE VRF IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_DELETE;
            current_command->object = OBJ_VRF;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            current_command->arg_count = 1;
        }
    }
    | DELETE VRF IDENTIFIER address_family {
        if (current_command) {
            current_command->type = CMD_DELETE;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            current_command->arg_count = 2;
        }
    }
    | DELETE INTERFACE NAME IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_DELETE;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], "name", sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            current_command->arg_count = 2;
        }
    }
    | DELETE INTERFACE NAME IDENTIFIER ADDRESS address_family {
        if (current_command) {
            current_command->type = CMD_DELETE;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], "name", sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "address", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $6, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | DELETE INTERFACE NAME IDENTIFIER MEMBER {
        if (current_command) {
            current_command->type = CMD_DELETE;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], "name", sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "member", sizeof(current_command->args[2]));
            current_command->arg_count = 3;
        }
    }
    | DELETE INTERFACE NAME IDENTIFIER MEMBER IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_DELETE;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], "name", sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "member", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $6, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | DELETE VRF IDENTIFIER address_family STATIC CIDR_ADDRESS {
        if (current_command) {
            current_command->type = CMD_DELETE;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], $6, sizeof(current_command->args[2]));
            current_command->arg_count = 3;
        }
    }
    | DELETE VRF IDENTIFIER address_family STATIC HOST IPV4_ADDRESS {
        if (current_command) {
            current_command->type = CMD_DELETE;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "host", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | DELETE VRF IDENTIFIER address_family STATIC HOST IPV6_ADDRESS {
        if (current_command) {
            current_command->type = CMD_DELETE;
            current_command->object = OBJ_ROUTE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "host", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $7, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    ;

/* Exit command */
exit_command
    : EXIT {
        if (current_command) {
            current_command->type = CMD_EXIT;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    | QUIT {
        if (current_command) {
            current_command->type = CMD_EXIT;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    ;

/* Help command */
help_command
    : HELP {
        if (current_command) {
            current_command->type = CMD_HELP;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    | QUESTION_MARK {
        if (current_command) {
            current_command->type = CMD_HELP;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    ;

/* Clear command */
clear_command
    : CLEAR {
        if (current_command) {
            current_command->type = CMD_CLEAR;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    ;

/* Commit command */
commit_command
    : COMMIT {
        if (current_command) {
            current_command->type = CMD_COMMIT;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    ;

/* Rollback command */
rollback_command
    : ROLLBACK {
        if (current_command) {
            current_command->type = CMD_ROLLBACK;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    ;

/* Assignment rules */
vrf_assignment
    : VRF IDENTIFIER
    ;

address_assignment
    : ADDRESS address_family IPV4_ADDRESS
    | ADDRESS address_family IPV6_ADDRESS
    | ADDRESS address_family CIDR_ADDRESS
    ;

mtu_assignment
    : MTU NUMBER
    ;

group_assignment
    : GROUP IDENTIFIER
    ;

route_action
    : GATEWAY IPV4_ADDRESS
    | GATEWAY IPV6_ADDRESS
    | IFACE IDENTIFIER
    | GATEWAY IPV4_ADDRESS IFACE IDENTIFIER
    | GATEWAY IPV6_ADDRESS IFACE IDENTIFIER
    ;

/* Interface type rules */
interface_type
    : ETHERNET { $$ = "ethernet"; }
    | LOOPBACK { $$ = "loopback"; }
    | BRIDGE { $$ = "bridge"; }
    | VLAN { $$ = "vlan"; }
    | GIF { $$ = "gif"; }
    | EPAIR { $$ = "epair"; }
    | LAGG { $$ = "lagg"; }
    | TAP { $$ = "tap"; }
    | VXLAN { $$ = "vxlan"; }
    | WLAN { $$ = "wlan"; }
    ;

/* Address family rules */
address_family
    : INET { $$ = "inet"; }
    | INET6 { $$ = "inet6"; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
    parse_error = 1;
} 