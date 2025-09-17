%{
/*
 * Copyright (c) 2024 Paige Thompson / Ravenhammer Research (paige@paige.bio)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

// Declare custom input function
extern int custom_input(char* buf, int max_size);

// Use custom input function instead of default file input
#define YY_INPUT(buf, result, max_size) \
    result = custom_input(buf, max_size)

%}

%option noyywrap
%option yylineno

%%

"set"                    { return SET; }
"delete"                 { return DELETE; }
"show"                   { return SHOW; }
"commit"                 { return COMMIT; }
"edit"                   { return EDIT; }
"quit"                   { printf("LEXER: Found QUIT token\n"); return QUIT; }
"exit"                   { printf("LEXER: Found QUIT token (exit)\n"); return QUIT; }
"help"                   { printf("LEXER: Found HELP token\n"); return HELP; }

"interfaces"             { return INTERFACES; }
"interface"              { return INTERFACE; }
"routing-instances"      { return ROUTING_INSTANCES; }
"routing-options"        { return ROUTING_OPTIONS; }

"unit"                   { return UNIT; }
"family"                 { return FAMILY; }
"inet"                   { return INET; }
"address"                { return ADDRESS; }
"description"            { return DESCRIPTION; }
"encapsulation"          { return ENCAPSULATION; }
"vlan-id"                { return VLAN_ID; }
"speed"                  { return SPEED; }
"vlan-tagging"           { return VLAN_TAGGING; }
"ethernet-vlan"          { return ETHERNET_VLAN; }

"instance-type"          { return INSTANCE_TYPE; }
"virtual-router"         { return VIRTUAL_ROUTER; }
"vrf"                    { return VRF; }
"vrf-target"             { return VRF_TARGET; }
"vrf-table-label"        { return VRF_TABLE_LABEL; }
"static"                 { return STATIC; }
"route"                  { return ROUTE; }
"next-hop"               { return NEXT_HOP; }
"table"                  { return TABLE; }
"protocol"               { return PROTOCOL; }
"instance"               { return INSTANCE; }

"ospf"                   { return OSPF; }
"bgp"                    { return BGP; }

"version"                { return VERSION; }
"configuration"          { return CONFIGURATION; }
"system"                 { return SYSTEM; }
"uptime"                 { return UPTIME; }
"chassis"                { return CHASSIS; }
"log"                    { return LOG; }
"messages"               { return MESSAGES; }
"neighbor"               { return NEIGHBOR; }
"neighbors"              { return NEIGHBORS; }
"summary"                { return SUMMARY; }
"arp"                    { return ARP; }
"no-resolve"             { return NO_RESOLVE; }
"protocols"              { return PROTOCOLS; }
"ipv6"                   { return IPV6; }

"terse"                  { return TERSE; }
"brief"                  { return BRIEF; }
"detail"                 { return DETAIL; }
"extensive"              { return EXTENSIVE; }
"display"                { return DISPLAY; }

"all"                    { return ALL; }

[a-z]+-[0-9]+\/[0-9]+\/[0-9]+ { yylval.string = strdup(yytext); return INTERFACE_NAME; }

[0-9]+                   { yylval.number = atoi(yytext); return NUMBER; }

[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\/[0-9]+ { yylval.string = strdup(yytext); return IP_CIDR; }
[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+ { yylval.string = strdup(yytext); return IP_ADDRESS; }

[A-Za-z_][A-Za-z0-9_]*  { yylval.string = strdup(yytext); return IDENTIFIER; }

\"[^"]*\"               { yylval.string = strdup(yytext); return STRING; }

[0-9]+g                  { yylval.string = strdup(yytext); return SPEED_VALUE; }

\[                      { return LBRACKET; }
\]                      { return RBRACKET; }
\.                      { return DOT; }

[ \t\n]                 ;

.                       { printf("LEXER: Unknown character %s\n", yytext); }

%%
