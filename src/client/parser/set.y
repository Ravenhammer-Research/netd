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
%token <string> LOOPBACK
%token <string> BRIDGE
%token <string> WLAN

/* Precedence declarations */
%left SET
%left VRF INTERFACE ROUTE
%left ETHERNET LOOPBACK BRIDGE VLAN GIF EPAIR LAGG TAP VXLAN WLAN
%left ADDRESS MTU GROUP MEMBER LAGGPROTO

/* Non-terminal type declarations */
%type <string> vrf_assignment
%type <string> address_assignment
%type <string> mtu_assignment
%type <string> group_assignment
%type <string> address_family
%type <string> interface_type

%%

/* Top level rule */
command
    : set_command
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
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $4, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $6, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "laggproto", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $8, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    ;

/* Assignment rules */
vrf_assignment
    : VRF IDENTIFIER { $$ = $2; }
    ;

address_assignment
    : ADDRESS address_family IPV4_ADDRESS { 
        char temp[128];
        snprintf(temp, sizeof(temp), "%s %s", $2, $3);
        $$ = strdup(temp);
    }
    | ADDRESS address_family IPV6_ADDRESS { 
        char temp[128];
        snprintf(temp, sizeof(temp), "%s %s", $2, $3);
        $$ = strdup(temp);
    }
    | ADDRESS address_family CIDR_ADDRESS { 
        char temp[128];
        snprintf(temp, sizeof(temp), "%s %s", $2, $3);
        $$ = strdup(temp);
    }
    ;

mtu_assignment
    : MTU NUMBER { 
        char temp[64];
        snprintf(temp, sizeof(temp), "%s", $2);
        $$ = strdup(temp);
    }
    ;

group_assignment
    : GROUP IDENTIFIER { $$ = $2; }
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