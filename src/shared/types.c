/*
 * Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "types.h"

/* Initialize capacity-constrained TAILQ structures */
void netd_interface_groups_init(netd_interface_groups_t *groups) {
  TAILQ_INIT(&groups->head);
  groups->count = 0;
  groups->max_count = MAX_GROUPS_PER_IF;
}

void netd_bridge_members_init(netd_bridge_members_t *members) {
  TAILQ_INIT(&members->head);
  members->count = 0;
  members->max_count = MAX_BRIDGE_MEMBERS;
}

void netd_lagg_members_init(netd_lagg_members_t *members) {
  TAILQ_INIT(&members->head);
  members->count = 0;
  members->max_count = MAX_LAGG_MEMBERS;
}

/* Add operations (FIFO append) */
bool netd_interface_groups_add(netd_interface_groups_t *groups, const char *group_name) {
  if (groups->count >= groups->max_count) {
    return false; /* Capacity exceeded */
  }
  
  netd_interface_group_t *group = malloc(sizeof(netd_interface_group_t));
  if (!group) {
    return false; /* Memory allocation failed */
  }
  
  strncpy(group->name, group_name, MAX_GROUP_NAME_LEN - 1);
  group->name[MAX_GROUP_NAME_LEN - 1] = '\0';
  
  TAILQ_INSERT_TAIL(&groups->head, group, entries);
  groups->count++;
  return true;
}

bool netd_bridge_members_add(netd_bridge_members_t *members, netd_interface_t *interface) {
  if (members->count >= members->max_count) {
    return false; /* Capacity exceeded */
  }
  
  netd_bridge_member_t *member = malloc(sizeof(netd_bridge_member_t));
  if (!member) {
    return false; /* Memory allocation failed */
  }
  
  member->interface = interface;
  TAILQ_INSERT_TAIL(&members->head, member, entries);
  members->count++;
  return true;
}

bool netd_lagg_members_add(netd_lagg_members_t *members, netd_interface_t *interface) {
  if (members->count >= members->max_count) {
    return false; /* Capacity exceeded */
  }
  
  netd_lagg_member_t *member = malloc(sizeof(netd_lagg_member_t));
  if (!member) {
    return false; /* Memory allocation failed */
  }
  
  member->interface = interface;
  TAILQ_INSERT_TAIL(&members->head, member, entries);
  members->count++;
  return true;
}

/* Remove operations (FIFO pop from head) */
netd_interface_group_t *netd_interface_groups_remove(netd_interface_groups_t *groups) {
  netd_interface_group_t *group = TAILQ_FIRST(&groups->head);
  if (group) {
    TAILQ_REMOVE(&groups->head, group, entries);
    groups->count--;
  }
  return group;
}

netd_bridge_member_t *netd_bridge_members_remove(netd_bridge_members_t *members) {
  netd_bridge_member_t *member = TAILQ_FIRST(&members->head);
  if (member) {
    TAILQ_REMOVE(&members->head, member, entries);
    members->count--;
  }
  return member;
}

netd_lagg_member_t *netd_lagg_members_remove(netd_lagg_members_t *members) {
  netd_lagg_member_t *member = TAILQ_FIRST(&members->head);
  if (member) {
    TAILQ_REMOVE(&members->head, member, entries);
    members->count--;
  }
  return member;
}

/* Find operations */
netd_interface_group_t *netd_interface_groups_find(netd_interface_groups_t *groups, const char *group_name) {
  netd_interface_group_t *group;
  TAILQ_FOREACH(group, &groups->head, entries) {
    if (strcmp(group->name, group_name) == 0) {
      return group;
    }
  }
  return NULL;
}

netd_bridge_member_t *netd_bridge_members_find(netd_bridge_members_t *members, netd_interface_t *interface) {
  netd_bridge_member_t *member;
  TAILQ_FOREACH(member, &members->head, entries) {
    if (member->interface == interface) {
      return member;
    }
  }
  return NULL;
}

netd_lagg_member_t *netd_lagg_members_find(netd_lagg_members_t *members, netd_interface_t *interface) {
  netd_lagg_member_t *member;
  TAILQ_FOREACH(member, &members->head, entries) {
    if (member->interface == interface) {
      return member;
    }
  }
  return NULL;
}

/* Remove specific item operations */
bool netd_interface_groups_remove_item(netd_interface_groups_t *groups, const char *group_name) {
  netd_interface_group_t *group = netd_interface_groups_find(groups, group_name);
  if (group) {
    TAILQ_REMOVE(&groups->head, group, entries);
    groups->count--;
    free(group);
    return true;
  }
  return false;
}

bool netd_bridge_members_remove_item(netd_bridge_members_t *members, netd_interface_t *interface) {
  netd_bridge_member_t *member = netd_bridge_members_find(members, interface);
  if (member) {
    TAILQ_REMOVE(&members->head, member, entries);
    members->count--;
    free(member);
    return true;
  }
  return false;
}

bool netd_lagg_members_remove_item(netd_lagg_members_t *members, netd_interface_t *interface) {
  netd_lagg_member_t *member = netd_lagg_members_find(members, interface);
  if (member) {
    TAILQ_REMOVE(&members->head, member, entries);
    members->count--;
    free(member);
    return true;
  }
  return false;
}

/* Clear operations */
void netd_interface_groups_clear(netd_interface_groups_t *groups) {
  netd_interface_group_t *group, *tmp;
  TAILQ_FOREACH_SAFE(group, &groups->head, entries, tmp) {
    TAILQ_REMOVE(&groups->head, group, entries);
    free(group);
  }
  groups->count = 0;
}

void netd_bridge_members_clear(netd_bridge_members_t *members) {
  netd_bridge_member_t *member, *tmp;
  TAILQ_FOREACH_SAFE(member, &members->head, entries, tmp) {
    TAILQ_REMOVE(&members->head, member, entries);
    free(member);
  }
  members->count = 0;
}

void netd_lagg_members_clear(netd_lagg_members_t *members) {
  netd_lagg_member_t *member, *tmp;
  TAILQ_FOREACH_SAFE(member, &members->head, entries, tmp) {
    TAILQ_REMOVE(&members->head, member, entries);
    free(member);
  }
  members->count = 0;
}
