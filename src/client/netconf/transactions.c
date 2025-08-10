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
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

#include <net.h>
#include <netconf.h>
#include <parser/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Transaction state */
static bool transaction_active = false;
static char transaction_commands[10][256]; /* Up to 10 commands, 256 chars each */
static int transaction_command_count = 0;

/**
 * Begin a new transaction
 */
int transaction_begin(void) {
    if (transaction_active) {
        fprintf(stderr, "Transaction already in progress\n");
        return -1;
    }
    
    transaction_active = true;
    transaction_command_count = 0;
    printf("Transaction begun\n");
    return 0;
}

/**
 * Add a command to the current transaction
 */
int transaction_add_command(const char *command) {
    if (!transaction_active) {
        fprintf(stderr, "No transaction in progress\n");
        return -1;
    }
    
    if (transaction_command_count >= 10) {
        fprintf(stderr, "Transaction command limit reached\n");
        return -1;
    }
    
    strncpy(transaction_commands[transaction_command_count], command, 255);
    transaction_commands[transaction_command_count][255] = '\0';
    transaction_command_count++;
    
    printf("Command added to transaction: %s\n", command);
    return 0;
}

/**
 * Commit the current transaction
 */
int transaction_commit(net_client_t *client) {
    if (!transaction_active) {
        fprintf(stderr, "No transaction in progress\n");
        return -1;
    }
    
    if (transaction_command_count == 0) {
        fprintf(stderr, "No commands in transaction\n");
        return -1;
    }
    
    printf("Committing transaction with %d commands...\n", transaction_command_count);
    
    /* Execute all commands in the transaction */
    for (int i = 0; i < transaction_command_count; i++) {
        command_t cmd;
        if (parse_command(transaction_commands[i], &cmd) == 0) {
            if (execute_command(client, &cmd) < 0) {
                fprintf(stderr, "Transaction failed at command %d: %s\n", i + 1, transaction_commands[i]);
                transaction_rollback();
                return -1;
            }
        }
    }
    
    /* Clear transaction state */
    transaction_active = false;
    transaction_command_count = 0;
    
    printf("Transaction committed successfully\n");
    return 0;
}

/**
 * Rollback the current transaction
 */
int transaction_rollback(void) {
    if (!transaction_active) {
        fprintf(stderr, "No transaction in progress\n");
        return -1;
    }
    
    printf("Rolling back transaction...\n");
    
    /* Clear transaction state */
    transaction_active = false;
    transaction_command_count = 0;
    
    printf("Transaction rolled back\n");
    return 0;
}

/**
 * Check if a transaction is currently active
 */
bool is_transaction_active(void) {
    return transaction_active;
}

/**
 * Get the number of commands in the current transaction
 */
int get_transaction_command_count(void) {
    return transaction_command_count;
} 