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

%{
#include "net.h"
#include "grammar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex(void);
extern int yyparse(void);
extern FILE *yyin;
extern char *yytext;
extern int yylineno;

%}

%union {
    char *string;
    int integer;
    command_type_t cmd_type;
    object_type_t obj_type;
    interface_type_t if_type;
    int family;
}

%token <string> IDENTIFIER
%token <string> IPV4_ADDRESS
%token <string> IPV6_ADDRESS
%token <string> CIDR_ADDRESS
%token <string> NUMBER
%token <string> STRING

/* Token declarations */
%token PROTOCOL

%token <cmd_type> COMMIT
%token <cmd_type> SAVE
%token <cmd_type> CLEAR
%token <cmd_type> ROLLBACK

%token <string> EXIT
%token <string> QUIT
%token <string> HELP
%token <string> QUESTION_MARK

/* Precedence declarations */
%left COMMIT SAVE CLEAR ROLLBACK
%left EXIT QUIT HELP QUESTION_MARK

%%

/* Top level rule */
command
    : utility_command
    | /* empty */
    ;

/* Utility commands */
utility_command
    : COMMIT {
        if (current_command) {
            current_command->type = CMD_COMMIT;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    | SAVE {
        if (current_command) {
            current_command->type = CMD_SAVE;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    | CLEAR {
        if (current_command) {
            current_command->type = CMD_CLEAR;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    | ROLLBACK {
        if (current_command) {
            current_command->type = CMD_ROLLBACK;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    | EXIT {
        if (current_command) {
            current_command->type = CMD_EXIT;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    | QUIT {
        if (current_command) {
            current_command->type = CMD_QUIT;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    | HELP {
        if (current_command) {
            current_command->type = CMD_HELP;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    | QUESTION_MARK {
        if (current_command) {
            current_command->type = CMD_HELP;
            current_command->object = OBJ_NONE;
            current_command->arg_count = 0;
        }
    }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
    parse_error = 1;
} 