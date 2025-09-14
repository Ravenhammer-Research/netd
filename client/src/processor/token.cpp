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

#include <client/include/processor/token.hpp>

namespace netd::client::processor {

const std::unordered_map<int, std::string> TokenMapper::token_names_ = {
    {257, "SET"},
    {258, "DELETE"},
    {259, "SHOW"},
    {260, "COMMIT"},
    {261, "EDIT"},
    {262, "INTERFACES"},
    {263, "ROUTING_INSTANCES"},
    {264, "ROUTING_OPTIONS"},
    {265, "UNIT"},
    {266, "FAMILY"},
    {267, "INET"},
    {268, "ADDRESS"},
    {269, "DESCRIPTION"},
    {270, "ENCAPSULATION"},
    {271, "VLAN_ID"},
    {272, "SPEED"},
    {273, "VLAN_TAGGING"},
    {274, "ETHERNET_VLAN"},
    {275, "INSTANCE_TYPE"},
    {276, "VIRTUAL_ROUTER"},
    {277, "VRF"},
    {278, "VRF_TARGET"},
    {279, "VRF_TABLE_LABEL"},
    {280, "INTERFACE"},
    {281, "STATIC"},
    {282, "ROUTE"},
    {283, "NEXT_HOP"},
    {284, "TABLE"},
    {285, "PROTOCOL"},
    {286, "INSTANCE"},
    {287, "OSPF"},
    {288, "BGP"},
    {289, "VERSION"},
    {290, "CONFIGURATION"},
    {291, "SYSTEM"},
    {292, "UPTIME"},
    {293, "CHASSIS"},
    {294, "LOG"},
    {295, "MESSAGES"},
    {296, "NEIGHBOR"},
    {297, "NEIGHBORS"},
    {298, "SUMMARY"},
    {299, "ARP"},
    {300, "NO_RESOLVE"},
    {301, "PROTOCOLS"},
    {302, "IPV6"},
    {303, "TERSE"},
    {304, "BRIEF"},
    {305, "DETAIL"},
    {306, "EXTENSIVE"},
    {307, "DISPLAY"},
    {308, "ALL"},
    {309, "INTERFACE_NAME"},
    {310, "NUMBER"},
    {311, "IP_CIDR"},
    {312, "IP_ADDRESS"},
    {313, "IDENTIFIER"},
    {314, "STRING"},
    {315, "SPEED_VALUE"},
    {316, "LBRACKET"},
    {317, "RBRACKET"},
    {318, "DOT"}
};

const std::unordered_map<std::string, int> TokenMapper::token_values_ = {
    {"SET", 257},
    {"DELETE", 258},
    {"SHOW", 259},
    {"COMMIT", 260},
    {"EDIT", 261},
    {"INTERFACES", 262},
    {"ROUTING_INSTANCES", 263},
    {"ROUTING_OPTIONS", 264},
    {"UNIT", 265},
    {"FAMILY", 266},
    {"INET", 267},
    {"ADDRESS", 268},
    {"DESCRIPTION", 269},
    {"ENCAPSULATION", 270},
    {"VLAN_ID", 271},
    {"SPEED", 272},
    {"VLAN_TAGGING", 273},
    {"ETHERNET_VLAN", 274},
    {"INSTANCE_TYPE", 275},
    {"VIRTUAL_ROUTER", 276},
    {"VRF", 277},
    {"VRF_TARGET", 278},
    {"VRF_TABLE_LABEL", 279},
    {"INTERFACE", 280},
    {"STATIC", 281},
    {"ROUTE", 282},
    {"NEXT_HOP", 283},
    {"TABLE", 284},
    {"PROTOCOL", 285},
    {"INSTANCE", 286},
    {"OSPF", 287},
    {"BGP", 288},
    {"VERSION", 289},
    {"CONFIGURATION", 290},
    {"SYSTEM", 291},
    {"UPTIME", 292},
    {"CHASSIS", 293},
    {"LOG", 294},
    {"MESSAGES", 295},
    {"NEIGHBOR", 296},
    {"NEIGHBORS", 297},
    {"SUMMARY", 298},
    {"ARP", 299},
    {"NO_RESOLVE", 300},
    {"PROTOCOLS", 301},
    {"IPV6", 302},
    {"TERSE", 303},
    {"BRIEF", 304},
    {"DETAIL", 305},
    {"EXTENSIVE", 306},
    {"DISPLAY", 307},
    {"ALL", 308},
    {"INTERFACE_NAME", 309},
    {"NUMBER", 310},
    {"IP_CIDR", 311},
    {"IP_ADDRESS", 312},
    {"IDENTIFIER", 313},
    {"STRING", 314},
    {"SPEED_VALUE", 315},
    {"LBRACKET", 316},
    {"RBRACKET", 317},
    {"DOT", 318}
};

std::string TokenMapper::getTokenName(int token) {
    auto it = token_names_.find(token);
    return (it != token_names_.end()) ? it->second : "UNKNOWN";
}

int TokenMapper::getTokenValue(const std::string& name) {
    auto it = token_values_.find(name);
    return (it != token_values_.end()) ? it->second : -1;
}

bool TokenMapper::isCommandToken(int token) {
    return token >= 257 && token <= 261; // SET, DELETE, SHOW, COMMIT, EDIT
}

bool TokenMapper::isInterfaceToken(int token) {
    return token == 262 || token == 265 || token == 266 || token == 267 || 
           token == 268 || token == 269 || token == 270 || token == 271 || 
           token == 272 || token == 273 || token == 274 || token == 280; // INTERFACES, UNIT, FAMILY, etc.
}

bool TokenMapper::isRoutingToken(int token) {
    return token == 263 || token == 264 || token == 275 || token == 276 || 
           token == 277 || token == 278 || token == 279 || token == 282 || 
           token == 283 || token == 284; // ROUTING_INSTANCES, ROUTING_OPTIONS, etc.
}

bool TokenMapper::isDisplayToken(int token) {
    return token >= 303 && token <= 308; // TERSE, BRIEF, DETAIL, EXTENSIVE, DISPLAY, ALL
}

bool TokenMapper::isProtocolToken(int token) {
    return token == 281 || token == 285 || token == 287 || token == 288; // STATIC, PROTOCOL, OSPF, BGP
}

bool TokenMapper::isValueToken(int token) {
    return token >= 309 && token <= 318; // INTERFACE_NAME, NUMBER, IP_CIDR, etc.
}

} // namespace netd::client::processor
