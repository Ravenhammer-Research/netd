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
#include <arpa/inet.h>

extern int yylineno;
extern int yyparse(void);

/* Function declarations */
int yylex(void);
void yyerror(const char *s);

/* Token definitions - these should match parser.y */
#define SET 1
#define SHOW 2
#define DELETE 3
#define COMMIT 4
#define SAVE 5
#define VRF 6
#define INTERFACE 7
#define ROUTE 8
#define ETHERNET 9
#define WIRELESS80211 10
#define EPAIR 11
#define GIF 12
#define GRE 13
#define LAGG 14
#define LO 15
#define OVPN 16
#define TUN 17
#define TAP 18
#define VLAN 19
#define VXLAN 20
#define INET 21
#define INET6 22
#define TABLE 23
#define STATIC 24
#define ADDRESS 25
#define MTU 26
#define GROUP 27
#define REJECT_TOKEN 28
#define BLACKHOLE 29
#define IPV4_ADDRESS 30
#define IPV6_ADDRESS 31
#define CIDR_ADDRESS 32
#define IDENTIFIER 33
#define NUMBER 34

/* YYSTYPE declaration for lexer values */
typedef union {
    char *string;
    int number;
} YYSTYPE;
extern YYSTYPE yylval;

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

"set"       { return SET; }
"show"      { return SHOW; }
"delete"    { return DELETE; }
"commit"    { return COMMIT; }
"save"      { return SAVE; }

"vrf"       { return VRF; }
"interface" { return INTERFACE; }
"route"     { return ROUTE; }

"ethernet"      { return ETHERNET; }
"wireless80211" { return WIRELESS80211; }
"epair"         { return EPAIR; }
"gif"           { return GIF; }
"gre"           { return GRE; }
"lagg"          { return LAGG; }
"lo"            { return LO; }
"ovpn"          { return OVPN; }
"tun"           { return TUN; }
"tap"           { return TAP; }
"vlan"          { return VLAN; }
"vxlan"         { return VXLAN; }

"inet"      { return INET; }
"inet6"     { return INET6; }

"table"     { return TABLE; }
"static"    { return STATIC; }
"address"   { return ADDRESS; }
"mtu"       { return MTU; }
"group"     { return GROUP; }
"reject"    { return REJECT_TOKEN; }
"blackhole" { return BLACKHOLE; }

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