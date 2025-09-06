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

extern int yylex();
extern int yyparse();
extern FILE* yyin;

void yyerror(const char* s) {
    fprintf(stderr, "Parse error: %s\n", s);
}

%}

/* Tokens */
%token SET DELETE SHOW COMMIT EDIT
%token INTERFACES ROUTING_INSTANCES ROUTING_OPTIONS
%token UNIT FAMILY INET ADDRESS DESCRIPTION ENCAPSULATION
%token VLAN_ID SPEED VLAN_TAGGING ETHERNET_VLAN
%token INSTANCE_TYPE VIRTUAL_ROUTER VRF VRF_TARGET VRF_TABLE_LABEL
%token INTERFACE STATIC ROUTE NEXT_HOP TABLE PROTOCOL INSTANCE
%token OSPF BGP VERSION CONFIGURATION SYSTEM UPTIME CHASSIS
%token LOG MESSAGES NEIGHBOR NEIGHBORS SUMMARY ARP NO_RESOLVE PROTOCOLS IPV6
%token TERSE BRIEF DETAIL EXTENSIVE DISPLAY ALL
%token INTERFACE_NAME NUMBER IP_CIDR IP_ADDRESS IDENTIFIER
%token STRING SPEED_VALUE LBRACKET RBRACKET DOT

%%

/* Main command entry point */
command: set_command
    | delete_command
    | show_command
    | commit_command
    | edit_command
    ;

/* SET commands */
set_command: SET set_config
    ;

set_config: interfaces_set
    | routing_instances_set
    | routing_options_set
    ;

interfaces_set: INTERFACES interface_name unit_set
    | INTERFACES interface_name interface_property_set
    ;

unit_set: UNIT NUMBER unit_property_set
    ;

unit_property_set: family_set
    | description_set
    | encapsulation_set
    | vlan_id_set
    | speed_set
    ;

family_set: FAMILY INET ADDRESS IP_CIDR
    ;

description_set: DESCRIPTION STRING
    ;

encapsulation_set: ENCAPSULATION ETHERNET_VLAN
    ;

vlan_id_set: VLAN_ID NUMBER
    ;

speed_set: SPEED SPEED_VALUE
    ;

interface_property_set: VLAN_TAGGING
    ;

routing_instances_set: ROUTING_INSTANCES IDENTIFIER instance_property_set
    ;

instance_property_set: INSTANCE_TYPE VIRTUAL_ROUTER
    | INSTANCE_TYPE VRF
    | VRF_TARGET STRING
    | VRF_TABLE_LABEL
    | INTERFACE interface_name DOT NUMBER
    ;

routing_options_set: ROUTING_OPTIONS STATIC ROUTE IP_CIDR NEXT_HOP IP_ADDRESS
    ;

/* DELETE commands */
delete_command: DELETE delete_config
    ;

delete_config: interfaces_delete
    | routing_instances_delete
    | routing_options_delete
    ;

interfaces_delete: INTERFACES interface_name
    | INTERFACES interface_name UNIT NUMBER
    | INTERFACES interface_name UNIT NUMBER family_delete
    | INTERFACES interface_name UNIT NUMBER description_delete
    | INTERFACES interface_name UNIT NUMBER encapsulation_delete
    | INTERFACES interface_name UNIT NUMBER vlan_id_delete
    | INTERFACES interface_name UNIT NUMBER speed_delete
    | INTERFACES interface_name interface_property_delete
    ;

family_delete: FAMILY INET ADDRESS IP_CIDR
    ;

description_delete: DESCRIPTION STRING
    ;

encapsulation_delete: ENCAPSULATION ETHERNET_VLAN
    ;

vlan_id_delete: VLAN_ID NUMBER
    ;

speed_delete: SPEED SPEED_VALUE
    ;

interface_property_delete: VLAN_TAGGING
    ;

routing_instances_delete: ROUTING_INSTANCES IDENTIFIER
    | ROUTING_INSTANCES IDENTIFIER INSTANCE_TYPE VIRTUAL_ROUTER
    | ROUTING_INSTANCES IDENTIFIER INSTANCE_TYPE VRF
    | ROUTING_INSTANCES IDENTIFIER VRF_TARGET STRING
    | ROUTING_INSTANCES IDENTIFIER VRF_TABLE_LABEL
    | ROUTING_INSTANCES IDENTIFIER INTERFACE interface_name DOT NUMBER
    ;

routing_options_delete: ROUTING_OPTIONS STATIC ROUTE IP_CIDR NEXT_HOP IP_ADDRESS
    ;

/* SHOW commands */
show_command: SHOW show_config
    ;

show_config: interfaces_show
    | routing_instances_show
    | routing_options_show
    | route_show
    | version_show
    | configuration_show
    | system_show
    | chassis_show
    | log_show
    | ospf_show
    | bgp_show
    | route_instance_show
    | arp_show
    | protocols_show
    | ipv6_neighbors_show
    ;

interfaces_show: INTERFACES interface_name
    | INTERFACES interface_name UNIT NUMBER
    | INTERFACES interface_name UNIT NUMBER family_show
    | INTERFACES interface_name UNIT NUMBER description_show
    | INTERFACES interface_name UNIT NUMBER encapsulation_show
    | INTERFACES interface_name UNIT NUMBER vlan_id_show
    | INTERFACES interface_name UNIT NUMBER speed_show
    | INTERFACES display_option
    | INTERFACES TERSE ROUTING_INSTANCES ALL
    | INTERFACES interface_name ROUTING_INSTANCES ALL
    ;

family_show: FAMILY INET ADDRESS IP_CIDR
    ;

description_show: DESCRIPTION STRING
    ;

encapsulation_show: ENCAPSULATION ETHERNET_VLAN
    ;

vlan_id_show: VLAN_ID NUMBER
    ;

speed_show: SPEED SPEED_VALUE
    ;

routing_instances_show: ROUTING_INSTANCES IDENTIFIER
    | ROUTING_INSTANCES IDENTIFIER INSTANCE
    ;

routing_options_show: ROUTING_OPTIONS STATIC ROUTE IP_CIDR NEXT_HOP IP_ADDRESS
    ;

route_show: ROUTE TABLE IDENTIFIER DOT INET DOT NUMBER PROTOCOL STATIC
    | ROUTE IDENTIFIER
    | ROUTE
    | ROUTE PROTOCOL protocol_name
    | ROUTE display_option
    ;

/* COMMIT command */
commit_command: COMMIT
    ;

/* EDIT command */
edit_command: EDIT INTERFACES interface_name UNIT NUMBER
    | EDIT ROUTING_INSTANCES IDENTIFIER
    ;

/* Version commands */
version_show: VERSION
    | VERSION BRIEF
    ;

/* Configuration commands */
configuration_show: CONFIGURATION
    | CONFIGURATION DISPLAY SET
    | CONFIGURATION DISPLAY DETAIL
    | CONFIGURATION ROUTING_INSTANCES IDENTIFIER DISPLAY SET
    ;

/* System commands */
system_show: SYSTEM UPTIME
    ;

/* Chassis commands */
chassis_show: CHASSIS
    ;

/* Log commands */
log_show: LOG MESSAGES
    ;

/* OSPF commands */
ospf_show: OSPF NEIGHBOR
    ;

/* BGP commands */
bgp_show: BGP SUMMARY
    ;

/* Route instance commands */
route_instance_show: ROUTE INSTANCE
    | ROUTE INSTANCE DETAIL
    ;

/* ARP commands */
arp_show: ARP
    | ARP NO_RESOLVE
    ;

/* Protocols commands */
protocols_show: PROTOCOLS
    ;

/* IPv6 neighbors commands */
ipv6_neighbors_show: IPV6 NEIGHBORS
    ;

/* Display options */
display_option: TERSE
    | BRIEF
    | DETAIL
    | EXTENSIVE
    ;

/* Protocol names */
protocol_name: OSPF
    | BGP
    | STATIC
    ;

/* Interface name */
interface_name: INTERFACE_NAME
    ;

%%
