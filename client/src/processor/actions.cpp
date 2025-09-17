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
#include <shared/include/logger.hpp>

namespace netd::client::processor {

  // Global command object for parsed commands
  static Command g_parsed_command;

  void ParserActions::setCommandType(CommandType type) {
    g_parsed_command.setCommandType(type);
  }

  void ParserActions::setInterfaceName(const char *name) {
    g_parsed_command.setInterfaceName(name);
  }

  void ParserActions::setUnitNumber(int unit) {
    g_parsed_command.setUnitNumber(unit);
  }

  void ParserActions::setIpAddress(const char *ip) {
    g_parsed_command.setIpAddress(ip);
  }

  void ParserActions::setDescription(const char *desc) {
    g_parsed_command.setDescription(desc);
  }

  void ParserActions::setVlanId(int vlan) { g_parsed_command.setVlanId(vlan); }

  void ParserActions::setSpeedValue(const char *speed) {
    g_parsed_command.setSpeedValue(speed);
  }

  void ParserActions::setIdentifier(const char *id) {
    g_parsed_command.setIdentifier(id);
  }

  void ParserActions::setStringValue(const char *str) {
    g_parsed_command.setStringValue(str);
  }

  void ParserActions::setVlanTagging(bool tagging) {
    g_parsed_command.setVlanTagging(tagging);
  }

  void ParserActions::setDisplayMode(DisplayMode mode) {
    g_parsed_command.setDisplayMode(mode);
  }

  // Routing instance actions
  void ParserActions::setInstanceType(const char *type) {
    // Store instance type in string value for now
    g_parsed_command.setStringValue(type);
  }

  void ParserActions::setVirtualRouter() {
    g_parsed_command.setStringValue("virtual-router");
  }

  void ParserActions::setVrf() { g_parsed_command.setStringValue("vrf"); }

  void ParserActions::setVrfTarget(const char *target) {
    g_parsed_command.setStringValue(target);
  }

  void ParserActions::setVrfTableLabel() {
    g_parsed_command.setStringValue("vrf-table-label");
  }

  // Routing options actions
  void ParserActions::setStaticRoute(const char *route) {
    g_parsed_command.setStringValue(route);
  }

  void ParserActions::setNextHop(const char *hop) {
    g_parsed_command.setStringValue(hop);
  }

  void ParserActions::setTable(const char *table) {
    g_parsed_command.setStringValue(table);
  }

  void ParserActions::setProtocol(const char *protocol) {
    g_parsed_command.setStringValue(protocol);
  }

  // Show command specific actions
  void ParserActions::setVersion() {
    g_parsed_command.setStringValue("version");
  }

  void ParserActions::setConfiguration() {
    g_parsed_command.setStringValue("configuration");
  }

  void ParserActions::setSystem() { g_parsed_command.setStringValue("system"); }

  void ParserActions::setUpTime() { g_parsed_command.setStringValue("uptime"); }

  void ParserActions::setChassis() {
    g_parsed_command.setStringValue("chassis");
  }

  void ParserActions::setLog() { g_parsed_command.setStringValue("log"); }

  void ParserActions::setMessages() {
    g_parsed_command.setStringValue("messages");
  }

  void ParserActions::setNeighbor() {
    g_parsed_command.setStringValue("neighbor");
  }

  void ParserActions::setNeighbors() {
    g_parsed_command.setStringValue("neighbors");
  }

  void ParserActions::setSummary() {
    g_parsed_command.setStringValue("summary");
  }

  void ParserActions::setArp() { g_parsed_command.setStringValue("arp"); }

  void ParserActions::setNoResolve() {
    g_parsed_command.setStringValue("no-resolve");
  }

  void ParserActions::setProtocols() {
    g_parsed_command.setStringValue("protocols");
  }

  void ParserActions::setIpv6() { g_parsed_command.setStringValue("ipv6"); }

  void ParserActions::setAll() { g_parsed_command.setStringValue("all"); }

  // Interface actions - these are mostly context setters
  void ParserActions::setInterfaces() {
    // Context: we're working with interfaces
  }

  void ParserActions::setRoutingInstances() {
    // Context: we're working with routing instances
  }

  void ParserActions::setRoutingOptions() {
    // Context: we're working with routing options
  }

  void ParserActions::setUnit() {
    // Context: we're working with a unit
  }

  void ParserActions::setFamily() {
    // Context: we're working with a family
  }

  void ParserActions::setInet() {
    // Context: we're working with inet family
  }

  void ParserActions::setAddress() {
    // Context: we're working with an address
  }

  void ParserActions::setEncapsulation() {
    // Context: we're working with encapsulation
  }

  void ParserActions::setEthernetVlan() {
    // Context: we're working with ethernet-vlan encapsulation
  }

  void ParserActions::setInterface() {
    // Context: we're working with an interface
  }

  // Protocol actions
  void ParserActions::setOspf() { g_parsed_command.setStringValue("ospf"); }

  void ParserActions::setBgp() { g_parsed_command.setStringValue("bgp"); }

  void ParserActions::setStatic() { g_parsed_command.setStringValue("static"); }

  void ParserActions::setRoute() { g_parsed_command.setStringValue("route"); }

  // Utility actions
  void ParserActions::setNumber(int num) {
    g_parsed_command.setUnitNumber(num);
  }

  void ParserActions::setIpCidr(const char *cidr) {
    g_parsed_command.setIpAddress(cidr);
  }

  void ParserActions::setLbracket() {
    // Context: left bracket
  }

  void ParserActions::setRbracket() {
    // Context: right bracket
  }

  void ParserActions::setDot() {
    // Context: dot separator
  }

  // Reset and validation
  void ParserActions::reset() { g_parsed_command.reset(); }

  bool ParserActions::isValid() { return g_parsed_command.isValid(); }

  Command &ParserActions::getCurrentCommand() { return g_parsed_command; }

} // namespace netd::client::processor
