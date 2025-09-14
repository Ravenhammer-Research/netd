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

#include <client/include/processor/actions.hpp>
#include <client/include/processor/command.hpp>

// Global variables for parser communication
static bool g_parse_success = false;

// C linkage functions for yacc parser
extern "C" {
    // Command type actions
    void set_command_action() {
        netd::client::processor::ParserActions::setCommandType(netd::client::processor::CommandType::SET_CMD);
        g_parse_success = true;
    }
    
    void delete_command_action() {
        netd::client::processor::ParserActions::setCommandType(netd::client::processor::CommandType::DELETE_CMD);
        g_parse_success = true;
    }
    
    void show_command_action() {
        netd::client::processor::ParserActions::setCommandType(netd::client::processor::CommandType::SHOW_CMD);
        g_parse_success = true;
    }
    
    void commit_command_action() {
        netd::client::processor::ParserActions::setCommandType(netd::client::processor::CommandType::COMMIT_CMD);
        g_parse_success = true;
    }
    
    void edit_command_action() {
        netd::client::processor::ParserActions::setCommandType(netd::client::processor::CommandType::EDIT_CMD);
        g_parse_success = true;
    }
    
    // Interface actions
    void set_interface_name(const char* name) {
        netd::client::processor::ParserActions::setInterfaceName(name);
    }
    
    void set_unit_number(int unit) {
        netd::client::processor::ParserActions::setUnitNumber(unit);
    }
    
    void set_ip_address(const char* ip) {
        netd::client::processor::ParserActions::setIpAddress(ip);
    }
    
    void set_description(const char* desc) {
        netd::client::processor::ParserActions::setDescription(desc);
    }
    
    void set_vlan_id(int vlan) {
        netd::client::processor::ParserActions::setVlanId(vlan);
    }
    
    void set_speed_value(const char* speed) {
        netd::client::processor::ParserActions::setSpeedValue(speed);
    }
    
    void set_identifier(const char* id) {
        netd::client::processor::ParserActions::setIdentifier(id);
    }
    
    void set_string_value(const char* str) {
        netd::client::processor::ParserActions::setStringValue(str);
    }
    
    void set_vlan_tagging() {
        netd::client::processor::ParserActions::setVlanTagging(true);
    }
    
    // Display mode actions
    void set_brief_mode() {
        netd::client::processor::ParserActions::setDisplayMode(netd::client::processor::DisplayMode::BRIEF_MODE);
    }
    
    void set_detail_mode() {
        netd::client::processor::ParserActions::setDisplayMode(netd::client::processor::DisplayMode::DETAIL_MODE);
    }
    
    void set_extensive_mode() {
        netd::client::processor::ParserActions::setDisplayMode(netd::client::processor::DisplayMode::EXTENSIVE_MODE);
    }
    
    void set_terse_mode() {
        netd::client::processor::ParserActions::setDisplayMode(netd::client::processor::DisplayMode::TERSE_MODE);
    }
    
    // Routing instance actions
    void set_instance_type(const char* type) {
        netd::client::processor::ParserActions::setInstanceType(type);
    }
    
    void set_virtual_router() {
        netd::client::processor::ParserActions::setVirtualRouter();
    }
    
    void set_vrf() {
        netd::client::processor::ParserActions::setVrf();
    }
    
    void set_vrf_target(const char* target) {
        netd::client::processor::ParserActions::setVrfTarget(target);
    }
    
    void set_vrf_table_label() {
        netd::client::processor::ParserActions::setVrfTableLabel();
    }
    
    // Routing options actions
    void set_static_route(const char* route) {
        netd::client::processor::ParserActions::setStaticRoute(route);
    }
    
    void set_next_hop(const char* hop) {
        netd::client::processor::ParserActions::setNextHop(hop);
    }
    
    void set_table(const char* table) {
        netd::client::processor::ParserActions::setTable(table);
    }
    
    void set_protocol(const char* protocol) {
        netd::client::processor::ParserActions::setProtocol(protocol);
    }
    
    // Show command actions
    void set_version() {
        netd::client::processor::ParserActions::setVersion();
    }
    
    void set_configuration() {
        netd::client::processor::ParserActions::setConfiguration();
    }
    
    void set_system() {
        netd::client::processor::ParserActions::setSystem();
    }
    
    void set_uptime() {
        netd::client::processor::ParserActions::setUpTime();
    }
    
    void set_chassis() {
        netd::client::processor::ParserActions::setChassis();
    }
    
    void set_log() {
        netd::client::processor::ParserActions::setLog();
    }
    
    void set_messages() {
        netd::client::processor::ParserActions::setMessages();
    }
    
    void set_neighbor() {
        netd::client::processor::ParserActions::setNeighbor();
    }
    
    void set_neighbors() {
        netd::client::processor::ParserActions::setNeighbors();
    }
    
    void set_summary() {
        netd::client::processor::ParserActions::setSummary();
    }
    
    void set_arp() {
        netd::client::processor::ParserActions::setArp();
    }
    
    void set_no_resolve() {
        netd::client::processor::ParserActions::setNoResolve();
    }
    
    void set_protocols() {
        netd::client::processor::ParserActions::setProtocols();
    }
    
    void set_ipv6() {
        netd::client::processor::ParserActions::setIpv6();
    }
    
    void set_all() {
        netd::client::processor::ParserActions::setAll();
    }
    
    // Protocol actions
    void set_ospf() {
        netd::client::processor::ParserActions::setOspf();
    }
    
    void set_bgp() {
        netd::client::processor::ParserActions::setBgp();
    }
    
    void set_static() {
        netd::client::processor::ParserActions::setStatic();
    }
    
    void set_route() {
        netd::client::processor::ParserActions::setRoute();
    }
    
    // Utility actions
    void set_number(int num) {
        netd::client::processor::ParserActions::setNumber(num);
    }
    
    void set_ip_cidr(const char* cidr) {
        netd::client::processor::ParserActions::setIpCidr(cidr);
    }
    
    void set_ip_address(const char* addr) {
        netd::client::processor::ParserActions::setIpAddress(addr);
    }
    
    void set_speed_value(const char* speed) {
        netd::client::processor::ParserActions::setSpeedValue(speed);
    }
    
    // Context actions (mostly for grammar structure)
    void set_interfaces() {
        netd::client::processor::ParserActions::setInterfaces();
    }
    
    void set_routing_instances() {
        netd::client::processor::ParserActions::setRoutingInstances();
    }
    
    void set_routing_options() {
        netd::client::processor::ParserActions::setRoutingOptions();
    }
    
    void set_unit() {
        netd::client::processor::ParserActions::setUnit();
    }
    
    void set_family() {
        netd::client::processor::ParserActions::setFamily();
    }
    
    void set_inet() {
        netd::client::processor::ParserActions::setInet();
    }
    
    void set_address() {
        netd::client::processor::ParserActions::setAddress();
    }
    
    void set_encapsulation() {
        netd::client::processor::ParserActions::setEncapsulation();
    }
    
    void set_ethernet_vlan() {
        netd::client::processor::ParserActions::setEthernetVlan();
    }
    
    void set_interface() {
        netd::client::processor::ParserActions::setInterface();
    }
    
    void set_lbracket() {
        netd::client::processor::ParserActions::setLbracket();
    }
    
    void set_rbracket() {
        netd::client::processor::ParserActions::setRbracket();
    }
    
    void set_dot() {
        netd::client::processor::ParserActions::setDot();
    }
    
    // Parser state functions
    void reset_parser() {
        netd::client::processor::ParserActions::reset();
        g_parse_success = false;
    }
    
    bool get_parse_success() {
        return g_parse_success;
    }
    
    void set_parse_success(bool success) {
        g_parse_success = success;
    }
}
