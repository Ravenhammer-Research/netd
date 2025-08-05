# Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Compiler and flags
CC = cc
CFLAGS = -std=c99 -Wall -Wextra -Werror -O2 -g -I/usr/include -I/usr/local/include
LDFLAGS = -L/usr/local/lib

# Directories
SRCDIR = src
SERVERDIR = $(SRCDIR)/server
CLIENTDIR = $(SRCDIR)/client
YANGDIR = yang
BUILDDIR = build
SERVER_BUILDDIR = $(BUILDDIR)/server
CLIENT_BUILDDIR = $(BUILDDIR)/client

# VPATH for source files
VPATH = $(SERVERDIR):$(CLIENTDIR)

# Source files
SERVER_SRCS = $(SERVERDIR)/main.c \
              $(SERVERDIR)/debug.c \
              $(SERVERDIR)/utils.c \
              $(SERVERDIR)/freebsd_system.c \
              $(SERVERDIR)/transaction.c \
              $(SERVERDIR)/netconf_vrf.c \
              $(SERVERDIR)/netconf_interface.c \
              $(SERVERDIR)/netconf_route.c \
              $(SERVERDIR)/config.c \
              $(SERVERDIR)/yang.c \
              $(SERVERDIR)/netconf.c

CLIENT_SRCS = $(CLIENTDIR)/main.c \
              $(CLIENT_BUILDDIR)/parser.c \
              $(CLIENT_BUILDDIR)/lexer.c \
              $(CLIENTDIR)/client.c \
              $(CLIENTDIR)/functions.c \
              $(CLIENTDIR)/netconf.c \
              $(CLIENTDIR)/yang.c \
              $(CLIENTDIR)/utils.c \
              $(CLIENTDIR)/xml_utils.c \
              $(CLIENTDIR)/table_utils.c \
              $(CLIENTDIR)/if_table.c \
              $(CLIENTDIR)/ifgrp_bridge_table.c \
              $(CLIENTDIR)/ifgrp_table.c \
              $(CLIENTDIR)/ifgrp_wlan_table.c \
              $(CLIENTDIR)/vrf_table.c \
              $(CLIENTDIR)/route_table.c

# Object files (in build directory)
SERVER_OBJS = $(SERVER_BUILDDIR)/main.o \
              $(SERVER_BUILDDIR)/debug.o \
              $(SERVER_BUILDDIR)/utils.o \
              $(SERVER_BUILDDIR)/freebsd_system.o \
              $(SERVER_BUILDDIR)/transaction.o \
              $(SERVER_BUILDDIR)/netconf_vrf.o \
              $(SERVER_BUILDDIR)/netconf_interface.o \
              $(SERVER_BUILDDIR)/netconf_route.o \
              $(SERVER_BUILDDIR)/config.o \
              $(SERVER_BUILDDIR)/yang.o \
              $(SERVER_BUILDDIR)/netconf.o

CLIENT_OBJS = $(CLIENT_BUILDDIR)/main.o \
              $(CLIENT_BUILDDIR)/parser.o \
              $(CLIENT_BUILDDIR)/lexer.o \
              $(CLIENT_BUILDDIR)/client.o \
              $(CLIENT_BUILDDIR)/functions.o \
              $(CLIENT_BUILDDIR)/netconf.o \
              $(CLIENT_BUILDDIR)/yang.o \
              $(CLIENT_BUILDDIR)/utils.o \
              $(CLIENT_BUILDDIR)/xml_utils.o \
              $(CLIENT_BUILDDIR)/table_utils.o \
              $(CLIENT_BUILDDIR)/if_table.o \
              $(CLIENT_BUILDDIR)/ifgrp_bridge_table.o \
              $(CLIENT_BUILDDIR)/ifgrp_table.o \
              $(CLIENT_BUILDDIR)/ifgrp_wlan_table.o \
              $(CLIENT_BUILDDIR)/vrf_table.o \
              $(CLIENT_BUILDDIR)/route_table.o

# Targets
SERVER_TARGET = $(BUILDDIR)/netd
CLIENT_TARGET = $(BUILDDIR)/net

# Libraries
SERVER_LIBS = -lyang -lexpat
CLIENT_LIBS = -lyang -lreadline -lfl -lexpat

# Default target
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Server target
$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(SERVER_OBJS) $(SERVER_LIBS)

# Client target
$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(CLIENT_OBJS) $(CLIENT_LIBS)

# Generate parser files in client build directory
$(CLIENT_BUILDDIR)/parser.c: $(CLIENTDIR)/parser.y
	@mkdir -p $(CLIENT_BUILDDIR)
	cd $(CLIENTDIR) && yacc -d parser.y
	@mv $(CLIENTDIR)/y.tab.c $(CLIENT_BUILDDIR)/parser.c
	@mv $(CLIENTDIR)/y.tab.h $(CLIENT_BUILDDIR)/y.tab.h

$(CLIENT_BUILDDIR)/lexer.c: $(CLIENTDIR)/lexer.l $(CLIENT_BUILDDIR)/parser.c
	@mkdir -p $(CLIENT_BUILDDIR)
	cd $(CLIENTDIR) && lex -o ../../$(CLIENT_BUILDDIR)/lexer.c lexer.l

# Compilation rules for server objects
$(SERVER_BUILDDIR)/main.o: $(SERVERDIR)/main.c
	@mkdir -p $(SERVER_BUILDDIR)
	$(CC) $(CFLAGS) -I$(SERVERDIR) -c -o $@ $(SERVERDIR)/main.c

$(SERVER_BUILDDIR)/debug.o: $(SERVERDIR)/debug.c
	@mkdir -p $(SERVER_BUILDDIR)
	$(CC) $(CFLAGS) -I$(SERVERDIR) -c -o $@ $(SERVERDIR)/debug.c

$(SERVER_BUILDDIR)/utils.o: $(SERVERDIR)/utils.c
	@mkdir -p $(SERVER_BUILDDIR)
	$(CC) $(CFLAGS) -I$(SERVERDIR) -c -o $@ $(SERVERDIR)/utils.c

$(SERVER_BUILDDIR)/freebsd_system.o: $(SERVERDIR)/freebsd_system.c
	@mkdir -p $(SERVER_BUILDDIR)
	$(CC) $(CFLAGS) -I$(SERVERDIR) -c -o $@ $(SERVERDIR)/freebsd_system.c

$(SERVER_BUILDDIR)/transaction.o: $(SERVERDIR)/transaction.c
	@mkdir -p $(SERVER_BUILDDIR)
	$(CC) $(CFLAGS) -I$(SERVERDIR) -c -o $@ $(SERVERDIR)/transaction.c

$(SERVER_BUILDDIR)/netconf_vrf.o: $(SERVERDIR)/netconf_vrf.c
	@mkdir -p $(SERVER_BUILDDIR)
	$(CC) $(CFLAGS) -I$(SERVERDIR) -c -o $@ $(SERVERDIR)/netconf_vrf.c

$(SERVER_BUILDDIR)/netconf_interface.o: $(SERVERDIR)/netconf_interface.c
	@mkdir -p $(SERVER_BUILDDIR)
	$(CC) $(CFLAGS) -I$(SERVERDIR) -c -o $@ $(SERVERDIR)/netconf_interface.c

$(SERVER_BUILDDIR)/netconf_route.o: $(SERVERDIR)/netconf_route.c
	@mkdir -p $(SERVER_BUILDDIR)
	$(CC) $(CFLAGS) -I$(SERVERDIR) -c -o $@ $(SERVERDIR)/netconf_route.c

$(SERVER_BUILDDIR)/config.o: $(SERVERDIR)/config.c
	@mkdir -p $(SERVER_BUILDDIR)
	$(CC) $(CFLAGS) -I$(SERVERDIR) -c -o $@ $(SERVERDIR)/config.c

$(SERVER_BUILDDIR)/yang.o: $(SERVERDIR)/yang.c
	@mkdir -p $(SERVER_BUILDDIR)
	$(CC) $(CFLAGS) -I$(SERVERDIR) -c -o $@ $(SERVERDIR)/yang.c

$(SERVER_BUILDDIR)/netconf.o: $(SERVERDIR)/netconf.c
	@mkdir -p $(SERVER_BUILDDIR)
	$(CC) $(CFLAGS) -I$(SERVERDIR) -c -o $@ $(SERVERDIR)/netconf.c

# Compilation rules for client objects
$(CLIENT_BUILDDIR)/main.o: $(CLIENTDIR)/main.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/main.c

$(CLIENT_BUILDDIR)/client.o: $(CLIENTDIR)/client.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/client.c

$(CLIENT_BUILDDIR)/functions.o: $(CLIENTDIR)/functions.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/functions.c

$(CLIENT_BUILDDIR)/netconf.o: $(CLIENTDIR)/netconf.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/netconf.c

$(CLIENT_BUILDDIR)/yang.o: $(CLIENTDIR)/yang.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/yang.c

$(CLIENT_BUILDDIR)/utils.o: $(CLIENTDIR)/utils.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/utils.c

$(CLIENT_BUILDDIR)/xml_utils.o: $(CLIENTDIR)/xml_utils.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/xml_utils.c

$(CLIENT_BUILDDIR)/table_utils.o: $(CLIENTDIR)/table_utils.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/table_utils.c

$(CLIENT_BUILDDIR)/if_table.o: $(CLIENTDIR)/if_table.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/if_table.c

$(CLIENT_BUILDDIR)/ifgrp_bridge_table.o: $(CLIENTDIR)/ifgrp_bridge_table.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/ifgrp_bridge_table.c

$(CLIENT_BUILDDIR)/ifgrp_table.o: $(CLIENTDIR)/ifgrp_table.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/ifgrp_table.c

$(CLIENT_BUILDDIR)/ifgrp_wlan_table.o: $(CLIENTDIR)/ifgrp_wlan_table.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/ifgrp_wlan_table.c

$(CLIENT_BUILDDIR)/vrf_table.o: $(CLIENTDIR)/vrf_table.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/vrf_table.c

$(CLIENT_BUILDDIR)/route_table.o: $(CLIENTDIR)/route_table.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENTDIR)/route_table.c

# Compilation rules for generated parser/lexer objects (from client build directory)
$(CLIENT_BUILDDIR)/parser.o: $(CLIENT_BUILDDIR)/parser.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENT_BUILDDIR)/parser.c

$(CLIENT_BUILDDIR)/lexer.o: $(CLIENT_BUILDDIR)/lexer.c
	@mkdir -p $(CLIENT_BUILDDIR)
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $(CLIENT_BUILDDIR)/lexer.c

# Clean target
clean:
	rm -rf $(BUILDDIR)

# Install target
install: $(SERVER_TARGET) $(CLIENT_TARGET)
	install -m 755 $(SERVER_TARGET) /usr/local/sbin/
	install -m 755 $(CLIENT_TARGET) /usr/local/bin/
	install -d /etc/netd
	install -m 644 $(YANGDIR)/netd.yang /usr/local/share/netd/

# Uninstall target
uninstall:
	rm -f /usr/local/sbin/netd
	rm -f /usr/local/bin/net
	rm -f /usr/local/share/netd/netd.yang
	rmdir /usr/local/share/netd 2>/dev/null || true

# Dependencies
$(SERVER_BUILDDIR)/main.o: $(SERVERDIR)/netd.h
$(SERVER_BUILDDIR)/debug.o: $(SERVERDIR)/netd.h
$(SERVER_BUILDDIR)/utils.o: $(SERVERDIR)/netd.h
$(SERVER_BUILDDIR)/freebsd_system.o: $(SERVERDIR)/netd.h
$(SERVER_BUILDDIR)/transaction.o: $(SERVERDIR)/netd.h
$(SERVER_BUILDDIR)/netconf_vrf.o: $(SERVERDIR)/netd.h
$(SERVER_BUILDDIR)/netconf_interface.o: $(SERVERDIR)/netd.h
$(SERVER_BUILDDIR)/netconf_route.o: $(SERVERDIR)/netd.h
$(SERVER_BUILDDIR)/config.o: $(SERVERDIR)/netd.h
$(SERVER_BUILDDIR)/yang.o: $(SERVERDIR)/netd.h
$(SERVER_BUILDDIR)/netconf.o: $(SERVERDIR)/netd.h

$(CLIENT_BUILDDIR)/main.o: $(CLIENTDIR)/net.h
$(CLIENT_BUILDDIR)/parser.o: $(CLIENTDIR)/net.h $(CLIENT_BUILDDIR)/parser.c
$(CLIENT_BUILDDIR)/lexer.o: $(CLIENTDIR)/net.h $(CLIENT_BUILDDIR)/lexer.c
$(CLIENT_BUILDDIR)/client.o: $(CLIENTDIR)/net.h
$(CLIENT_BUILDDIR)/functions.o: $(CLIENTDIR)/net.h
$(CLIENT_BUILDDIR)/netconf.o: $(CLIENTDIR)/net.h
$(CLIENT_BUILDDIR)/yang.o: $(CLIENTDIR)/net.h
$(CLIENT_BUILDDIR)/utils.o: $(CLIENTDIR)/net.h
$(CLIENT_BUILDDIR)/xml_utils.o: $(CLIENTDIR)/net.h
$(CLIENT_BUILDDIR)/vrf_table.o: $(CLIENTDIR)/net.h
$(CLIENT_BUILDDIR)/route_table.o: $(CLIENTDIR)/net.h
$(CLIENT_BUILDDIR)/table_utils.o: $(CLIENTDIR)/net.h

# Phony targets
.PHONY: all clean install uninstall 