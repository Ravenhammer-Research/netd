#ifndef NETCONF_UTILS_H
#define NETCONF_UTILS_H

#include <net.h>
#include <sys/socket.h>

/* Address parsing functions */
int parse_address(const char *addr_str, struct sockaddr_storage *addr);
int format_address(const struct sockaddr_storage *addr, char *str, size_t len);
int get_address_family(const struct sockaddr_storage *addr);
int get_prefix_length(const struct sockaddr_storage *addr);

/* Output functions */
void print_success(const char *format, ...);
void print_info(const char *format, ...);

#endif /* NETCONF_UTILS_H */ 