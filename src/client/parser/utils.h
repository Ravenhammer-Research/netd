#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include <net.h>

/* Enum conversion functions */
const char *command_type_to_string(command_type_t type);
command_type_t command_type_from_string(const char *str);
const char *object_type_to_string(object_type_t type);
object_type_t object_type_from_string(const char *str);
const char *interface_type_to_string(interface_type_t type);
interface_type_t interface_type_from_string(const char *str);

/* Command parsing functions */
int parse_command(const char *line, command_t *cmd);
int parse_command_yacc(const char *line, command_t *cmd);
int parse_command_simple(const char *line, command_t *cmd);

#endif /* PARSER_UTILS_H */ 