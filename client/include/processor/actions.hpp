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

#ifndef NETD_CLIENT_PROCESSOR_ACTIONS_HPP
#define NETD_CLIENT_PROCESSOR_ACTIONS_HPP

#include <client/include/processor/command.hpp>

namespace netd::client::processor {

class ParserActions {
public:
    static void setCommandType(CommandType type);
    static void setInterfaceName(const char* name);
    static void setUnitNumber(int unit);
    static void setIpAddress(const char* ip);
    static void setDescription(const char* desc);
    static void setVlanId(int vlan);
    static void setSpeedValue(const char* speed);
    static void setIdentifier(const char* id);
    static void setStringValue(const char* str);
    static void setVlanTagging(bool tagging);
    static void setDisplayMode(DisplayMode mode);
    
    // Routing instance actions
    static void setInstanceType(const char* type);
    static void setVirtualRouter();
    static void setVrf();
    static void setVrfTarget(const char* target);
    static void setVrfTableLabel();
    
    // Routing options actions
    static void setStaticRoute(const char* route);
    static void setNextHop(const char* hop);
    static void setTable(const char* table);
    static void setProtocol(const char* protocol);
    
    // Show command specific actions
    static void setVersion();
    static void setConfiguration();
    static void setSystem();
    static void setUpTime();
    static void setChassis();
    static void setLog();
    static void setMessages();
    static void setNeighbor();
    static void setNeighbors();
    static void setSummary();
    static void setArp();
    static void setNoResolve();
    static void setProtocols();
    static void setIpv6();
    static void setAll();
    
    // Interface actions
    static void setInterfaces();
    static void setRoutingInstances();
    static void setRoutingOptions();
    static void setUnit();
    static void setFamily();
    static void setInet();
    static void setAddress();
    static void setEncapsulation();
    static void setEthernetVlan();
    static void setInterface();
    
    // Protocol actions
    static void setOspf();
    static void setBgp();
    static void setStatic();
    static void setRoute();
    
    // Utility actions
    static void setNumber(int num);
    static void setIpCidr(const char* cidr);
    static void setLbracket();
    static void setRbracket();
    static void setDot();
    
    // Reset and validation
    static void reset();
    static bool isValid();
    static Command& getCurrentCommand();
};

} // namespace netd::client::processor

#endif // NETD_CLIENT_PROCESSOR_ACTIONS_HPP
