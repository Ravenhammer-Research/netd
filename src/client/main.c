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

#include <net.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/**
 * Print usage information
 * @param progname Program name
 */
static void print_usage(const char *progname) {
  fprintf(stderr, "Usage: %s [OPTIONS] [COMMAND] [ARGUMENTS...]\n", progname);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -d, --debug LEVEL    Set debug level (0-5)\n");
  fprintf(stderr, "  -h, --help           Show this help message\n");
  fprintf(stderr, "Debug levels:\n");
  fprintf(stderr, "  0 = None\n");
  fprintf(stderr, "  1 = Error\n");
  fprintf(stderr, "  2 = Warning\n");
  fprintf(stderr, "  3 = Info\n");
  fprintf(stderr, "  4 = Debug\n");
  fprintf(stderr, "  5 = Trace\n");
  fprintf(stderr, "Run without arguments for interactive mode.\n");
}

/**
 * Main function
 * @param argc Argument count
 * @param argv Argument vector
 * @return 0 on success, 1 on failure
 */
int main(int argc, char *argv[]) {
  net_client_t client;
  bool interactive = false;
  int ret = 0;
  debug_level_t debug_level = DEBUG_NONE;
  int opt;

  /* Parse command line options */
  while ((opt = getopt(argc, argv, "d:h")) != -1) {
    switch (opt) {
    case 'd':
      debug_level = atoi(optarg);
      if (debug_level < DEBUG_NONE || debug_level > DEBUG_DEBUG) {
        fprintf(stderr, "Invalid debug level: %s\n", optarg);
        return 1;
      }
      break;
    case 'h':
      print_usage(argv[0]);
      return 0;
    default:
      print_usage(argv[0]);
      return 1;
    }
  }

  /* Initialize debug logging */
  debug_init(debug_level);
  debug_log(DEBUG_INFO, "Starting net client (debug level: %d)", debug_level);

  /* Check if interactive mode */
  if (optind >= argc) {
    interactive = true;
    debug_log(DEBUG_INFO, "Running in interactive mode");
  } else {
    debug_log(DEBUG_INFO, "Running in command mode");
  }

  /* Initialize client */
  debug_log(DEBUG_DEBUG, "Initializing client");
  if (client_init(&client, interactive) < 0) {
    print_error("Failed to initialize client");
    return 1;
  }

  if (interactive) {
    /* Interactive mode */
    debug_log(DEBUG_DEBUG, "Starting interactive mode");
    ret = interactive_mode(&client);
  } else {
    /* Command line mode */
    debug_log(DEBUG_DEBUG, "Processing command line arguments");
    char *cmd_line = NULL;

    /* Calculate total length needed */
    size_t total_len = 0;
    for (int i = optind; i < argc; i++) {
      total_len += strlen(argv[i]) + 1; /* +1 for space */
    }

    /* Allocate command line buffer */
    cmd_line = malloc(total_len + 1);
    if (!cmd_line) {
      print_error("Failed to allocate command line buffer");
      return 1;
    }
    cmd_line[0] = '\0';

    /* Concatenate all arguments into a single command line */
    for (int i = optind; i < argc; i++) {
      if (i > optind) {
        strcat(cmd_line, " ");
      }
      strcat(cmd_line, argv[i]);
    }

    /* Parse and execute command */
    command_t cmd;
    if (parse_command_yacc(cmd_line, &cmd) < 0) {
      /* Fall back to simple parser if YACC fails */
      debug_log(DEBUG_DEBUG, "YACC parsing failed, trying simple parser");
      if (parse_command(cmd_line, &cmd) < 0) {
        print_error("Failed to parse command: %s", cmd_line);
        ret = 1;
      } else {
        ret = execute_command(&client, &cmd);
      }
    } else {
      ret = execute_command(&client, &cmd);
    }

    /* Clean up */
    free(cmd_line);
  }

  /* Cleanup */
  debug_log(DEBUG_DEBUG, "Cleaning up client");
  client_cleanup(&client);
  debug_log(DEBUG_INFO, "Client shutdown complete");
  return ret;
}