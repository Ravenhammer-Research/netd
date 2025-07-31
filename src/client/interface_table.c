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

#include "net.h"
#include <bsdxml.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <stdbool.h>

/* Forward declarations for functions from other modules */
extern int find_tag_content(const char *xml, const char *tag, char *content, size_t max_len);
extern void format_interface_flags(int flags, char *flag_str, size_t max_len);

/**
 * Print interface table from XML response
 * @param xml_response XML response string
 * @return 0 on success, -1 on failure
 */
int print_interface_table(const char *xml_response)
{
    const char *pos = xml_response;
    char name[64], enabled[16], fib[16], mtu[16], flags[16], addr[64], addr6[64];
    char groups[256]; /* Buffer for group information */
    
    /* Column width tracking */
    int name_width = 4;      /* "Name" */
    int status_width = 6;    /* "Status" */
    int vrf_width = 3;       /* "VRF" */
    int tunvrf_width = 6;    /* "TUNVRF" */
    int mtu_width = 3;       /* "MTU" */
    int flags_width = 5;     /* "Flags" */
    int ipv4_width = 4;      /* "IPv4" */
    int ipv6_width = 4;      /* "IPv6" */
    int groups_width = 6;    /* "Groups" */
    
    if (!xml_response) {
        return -1;
    }

    /* First pass: find maximum widths */
    pos = strstr(xml_response, "<interfaces");
    if (pos) {
        while ((pos = strstr(pos, "<interface>")) != NULL) {
            pos += 11;
            
            if (find_tag_content(pos, "name", name, sizeof(name)) == 0 &&
                find_tag_content(pos, "enabled", enabled, sizeof(enabled)) == 0) {
                
                find_tag_content(pos, "vrf", fib, sizeof(fib));
                find_tag_content(pos, "mtu", mtu, sizeof(mtu));
                find_tag_content(pos, "flags", flags, sizeof(flags));
                find_tag_content(pos, "primary-address", addr, sizeof(addr));
                find_tag_content(pos, "primary-address6", addr6, sizeof(addr6));
                
                /* Get group information - look for <group> tag with <group-name> sub-elements */
                groups[0] = '\0'; /* Initialize empty */
                const char *group_start = strstr(pos, "<group>");
                if (group_start) {
                    const char *group_end = strstr(group_start, "</group>");
                    if (group_end) {
                        const char *group_name_pos = group_start;
                        while ((group_name_pos = strstr(group_name_pos, "<group-name>")) != NULL && group_name_pos < group_end) {
                            group_name_pos += 12; /* Skip "<group-name>" */
                            const char *group_name_end = strstr(group_name_pos, "</group-name>");
                            if (group_name_end && group_name_end < group_end) {
                                size_t len = group_name_end - group_name_pos;
                                if (len < sizeof(groups) - strlen(groups) - 2) {
                                    if (groups[0] != '\0') {
                                        strcat(groups, ",");
                                    }
                                    strncat(groups, group_name_pos, len);
                                }
                            }
                            if (group_name_end) {
                                group_name_pos = group_name_end;
                            } else {
                                break; /* No closing tag found, exit loop */
                            }
                        }
                    }
                }
                
                /* Update max widths */
                int len = strlen(name);
                if (len > name_width) name_width = len;
                
                len = strlen(strcmp(enabled, "true") == 0 ? "UP" : "DOWN");
                if (len > status_width) status_width = len;
                
                len = strlen(fib);
                if (len > vrf_width) vrf_width = len;
                
                /* Check if this is a bridge interface and get bridge members */
                char bridge_members[256] = "";
                if (strncmp(name, "bridge", 6) == 0) {
                    find_tag_content(pos, "bridge-members", bridge_members, sizeof(bridge_members));
                }
                
                len = strlen(bridge_members) > 0 ? strlen(bridge_members) : 1; /* Bridge members or "-" for TUNVRF */
                if (len > tunvrf_width) tunvrf_width = len;
                
                len = strlen(mtu);
                if (len > mtu_width) mtu_width = len;
                
                /* Format flags */
                char flag_str[16] = "";
                int flag_val = atoi(flags);
                format_interface_flags(flag_val, flag_str, sizeof(flag_str));
                
                len = strlen(flag_str);
                if (len > flags_width) flags_width = len;
                
                len = strlen(addr);
                if (len > ipv4_width) ipv4_width = len;
                
                len = strlen(addr6);
                if (len > ipv6_width) ipv6_width = len;
                
                len = strlen(groups);
                if (len > groups_width) groups_width = len;
                
                /* Check alias addresses for width calculation - only within this interface */
                const char *interface_end_check = strstr(pos, "</interface>");
                if (!interface_end_check) {
                    interface_end_check = pos + strlen(pos); /* Fallback */
                }
                
                const char *alias_check = pos;
                while ((alias_check = strstr(alias_check, "<alias>")) != NULL && alias_check < interface_end_check) {
                    alias_check += 7;
                    
                    const char *addr_start = strstr(alias_check, "<address>");
                    if (addr_start) {
                        addr_start += 9;
                        const char *addr_end = strstr(addr_start, "</address>");
                        if (addr_end) {
                            char temp_addr[64];
                            size_t len = addr_end - addr_start;
                            if (len < sizeof(temp_addr)) {
                                strncpy(temp_addr, addr_start, len);
                                temp_addr[len] = '\0';
                                int temp_len = strlen(temp_addr);
                                if (temp_len > ipv4_width) ipv4_width = temp_len;
                            }
                        }
                    }
                    
                    const char *addr6_start = strstr(alias_check, "<address6>");
                    if (addr6_start) {
                        addr6_start += 10;
                        const char *addr6_end = strstr(addr6_start, "</address6>");
                        if (addr6_end) {
                            char temp_addr6[64];
                            size_t len = addr6_end - addr6_start;
                            if (len < sizeof(temp_addr6)) {
                                strncpy(temp_addr6, addr6_start, len);
                                temp_addr6[len] = '\0';
                                int temp_len = strlen(temp_addr6);
                                if (temp_len > ipv6_width) ipv6_width = temp_len;
                            }
                        }
                    }
                    
                    alias_check = strstr(alias_check, "</alias>");
                    if (alias_check) alias_check += 8;
                }
            }
            
            pos = strstr(pos, "</interface>");
            if (pos) pos += 12;
        }
    }

    /* Print table header */
    printf("\nInterface Table:\n");
    printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
           name_width, "Name", status_width, "Status", vrf_width, "VRF", 
           tunvrf_width, "Members", mtu_width, "MTU", flags_width, "Flags", 
           ipv4_width, "IPv4", ipv6_width, "IPv6", groups_width, "Groups");
    
    /* Find interfaces section */
    pos = strstr(xml_response, "<interfaces");
    if (!pos) {
        print_error("Could not find interfaces section in XML");
        return -1;
    }

    /* Find each interface */
    while ((pos = strstr(pos, "<interface>")) != NULL) {
        pos += 11; /* Skip "<interface>" */
        
        /* Extract interface data */
        if (find_tag_content(pos, "name", name, sizeof(name)) == 0 &&
            find_tag_content(pos, "enabled", enabled, sizeof(enabled)) == 0) {
            
            /* Extract optional fields - don't fail if missing */
            find_tag_content(pos, "vrf", fib, sizeof(fib));
            find_tag_content(pos, "mtu", mtu, sizeof(mtu));
            find_tag_content(pos, "flags", flags, sizeof(flags));
            find_tag_content(pos, "primary-address", addr, sizeof(addr));
            find_tag_content(pos, "primary-address6", addr6, sizeof(addr6));
            
            /* Get group information - look for <group> tag with <group-name> sub-elements */
            groups[0] = '\0'; /* Initialize empty */
            const char *group_start = strstr(pos, "<group>");
            if (group_start) {
                const char *group_end = strstr(group_start, "</group>");
                if (group_end) {
                    const char *group_name_pos = group_start;
                    while ((group_name_pos = strstr(group_name_pos, "<group-name>")) != NULL && group_name_pos < group_end) {
                        group_name_pos += 12; /* Skip "<group-name>" */
                        const char *group_name_end = strstr(group_name_pos, "</group-name>");
                        if (group_name_end && group_name_end < group_end) {
                            size_t len = group_name_end - group_name_pos;
                            if (len < sizeof(groups) - strlen(groups) - 2) {
                                if (groups[0] != '\0') {
                                    strcat(groups, ",");
                                }
                                strncat(groups, group_name_pos, len);
                            }
                        }
                        group_name_pos = group_name_end;
                    }
                }
            }
            
            /* Format flags */
            char flag_str[16] = "";
            int flag_val = atoi(flags);
            format_interface_flags(flag_val, flag_str, sizeof(flag_str));
            
            /* Get bridge members for bridge interfaces */
            char bridge_members[256] = "";
            if (strncmp(name, "bridge", 6) == 0) {
                find_tag_content(pos, "bridge-members", bridge_members, sizeof(bridge_members));
            }
            
            /* Print interface with groups - each group on a separate row */
            if (groups[0] != '\0') {
                char *groups_copy = strdup(groups);
                char *group = strtok(groups_copy, ",");
                bool first_group = true;
                
                while (group != NULL) {
                    /* Trim whitespace */
                    while (*group == ' ') group++;
                    char *end = group + strlen(group) - 1;
                    while (end > group && *end == ' ') end--;
                    *(end + 1) = '\0';
                    
                    if (first_group) {
                        /* First group row shows the interface info */
                        if (strncmp(name, "bridge", 6) == 0 && bridge_members[0] != '\0') {
                            /* For bridge interfaces, also handle bridge members */
                            char *members_copy = strdup(bridge_members);
                            char *member = strtok(members_copy, ",");
                            bool first_member = true;
                            
                            while (member != NULL) {
                                /* Trim whitespace */
                                while (*member == ' ') member++;
                                char *member_end = member + strlen(member) - 1;
                                while (member_end > member && *member_end == ' ') member_end--;
                                *(member_end + 1) = '\0';
                                
                                if (first_member) {
                                    /* First member row shows the bridge interface info */
                                    printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                                           name_width, name,
                                           status_width, strcmp(enabled, "true") == 0 ? "UP" : "DOWN",
                                           vrf_width, fib,
                                           tunvrf_width, member,
                                           mtu_width, mtu,
                                           flags_width, flag_str,
                                           ipv4_width, addr[0] ? addr : "",
                                           ipv6_width, addr6[0] ? addr6 : "",
                                           groups_width, group);
                                    first_member = false;
                                } else {
                                    /* Additional member rows show just the member name */
                                    printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                                           name_width, "", /* Empty name for member row */
                                           status_width, "", /* Empty status for member row */
                                           vrf_width, "", /* Empty VRF for member row */
                                           tunvrf_width, member,
                                           mtu_width, "", /* Empty MTU for member row */
                                           flags_width, "", /* Empty flags for member row */
                                           ipv4_width, "", /* Empty IPv4 for member row */
                                           ipv6_width, "", /* Empty IPv6 for member row */
                                           groups_width, ""); /* Empty groups for member row */
                                }
                                
                                member = strtok(NULL, ",");
                            }
                            
                            free(members_copy);
                        } else {
                            /* Non-bridge interface */
                            printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                                   name_width, name,
                                   status_width, strcmp(enabled, "true") == 0 ? "UP" : "DOWN",
                                   vrf_width, fib,
                                   tunvrf_width, bridge_members[0] ? bridge_members : "-",
                                   mtu_width, mtu,
                                   flags_width, flag_str,
                                   ipv4_width, addr[0] ? addr : "",
                                   ipv6_width, addr6[0] ? addr6 : "",
                                   groups_width, group);
                        }
                        first_group = false;
                    } else {
                        /* Additional group rows show just the group name */
                        printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                               name_width, "", /* Empty name for group row */
                               status_width, "", /* Empty status for group row */
                               vrf_width, "", /* Empty VRF for group row */
                               tunvrf_width, "", /* Empty members for group row */
                               mtu_width, "", /* Empty MTU for group row */
                               flags_width, "", /* Empty flags for group row */
                               ipv4_width, "", /* Empty IPv4 for group row */
                               ipv6_width, "", /* Empty IPv6 for group row */
                               groups_width, group);
                    }
                    
                    group = strtok(NULL, ",");
                }
                
                free(groups_copy);
            } else {
                /* Interface without groups */
                if (strncmp(name, "bridge", 6) == 0 && bridge_members[0] != '\0') {
                    /* Bridge interface without groups but with members */
                    char *members_copy = strdup(bridge_members);
                    char *member = strtok(members_copy, ",");
                    bool first_member = true;
                    
                    while (member != NULL) {
                        /* Trim whitespace */
                        while (*member == ' ') member++;
                        char *end = member + strlen(member) - 1;
                        while (end > member && *end == ' ') end--;
                        *(end + 1) = '\0';
                        
                        if (first_member) {
                            /* First member row shows the bridge interface info */
                            printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                                   name_width, name,
                                   status_width, strcmp(enabled, "true") == 0 ? "UP" : "DOWN",
                                   vrf_width, fib,
                                   tunvrf_width, member,
                                   mtu_width, mtu,
                                   flags_width, flag_str,
                                   ipv4_width, addr[0] ? addr : "",
                                   ipv6_width, addr6[0] ? addr6 : "",
                                   groups_width, "");
                            first_member = false;
                        } else {
                            /* Additional member rows show just the member name */
                            printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                                   name_width, "", /* Empty name for member row */
                                   status_width, "", /* Empty status for member row */
                                   vrf_width, "", /* Empty VRF for member row */
                                   tunvrf_width, member,
                                   mtu_width, "", /* Empty MTU for member row */
                                   flags_width, "", /* Empty flags for member row */
                                   ipv4_width, "", /* Empty IPv4 for member row */
                                   ipv6_width, "", /* Empty IPv6 for member row */
                                   groups_width, "");
                        }
                        
                        member = strtok(NULL, ",");
                    }
                    
                    free(members_copy);
                } else {
                    /* Regular interface without groups or members */
                    printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                           name_width, name,
                           status_width, strcmp(enabled, "true") == 0 ? "UP" : "DOWN",
                           vrf_width, fib,
                           tunvrf_width, bridge_members[0] ? bridge_members : "-",
                           mtu_width, mtu,
                           flags_width, flag_str,
                           ipv4_width, addr[0] ? addr : "",
                           ipv6_width, addr6[0] ? addr6 : "",
                           groups_width, "");
                }
            }
            
            /* Print alias addresses if any - for all interfaces */
            {
                const char *interface_end = strstr(pos, "</interface>");
                if (!interface_end) {
                    interface_end = pos + strlen(pos); /* Fallback */
                }
                size_t interface_len = interface_end - pos;
                char interface_xml[4096];
                if (interface_len >= sizeof(interface_xml)) interface_len = sizeof(interface_xml) - 1;
                strncpy(interface_xml, pos, interface_len);
                interface_xml[interface_len] = '\0';

                const char *alias_pos = interface_xml;
                while ((alias_pos = strstr(alias_pos, "<alias>")) != NULL) {
                    alias_pos += 7; /* Skip "<alias>" */
                    char alias_addr[64] = "", alias_addr6[64] = "";
                    /* Look for address tag within this alias */
                    const char *addr_start = strstr(alias_pos, "<address>");
                    if (addr_start) {
                        addr_start += 9; /* Skip "<address>" */
                        const char *addr_end = strstr(addr_start, "</address>");
                        if (addr_end) {
                            size_t len = addr_end - addr_start;
                            if (len < sizeof(alias_addr)) {
                                strncpy(alias_addr, addr_start, len);
                                alias_addr[len] = '\0';
                            }
                        }
                    }
                    /* Look for address6 tag within this alias */
                    const char *addr6_start = strstr(alias_pos, "<address6>");
                    if (addr6_start) {
                        addr6_start += 10; /* Skip "<address6>" */
                        const char *addr6_end = strstr(addr6_start, "</address6>");
                        if (addr6_end) {
                            size_t len = addr6_end - addr6_start;
                            if (len < sizeof(alias_addr6)) {
                                strncpy(alias_addr6, addr6_start, len);
                                alias_addr6[len] = '\0';
                            }
                        }
                    }
                    if (alias_addr[0] || alias_addr6[0]) {
                        printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                               name_width, "", /* Empty name for alias row */
                               status_width, "", /* Empty status for alias row */
                               vrf_width, "", /* Empty VRF for alias row */
                               tunvrf_width, "", /* Empty members for alias row */
                               mtu_width, "", /* Empty MTU for alias row */
                               flags_width, "", /* Empty flags for alias row */
                               ipv4_width, alias_addr[0] ? alias_addr : "",
                               ipv6_width, alias_addr6[0] ? alias_addr6 : "",
                               groups_width, ""); /* Empty groups for alias row */
                    }
                    alias_pos = strstr(alias_pos, "</alias>");
                    if (alias_pos) alias_pos += 8; /* Skip "</alias>" */
                }
            }
        }
        
        /* Move to next interface */
        pos = strstr(pos, "</interface>");
        if (pos) {
            pos += 12; /* Skip "</interface>" */
        } else {
            break; /* No closing interface tag found, exit loop */
        }
    }

    printf("Total interfaces displayed above.\n");
    printf("Flags: U=UP, B=BROADCAST, R=RUNNING, P=PROMISC, M=MULTICAST, L=LOOPBACK\n\n");
    return 0;
}

/**
 * Print interface table from XML response, filtered by group
 * @param xml_response XML response string
 * @param group_name Group name to filter by
 * @return 0 on success, -1 on failure
 */
int print_interface_table_filtered(const char *xml_response, const char *group_name)
{
    const char *pos = xml_response;
    char name[64], enabled[16], fib[16], mtu[16], flags[16], addr[64], addr6[64];
    char groups[256]; /* Buffer for group information */
    int count = 0;
    
    /* Column width tracking */
    int name_width = 4;      /* "Name" */
    int status_width = 6;    /* "Status" */
    int vrf_width = 3;       /* "VRF" */
    int tunvrf_width = 6;    /* "TUNVRF" */
    int mtu_width = 3;       /* "MTU" */
    int flags_width = 5;     /* "Flags" */
    int ipv4_width = 4;      /* "IPv4" */
    int ipv6_width = 4;      /* "IPv6" */
    int groups_width = 6;    /* "Groups" */
    
    if (!xml_response || !group_name) {
        return -1;
    }

    /* First pass: find maximum widths and count matching interfaces */
    pos = strstr(xml_response, "<interfaces");
    if (pos) {
        while (pos && (pos = strstr(pos, "<interface>")) != NULL) {
            pos += 11;
            
            if (find_tag_content(pos, "name", name, sizeof(name)) == 0 &&
                find_tag_content(pos, "enabled", enabled, sizeof(enabled)) == 0) {
                
                /* Get group information - look for <group> tag with <group-name> sub-elements */
                groups[0] = '\0'; /* Initialize empty */
                const char *group_start = strstr(pos, "<group>");
                if (group_start) {
                    const char *group_end = strstr(group_start, "</group>");
                    if (group_end) {
                        const char *group_name_pos = group_start;
                        while ((group_name_pos = strstr(group_name_pos, "<group-name>")) != NULL && group_name_pos < group_end) {
                            group_name_pos += 12; /* Skip "<group-name>" */
                            const char *group_name_end = strstr(group_name_pos, "</group-name>");
                            if (group_name_end && group_name_end < group_end) {
                                size_t len = group_name_end - group_name_pos;
                                if (len < sizeof(groups) - strlen(groups) - 2) {
                                    if (groups[0] != '\0') {
                                        strcat(groups, ",");
                                    }
                                    strncat(groups, group_name_pos, len);
                                }
                            }
                            if (group_name_end) {
                                group_name_pos = group_name_end;
                            } else {
                                break; /* No closing tag found, exit loop */
                            }
                        }
                    }
                }
                
                /* Check if interface belongs to the specified group */
                if (strstr(groups, group_name) != NULL) {
                    find_tag_content(pos, "vrf", fib, sizeof(fib));
                    find_tag_content(pos, "mtu", mtu, sizeof(mtu));
                    find_tag_content(pos, "flags", flags, sizeof(flags));
                    find_tag_content(pos, "primary-address", addr, sizeof(addr));
                    find_tag_content(pos, "primary-address6", addr6, sizeof(addr6));
                    
                    /* Update max widths */
                    int len = strlen(name);
                    if (len > name_width) name_width = len;
                    
                    len = strlen(strcmp(enabled, "true") == 0 ? "UP" : "DOWN");
                    if (len > status_width) status_width = len;
                    
                    len = strlen(fib);
                    if (len > vrf_width) vrf_width = len;
                    
                    /* Check if this is a bridge interface and get bridge members */
                    char bridge_members[256] = "";
                    if (strncmp(name, "bridge", 6) == 0) {
                        find_tag_content(pos, "bridge-members", bridge_members, sizeof(bridge_members));
                    }
                    
                    len = strlen(bridge_members) > 0 ? strlen(bridge_members) : 1; /* Bridge members or "-" for TUNVRF */
                    if (len > tunvrf_width) tunvrf_width = len;
                    
                    len = strlen(mtu);
                    if (len > mtu_width) mtu_width = len;
                    
                    /* Format flags */
                    char flag_str[16] = "";
                    int flag_val = atoi(flags);
                    format_interface_flags(flag_val, flag_str, sizeof(flag_str));
                    
                    len = strlen(flag_str);
                    if (len > flags_width) flags_width = len;
                    
                    len = strlen(addr);
                    if (len > ipv4_width) ipv4_width = len;
                    
                    len = strlen(addr6);
                    if (len > ipv6_width) ipv6_width = len;
                    
                    len = strlen(groups);
                    if (len > groups_width) groups_width = len;
                    
                    count++;
                }
            }
            
            pos = strstr(pos, "</interface>");
            if (pos) pos += 12;
        }
    }

    /* Print table header */
    printf("\nInterface Table (Group: %s):\n", group_name);
    printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
           name_width, "Name", status_width, "Status", vrf_width, "VRF", 
           tunvrf_width, "Members", mtu_width, "MTU", flags_width, "Flags", 
           ipv4_width, "IPv4", ipv6_width, "IPv6", groups_width, "Groups");

    /* Reset for second pass */
    pos = strstr(xml_response, "<interfaces");
    if (!pos) {
        print_error("Could not find interfaces section in XML");
        return -1;
    }

    /* Find each interface */
    while ((pos = strstr(pos, "<interface>")) != NULL) {
        pos += 11; /* Skip "<interface>" */
        
        /* Extract interface data */
        if (find_tag_content(pos, "name", name, sizeof(name)) == 0 &&
            find_tag_content(pos, "enabled", enabled, sizeof(enabled)) == 0) {
            
            /* Get group information - look for <group> tag with <group-name> sub-elements */
            groups[0] = '\0'; /* Initialize empty */
            const char *group_start = strstr(pos, "<group>");
            if (group_start) {
                const char *group_end = strstr(group_start, "</group>");
                if (group_end) {
                    const char *group_name_pos = group_start;
                    while ((group_name_pos = strstr(group_name_pos, "<group-name>")) != NULL && group_name_pos < group_end) {
                        group_name_pos += 12; /* Skip "<group-name>" */
                        const char *group_name_end = strstr(group_name_pos, "</group-name>");
                        if (group_name_end && group_name_end < group_end) {
                            size_t len = group_name_end - group_name_pos;
                            if (len < sizeof(groups) - strlen(groups) - 2) {
                                if (groups[0] != '\0') {
                                    strcat(groups, ",");
                                }
                                strncat(groups, group_name_pos, len);
                            }
                        }
                        if (group_name_end) {
                            group_name_pos = group_name_end;
                        } else {
                            break; /* No closing tag found, exit loop */
                        }
                    }
                }
            }
            
            /* Only process interfaces that belong to the specified group */
            if (strstr(groups, group_name) != NULL) {
                /* Extract optional fields - don't fail if missing */
                find_tag_content(pos, "vrf", fib, sizeof(fib));
                find_tag_content(pos, "mtu", mtu, sizeof(mtu));
                find_tag_content(pos, "flags", flags, sizeof(flags));
                find_tag_content(pos, "primary-address", addr, sizeof(addr));
                find_tag_content(pos, "primary-address6", addr6, sizeof(addr6));
                
                /* Format flags */
                char flag_str[16] = "";
                int flag_val = atoi(flags);
                format_interface_flags(flag_val, flag_str, sizeof(flag_str));
                
                /* Get bridge members for bridge interfaces */
                char bridge_members[256] = "";
                if (strncmp(name, "bridge", 6) == 0) {
                    find_tag_content(pos, "bridge-members", bridge_members, sizeof(bridge_members));
                }
                
                /* Print interface with groups - each group on a separate row */
                if (groups[0] != '\0') {
                    char *groups_copy = strdup(groups);
                    char *group = strtok(groups_copy, ",");
                    bool first_group = true;
                    
                    while (group != NULL) {
                        /* Trim whitespace */
                        while (*group == ' ') group++;
                        char *end = group + strlen(group) - 1;
                        while (end > group && *end == ' ') end--;
                        *(end + 1) = '\0';
                        
                        if (first_group) {
                            /* First group row shows the interface info */
                            if (strncmp(name, "bridge", 6) == 0 && bridge_members[0] != '\0') {
                                /* For bridge interfaces, also handle bridge members */
                                char *members_copy = strdup(bridge_members);
                                char *member = strtok(members_copy, ",");
                                bool first_member = true;
                                
                                while (member != NULL) {
                                    /* Trim whitespace */
                                    while (*member == ' ') member++;
                                    char *member_end = member + strlen(member) - 1;
                                    while (member_end > member && *member_end == ' ') member_end--;
                                    *(member_end + 1) = '\0';
                                    
                                    if (first_member) {
                                        /* First member row shows the bridge interface info */
                                        printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                                               name_width, name,
                                               status_width, strcmp(enabled, "true") == 0 ? "UP" : "DOWN",
                                               vrf_width, fib,
                                               tunvrf_width, member,
                                               mtu_width, mtu,
                                               flags_width, flag_str,
                                               ipv4_width, addr[0] ? addr : "",
                                               ipv6_width, addr6[0] ? addr6 : "",
                                               groups_width, group);
                                        first_member = false;
                                    } else {
                                        /* Additional member rows show just the member name */
                                        printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                                               name_width, "", /* Empty name for member row */
                                               status_width, "", /* Empty status for member row */
                                               vrf_width, "", /* Empty VRF for member row */
                                               tunvrf_width, member,
                                               mtu_width, "", /* Empty MTU for member row */
                                               flags_width, "", /* Empty flags for member row */
                                               ipv4_width, "", /* Empty IPv4 for member row */
                                               ipv6_width, "", /* Empty IPv6 for member row */
                                               groups_width, ""); /* Empty groups for member row */
                                    }
                                    
                                    member = strtok(NULL, ",");
                                }
                                
                                free(members_copy);
                            } else {
                                /* Non-bridge interface */
                                printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                                       name_width, name,
                                       status_width, strcmp(enabled, "true") == 0 ? "UP" : "DOWN",
                                       vrf_width, fib,
                                       tunvrf_width, bridge_members[0] ? bridge_members : "-",
                                       mtu_width, mtu,
                                       flags_width, flag_str,
                                       ipv4_width, addr[0] ? addr : "",
                                       ipv6_width, addr6[0] ? addr6 : "",
                                       groups_width, group);
                            }
                            first_group = false;
                        } else {
                            /* Additional group rows show just the group name */
                            printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                                   name_width, "", /* Empty name for group row */
                                   status_width, "", /* Empty status for group row */
                                   vrf_width, "", /* Empty VRF for group row */
                                   tunvrf_width, "", /* Empty members for group row */
                                   mtu_width, "", /* Empty MTU for group row */
                                   flags_width, "", /* Empty flags for group row */
                                   ipv4_width, "", /* Empty IPv4 for group row */
                                   ipv6_width, "", /* Empty IPv6 for group row */
                                   groups_width, group);
                        }
                        
                        group = strtok(NULL, ",");
                    }
                    
                    free(groups_copy);
                } else {
                    /* Interface without groups */
                    if (strncmp(name, "bridge", 6) == 0 && bridge_members[0] != '\0') {
                        /* Bridge interface without groups but with members */
                        char *members_copy = strdup(bridge_members);
                        char *member = strtok(members_copy, ",");
                        bool first_member = true;
                        
                        while (member != NULL) {
                            /* Trim whitespace */
                            while (*member == ' ') member++;
                            char *end = member + strlen(member) - 1;
                            while (end > member && *end == ' ') end--;
                            *(end + 1) = '\0';
                            
                            if (first_member) {
                                /* First member row shows the bridge interface info */
                                printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                                       name_width, name,
                                       status_width, strcmp(enabled, "true") == 0 ? "UP" : "DOWN",
                                       vrf_width, fib,
                                       tunvrf_width, member,
                                       mtu_width, mtu,
                                       flags_width, flag_str,
                                       ipv4_width, addr[0] ? addr : "",
                                       ipv6_width, addr6[0] ? addr6 : "",
                                       groups_width, "");
                                first_member = false;
                            } else {
                                /* Additional member rows show just the member name */
                                printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                                       name_width, "", /* Empty name for member row */
                                       status_width, "", /* Empty status for member row */
                                       vrf_width, "", /* Empty VRF for member row */
                                       tunvrf_width, member,
                                       mtu_width, "", /* Empty MTU for member row */
                                       flags_width, "", /* Empty flags for member row */
                                       ipv4_width, "", /* Empty IPv4 for member row */
                                       ipv6_width, "", /* Empty IPv6 for member row */
                                       groups_width, "");
                            }
                            
                            member = strtok(NULL, ",");
                        }
                        
                        free(members_copy);
                    } else {
                        /* Regular interface without groups or members */
                        printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                               name_width, name,
                               status_width, strcmp(enabled, "true") == 0 ? "UP" : "DOWN",
                               vrf_width, fib,
                               tunvrf_width, bridge_members[0] ? bridge_members : "-",
                               mtu_width, mtu,
                               flags_width, flag_str,
                               ipv4_width, addr[0] ? addr : "",
                               ipv6_width, addr6[0] ? addr6 : "",
                               groups_width, "");
                    }
                }
                
                /* Print alias addresses if any - for all interfaces */
                {
                    const char *interface_end = strstr(pos, "</interface>");
                    if (!interface_end) {
                        interface_end = pos + strlen(pos); /* Fallback */
                    }
                    size_t interface_len = interface_end - pos;
                    char interface_xml[4096];
                    if (interface_len >= sizeof(interface_xml)) interface_len = sizeof(interface_xml) - 1;
                    strncpy(interface_xml, pos, interface_len);
                    interface_xml[interface_len] = '\0';

                    const char *alias_pos = interface_xml;
                    while ((alias_pos = strstr(alias_pos, "<alias>")) != NULL) {
                        alias_pos += 7; /* Skip "<alias>" */
                        char alias_addr[64] = "", alias_addr6[64] = "";
                        /* Look for address tag within this alias */
                        const char *addr_start = strstr(alias_pos, "<address>");
                        if (addr_start) {
                            addr_start += 9; /* Skip "<address>" */
                            const char *addr_end = strstr(addr_start, "</address>");
                            if (addr_end) {
                                size_t len = addr_end - addr_start;
                                if (len < sizeof(alias_addr)) {
                                    strncpy(alias_addr, addr_start, len);
                                    alias_addr[len] = '\0';
                                }
                            }
                        }
                        /* Look for address6 tag within this alias */
                        const char *addr6_start = strstr(alias_pos, "<address6>");
                        if (addr6_start) {
                            addr6_start += 10; /* Skip "<address6>" */
                            const char *addr6_end = strstr(addr6_start, "</address6>");
                            if (addr6_end) {
                                size_t len = addr6_end - addr6_start;
                                if (len < sizeof(alias_addr6)) {
                                    strncpy(alias_addr6, addr6_start, len);
                                    alias_addr6[len] = '\0';
                                }
                            }
                        }
                        if (alias_addr[0] || alias_addr6[0]) {
                            printf("%*s %*s %*s %*s %*s %*s %*s %*s %*s\n", 
                                   name_width, "", /* Empty name for alias row */
                                   status_width, "", /* Empty status for alias row */
                                   vrf_width, "", /* Empty VRF for alias row */
                                   tunvrf_width, "", /* Empty members for alias row */
                                   mtu_width, "", /* Empty MTU for alias row */
                                   flags_width, "", /* Empty flags for alias row */
                                   ipv4_width, alias_addr[0] ? alias_addr : "",
                                   ipv6_width, alias_addr6[0] ? alias_addr6 : "",
                                   groups_width, ""); /* Empty groups for alias row */
                        }
                        alias_pos = strstr(alias_pos, "</alias>");
                        if (alias_pos) alias_pos += 8; /* Skip "</alias>" */
                    }
                }
            }
        }
        
        /* Move to next interface */
        pos = strstr(pos, "</interface>");
        if (pos) {
            pos += 12; /* Skip "</interface>" */
        } else {
            break; /* No closing interface tag found, exit loop */
        }
    }

    printf("Total interfaces in group '%s': %d\n", group_name, count);
    printf("Flags: U=UP, B=BROADCAST, R=RUNNING, P=PROMISC, M=MULTICAST, L=LOOPBACK\n\n");
    return 0;
} 