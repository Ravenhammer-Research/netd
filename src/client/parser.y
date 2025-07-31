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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex(void);
extern int yyparse(void);
extern FILE *yyin;
extern char *yytext;
extern int yylineno;

/* Global variables for parser */
command_t *current_command = NULL;
int parse_error = 0;

/* Function declarations */
void yyerror(const char *s);
int yylex(void);

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

/* Non-terminal type declarations */
%type <string> vrf_assignment
%type <string> address_assignment
%type <string> mtu_assignment
%type <string> group_assignment
%type <string> address_family
%type <string> interface_type
%type <string> route_action

%token <cmd_type> SET
%token <cmd_type> SHOW
%token <cmd_type> DELETE
%token <cmd_type> COMMIT
%token <cmd_type> SAVE

%token <obj_type> VRF
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

%token TABLE
%token STATIC
%token ADDRESS
%token MTU
%token GROUP
%token REJECT_TOKEN
%token BLACKHOLE

%type <string> address_family
%type <string> interface_type
%type <string> route_action

%%

command
    : set_command
    | show_command
    | delete_command
    | commit_command
    | save_command
    ;

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
    | SET INTERFACE interface_type IDENTIFIER vrf_assignment {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "vrf", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $5, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE interface_type IDENTIFIER address_assignment {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "address", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $5, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE interface_type IDENTIFIER mtu_assignment {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "mtu", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $5, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    | SET INTERFACE interface_type IDENTIFIER group_assignment {
        if (current_command) {
            current_command->type = CMD_SET;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "group", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $5, sizeof(current_command->args[3]));
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
    | SET VRF IDENTIFIER address_family STATIC IPV4_ADDRESS IPV4_ADDRESS {
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
    | SET VRF IDENTIFIER address_family STATIC IPV6_ADDRESS IPV6_ADDRESS {
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
    ;

show_command
    : SHOW VRF {
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
    | SHOW INTERFACE {
        if (current_command) {
            current_command->type = CMD_SHOW;
            current_command->object = OBJ_INTERFACE;
            current_command->arg_count = 0;
        }
    }
    | SHOW INTERFACE interface_type {
        if (current_command) {
            current_command->type = CMD_SHOW;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            current_command->arg_count = 1;
        }
    }
    | SHOW INTERFACE interface_type IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_SHOW;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            current_command->arg_count = 2;
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
    ;

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
    | DELETE INTERFACE interface_type IDENTIFIER {
        if (current_command) {
            current_command->type = CMD_DELETE;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            current_command->arg_count = 2;
        }
    }
    | DELETE INTERFACE interface_type IDENTIFIER ADDRESS address_family {
        if (current_command) {
            current_command->type = CMD_DELETE;
            current_command->object = OBJ_INTERFACE;
            strlcpy(current_command->args[0], $3, sizeof(current_command->args[0]));
            strlcpy(current_command->args[1], $4, sizeof(current_command->args[1]));
            strlcpy(current_command->args[2], "address", sizeof(current_command->args[2]));
            strlcpy(current_command->args[3], $6, sizeof(current_command->args[3]));
            current_command->arg_count = 4;
        }
    }
    ;

commit_command
    : COMMIT {
        if (current_command) {
            current_command->type = CMD_COMMIT;
            current_command->object = OBJ_UNKNOWN;
            current_command->arg_count = 0;
        }
    }
    ;

save_command
    : SAVE {
        if (current_command) {
            current_command->type = CMD_SAVE;
            current_command->object = OBJ_UNKNOWN;
            current_command->arg_count = 0;
        }
    }
    ;

vrf_assignment
    : VRF IDENTIFIER { $$ = $2; }
    ;

address_assignment
    : ADDRESS address_family CIDR_ADDRESS { $$ = $3; }
    ;

mtu_assignment
    : MTU NUMBER { $$ = $2; }
    ;

group_assignment
    : GROUP IDENTIFIER { $$ = $2; }
    ;

address_family
    : INET { $$ = "inet"; }
    | INET6 { $$ = "inet6"; }
    ;

interface_type
    : ETHERNET { $$ = "ethernet"; }
    | WIRELESS80211 { $$ = "wireless80211"; }
    | EPAIR { $$ = "epair"; }
    | GIF { $$ = "gif"; }
    | GRE { $$ = "gre"; }
    | LAGG { $$ = "lagg"; }
    | LO { $$ = "lo"; }
    | OVPN { $$ = "ovpn"; }
    | TUN { $$ = "tun"; }
    | TAP { $$ = "tap"; }
    | VLAN { $$ = "vlan"; }
    | VXLAN { $$ = "vxlan"; }
    ;

route_action
    : REJECT_TOKEN { $$ = "reject"; }
    | BLACKHOLE { $$ = "blackhole"; }
    | IPV4_ADDRESS { $$ = $1; }
    | IPV6_ADDRESS { $$ = $1; }
    ;

%%

void yyerror(const char *s) {
    parse_error = 1;
    fprintf(stderr, "Parse error: %s\n", s);
} 