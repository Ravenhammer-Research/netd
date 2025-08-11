#ifndef INTERFACE_H
#define INTERFACE_H

#include <netd.h>

/* Interface management */
int interface_create(netd_state_t *state, const char *name,
    interface_type_t type);
int interface_delete(netd_state_t *state, const char *name);
int interface_set_fib(netd_state_t *state, const char *name, uint32_t fib);
int interface_add_group(netd_state_t *state, const char *name,
       const char *group);
int interface_remove_group(netd_state_t *state, const char *name,
          const char *group);
int interface_set_address(netd_state_t *state, const char *name,
         const char *address, int family);
int interface_delete_address(netd_state_t *state, const char *name, int family);
int interface_set_mtu(netd_state_t *state, const char *name, int mtu);
int interface_add(netd_state_t *state, const char *name);
int interface_modify(netd_state_t *state, const char *name,
    const char *property, const char *value);
interface_t *interface_find(netd_state_t *state, const char *name);
int interface_list(netd_state_t *state, interface_type_t type);
int interface_enumerate_system(netd_state_t *state);
char *interface_get_all(netd_state_t *state);
char *create_interfaces_xml_response(netd_state_t *state, const char *message_id);
int handle_get_interfaces_request(netd_state_t *state, const char *request,
    const char *message_id, char **response);

extern int add_pending_interface_set_fib(netd_state_t *state, const char *name,
    uint32_t fib);

#endif /* INTERFACE_H */