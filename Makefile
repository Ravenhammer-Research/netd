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

# Source files
SERVER_SRCS = $(SERVERDIR)/main.c \
              $(SERVERDIR)/debug.c \
              $(SERVERDIR)/utils.c \
              $(SERVERDIR)/system.c \
              $(SERVERDIR)/transaction.c \
              $(SERVERDIR)/vrf.c \
              $(SERVERDIR)/interface.c \
              $(SERVERDIR)/route.c \
              $(SERVERDIR)/config.c \
              $(SERVERDIR)/yang.c \
              $(SERVERDIR)/netconf.c

CLIENT_SRCS = $(CLIENTDIR)/main.c \
              $(CLIENTDIR)/parser.c \
              $(CLIENTDIR)/lexer.c \
              $(CLIENTDIR)/client.c \
              $(CLIENTDIR)/functions.c \
              $(CLIENTDIR)/netconf.c \
              $(CLIENTDIR)/utils.c \
              $(CLIENTDIR)/xml_utils.c \
              $(CLIENTDIR)/display_utils.c \
              $(CLIENTDIR)/interface_table.c \
              $(CLIENTDIR)/vrf_table.c \
              $(CLIENTDIR)/route_table.c

# Object files
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)

# Targets
SERVER_TARGET = netd
CLIENT_TARGET = net

# Libraries
SERVER_LIBS = -lyang
CLIENT_LIBS = -lyang -lreadline -lfl

# Default target
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Server target
$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(SERVER_OBJS) $(SERVER_LIBS)

# Client target
$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(CLIENT_OBJS) $(CLIENT_LIBS)

# Generate parser files
$(CLIENTDIR)/parser.c: $(CLIENTDIR)/parser.y
	cd $(CLIENTDIR) && yacc -d -o parser.c parser.y

$(CLIENTDIR)/lexer.c: $(CLIENTDIR)/lexer.l $(CLIENTDIR)/parser.c
	lex -o $@ $<

# Compilation rules
$(SERVERDIR)/%.o: $(SERVERDIR)/%.c
	$(CC) $(CFLAGS) -I$(SERVERDIR) -c -o $@ $<

$(CLIENTDIR)/%.o: $(CLIENTDIR)/%.c
	$(CC) $(CFLAGS) -I$(CLIENTDIR) -c -o $@ $<

# Clean target
clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) $(SERVER_TARGET) $(CLIENT_TARGET)
	rm -f $(CLIENTDIR)/parser.c $(CLIENTDIR)/lexer.c $(CLIENTDIR)/y.tab.h

# Install target
install: $(SERVER_TARGET) $(CLIENT_TARGET)
	install -m 755 $(SERVER_TARGET) /usr/local/sbin/
	install -m 755 $(CLIENT_TARGET) /usr/local/bin/
	install -d /etc/netd
	install -m 644 $(YANGDIR)/netd.yang /usr/local/share/netd/

# Uninstall target
uninstall:
	rm -f /usr/local/sbin/$(SERVER_TARGET)
	rm -f /usr/local/bin/$(CLIENT_TARGET)
	rm -f /usr/local/share/netd/netd.yang
	rmdir /usr/local/share/netd 2>/dev/null || true

# Dependencies
$(SERVERDIR)/main.o: $(SERVERDIR)/netd.h
$(SERVERDIR)/debug.o: $(SERVERDIR)/netd.h
$(SERVERDIR)/utils.o: $(SERVERDIR)/netd.h
$(SERVERDIR)/system.o: $(SERVERDIR)/netd.h
$(SERVERDIR)/transaction.o: $(SERVERDIR)/netd.h
$(SERVERDIR)/vrf.o: $(SERVERDIR)/netd.h
$(SERVERDIR)/interface.o: $(SERVERDIR)/netd.h
$(SERVERDIR)/route.o: $(SERVERDIR)/netd.h
$(SERVERDIR)/config.o: $(SERVERDIR)/netd.h
$(SERVERDIR)/yang.o: $(SERVERDIR)/netd.h
$(SERVERDIR)/netconf.o: $(SERVERDIR)/netd.h

$(CLIENTDIR)/main.o: $(CLIENTDIR)/net.h
$(CLIENTDIR)/parser.o: $(CLIENTDIR)/net.h $(CLIENTDIR)/parser.c
$(CLIENTDIR)/lexer.o: $(CLIENTDIR)/net.h $(CLIENTDIR)/lexer.c
$(CLIENTDIR)/client.o: $(CLIENTDIR)/net.h
$(CLIENTDIR)/functions.o: $(CLIENTDIR)/net.h
$(CLIENTDIR)/netconf.o: $(CLIENTDIR)/net.h
$(CLIENTDIR)/utils.o: $(CLIENTDIR)/net.h
$(CLIENTDIR)/xml_utils.o: $(CLIENTDIR)/net.h
$(CLIENTDIR)/display_utils.o: $(CLIENTDIR)/net.h
$(CLIENTDIR)/interface_table.o: $(CLIENTDIR)/net.h
$(CLIENTDIR)/vrf_table.o: $(CLIENTDIR)/net.h
$(CLIENTDIR)/route_table.o: $(CLIENTDIR)/net.h
$(CLIENTDIR)/table.o: $(CLIENTDIR)/net.h

# Phony targets
.PHONY: all clean install uninstall 