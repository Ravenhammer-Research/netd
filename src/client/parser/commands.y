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
#include "tokens.h"
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

%%

command
    : set_command
    | show_command
    | delete_command
    | commit_command
    | save_command
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

%% 