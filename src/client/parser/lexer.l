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
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

extern int yylineno;
extern int yyparse(void);

/* Function declarations */
int yylex(void);
void yyerror(const char *s);

%}

%option noyywrap
%option yylineno
%option noinput
%option nounput

DIGIT       [0-9]
LETTER      [a-zA-Z]
HEX         [0-9a-fA-F]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT}|[-_])*
NUMBER      {DIGIT}+
IPV4_OCTET  {DIGIT}{1,3}
IPV6_HEX    {HEX}{1,4}

%%

"set"       { yylval.cmd_type = CMD_SET; return SET; }
"show"      { yylval.cmd_type = CMD_SHOW; return SHOW; }
"delete"    { yylval.cmd_type = CMD_DELETE; return DELETE; }
"commit"    { yylval.cmd_type = CMD_COMMIT; return COMMIT; }
"save"      { yylval.cmd_type = CMD_SAVE; return SAVE; }

"vrf"       { yylval.string = strdup("vrf"); return VRF; }
"interface" { yylval.obj_type = OBJ_INTERFACE; return INTERFACE; }
"route"     { yylval.obj_type = OBJ_ROUTE; return ROUTE; }

"type"      { yylval.string = strdup("type"); return TYPE; }
"name"      { yylval.string = strdup("name"); return NAME; }

"ethernet"      { yylval.if_type = IF_TYPE_ETHERNET; return ETHERNET; }
"wireless80211" { yylval.if_type = IF_TYPE_WIRELESS; return WIRELESS80211; }
"epair"         { yylval.if_type = IF_TYPE_EPAIR; return EPAIR; }
"gif"           { yylval.if_type = IF_TYPE_GIF; return GIF; }
"gre"           { yylval.if_type = IF_TYPE_GRE; return GRE; }
"lagg"          { yylval.if_type = IF_TYPE_LAGG; return LAGG; }
"lo"            { yylval.if_type = IF_TYPE_LOOPBACK; return LO; }
"ovpn"          { yylval.if_type = IF_TYPE_OVPN; return OVPN; }
"tun"           { yylval.if_type = IF_TYPE_TUN; return TUN; }
"tap"           { yylval.if_type = IF_TYPE_TAP; return TAP; }
"vlan"          { yylval.if_type = IF_TYPE_VLAN; return VLAN; }
"vxlan"         { yylval.if_type = IF_TYPE_VXLAN; return VXLAN; }
"bridge"        { yylval.if_type = IF_TYPE_BRIDGE; return IDENTIFIER; }
"vxlanid"          { yylval.string = strdup("vxlanid"); return VXLANID; }
"vxlanlocal"       { yylval.string = strdup("vxlanlocal"); return VXLANLOCAL; }
"vxlanremote"      { yylval.string = strdup("vxlanremote"); return VXLANREMOTE; }
"vxlandev"         { yylval.string = strdup("vxlandev"); return VXLANDEV; }

"inet"      { yylval.family = AF_INET; return INET; }
"inet6"     { yylval.family = AF_INET6; return INET6; }

"table"     { yylval.string = strdup("table"); return TABLE; }
"static"    { yylval.string = strdup("static"); return STATIC; }
"address"   { yylval.string = strdup("address"); return ADDRESS; }
"mtu"       { yylval.string = strdup("mtu"); return MTU; }
"group"     { yylval.string = strdup("group"); return GROUP; }
"protocol"  { return PROTOCOL; }
"reject"    { yylval.string = strdup("reject"); return REJECT_TOKEN; }
"blackhole" { yylval.string = strdup("blackhole"); return BLACKHOLE; }
"id"        { yylval.string = strdup("id"); return ID_TOKEN; }
"member"    { yylval.string = strdup("member"); return MEMBER; }
"laggproto" { yylval.string = strdup("laggproto"); return LAGGPROTO; }
"laggport"  { yylval.string = strdup("laggport"); return LAGGPORT; }
"peer"      { yylval.string = strdup("peer"); return PEER; }
"vlandev"   { yylval.string = strdup("vlandev"); return VLANDEV; }
"vlanproto" { yylval.string = strdup("vlanproto"); return VLANPROTO; }
"tunnel"    { yylval.string = strdup("tunnel"); return TUNNEL; }
"local"     { yylval.string = strdup("local"); return LOCAL; }
"remote"    { yylval.string = strdup("remote"); return REMOTE; }
"tunnelvrf" { yylval.string = strdup("tunnelvrf"); return TUNNELVRF; }
"layer2"    { yylval.string = strdup("layer2"); return LAYER2; }
"host"      { yylval.string = strdup("host"); return HOST; }
"gateway"   { yylval.string = strdup("gateway"); return GATEWAY; }
"iface"     { yylval.string = strdup("iface"); return IFACE; }

{IPV4_OCTET}\.{IPV4_OCTET}\.{IPV4_OCTET}\.{IPV4_OCTET} {
    /* IPv4 address */
    yylval.string = strdup(yytext);
    return IPV4_ADDRESS;
}

{IPV4_OCTET}\.{IPV4_OCTET}\.{IPV4_OCTET}\.{IPV4_OCTET}\/{NUMBER} {
    /* IPv4 CIDR address */
    yylval.string = strdup(yytext);
    return CIDR_ADDRESS;
}

({IPV6_HEX}:){7}{IPV6_HEX} {
    /* IPv6 address */
    yylval.string = strdup(yytext);
    return IPV6_ADDRESS;
}

({IPV6_HEX}:){7}{IPV6_HEX}\/{NUMBER} {
    /* IPv6 CIDR address */
    yylval.string = strdup(yytext);
    return CIDR_ADDRESS;
}

{NUMBER} {
    /* Number */
    yylval.string = strdup(yytext);
    return NUMBER;
}

{IDENTIFIER} {
    /* Identifier */
    yylval.string = strdup(yytext);
    return IDENTIFIER;
}

[ \t]+ {
    /* Skip whitespace */
}

\n {
    /* End of line */
    return 0;
}

. {
    /* Any other character */
    fprintf(stderr, "Unexpected character: %s\n", yytext);
}

%% 