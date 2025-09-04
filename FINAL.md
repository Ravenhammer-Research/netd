# NETD Final Implementation Specification

## Overview
NETD is a network configuration daemon that provides NETCONF server functionality with a CLI client for FreeBSD systems. This document consolidates the complete implementation specification, architecture, and technical requirements.

## Project Structure

### Shared Library
- `shared/CMakeLists.txt`
- `shared/include/CMakeLists.txt`
- `shared/src/CMakeLists.txt`

- `shared/src/vrf.cpp` (abstract) should have toYang/fromYang methods 
- `shared/src/route.cpp` (abstract) should have toYang/fromYang methods 
- `shared/src/request.cpp` (abstract) get-config/edit-config/commit/etc (can contain interfaces vrfs or route references) should have toYang/fromYang methods 
- `shared/src/response.cpp` (abstract) get-config/edit-config/commit/etc (can contain interfaces vrfs or route references) should have toYang/fromYang methods 
- `shared/src/logger.cpp` INFO/WARNING/ERROR/DEBUG (singleton, thread safe, supports user-defined callbacks)
- `shared/src/yang.cpp` (abstract) methods for loading yang schemas, setting up the context, etc, find libraries in PkgConfig/CMake, use lib/include dirs provided by PkgConfig
- `shared/src/address.cpp` (abstract) should have toYang/fromYang methods for ietf-ip integration
- `shared/src/yang.cpp` (abstract) methods for loading yang schemas, setting up the context, etc, find libraries in PkgConfig/CMake, use lib/include dirs provided by PkgConfig, plus static utility functions for YANG data conversion: yangToXml, yangToJson, xmlToYang, jsonToYang
- `shared/src/exception.cpp` (custom exception classes including not_implemented_error)


- `shared/include/vrf.hpp` (abstract sysctl net.fibs)
- `shared/include/route.hpp` (abstract /usr/include/netlink/)
- `shared/include/request.hpp` (abstract, netconf server/client functionality, should have toNetconfRequest and fromNetconfRequest)
- `shared/include/response.hpp` (abstract, netconf server/client functionality, should have toNetconfResponse and fromNetconfResponse)
- `shared/include/logger.hpp` (singleton, thread safe)
- `shared/include/yang.hpp` (abstract)
- `shared/include/exception.hpp` (custom exception classes)

#### Base Interface Classes (Mixin Classes)
- `shared/include/interface/base/ether.hpp` (base ethernet functionality: addAddress, removeAddress, addGroup, removeGroup, setMTU, setFlags, up, down, setVRF)
- `shared/include/interface/base/tunnel.hpp` (base tunnel functionality: setLocalAddr, setRemoteAddr, setTunnelVRF, setTunnelMTU)
- `shared/include/interface/base/master.hpp` (base master/slave functionality: addSlave, removeSlave for bridge and lagg interfaces)

#### Base Classes
- `shared/include/base/serialization.hpp` (base YANG serialization functionality: toYang, fromYang)

#### Shared Interface Classes
- `shared/include/ethernet.hpp` (inherits from interface::base::Ether, provides ethernet interface contract)
- `shared/include/tunnel.hpp` (inherits from interface::base::Ether and interface::base::Tunnel, provides tunnel interface contract)
- `shared/include/master.hpp` (inherits from interface::base::Ether and interface::base::Master, provides master interface contract for bridge/LAG)

#### Shared Interface Implementation
- `shared/src/interface/bridge.cpp` (implements shared bridge interface with YANG serialization)
- `shared/src/interface/epair.cpp` (implements shared epair interface with YANG serialization)
- `shared/src/interface/ethernet.cpp` (implements shared ethernet interface with YANG serialization)
- `shared/src/interface/lagg.cpp` (implements shared lagg interface with YANG serialization)
- `shared/src/interface/ppp.cpp` (implements shared ppp interface with YANG serialization)
- `shared/src/interface/tap.cpp` (implements shared tap interface with YANG serialization)
- `shared/src/interface/tun.cpp` (implements shared tun interface with YANG serialization)
- `shared/src/interface/vlan.cpp` (implements shared vlan interface with YANG serialization)
- `shared/src/interface/vxlan.cpp` (implements shared vxlan interface with YANG serialization)
- `shared/src/interface/wireguard.cpp` (implements shared wireguard interface with YANG serialization)
- `shared/src/interface/80211.cpp` (implements shared 802.11 wireless interface with YANG serialization)

### YANG Schemas
- `shared/yang/netd-interface.yang` (base interface augmentations for ietf-interfaces)
- `shared/yang/netd-interface-bridge.yang` (augments ietf-interfaces for bridge interfaces)
- `shared/yang/netd-interface-epair.yang` (augments ietf-interfaces for epair interfaces)
- `shared/yang/netd-interface-ethernet.yang` (augments ietf-interfaces for ethernet interfaces)
- `shared/yang/netd-interface-lagg.yang` (augments ietf-interfaces for LAG interfaces)
- `shared/yang/netd-interface-ppp.yang` (augments ietf-interfaces for PPP interfaces)
- `shared/yang/netd-interface-tap.yang` (augments ietf-interfaces for TAP interfaces)
- `shared/yang/netd-interface-tun.yang` (augments ietf-interfaces for TUN interfaces)
- `shared/yang/netd-interface-vlan.yang` (augments ietf-interfaces for VLAN interfaces)
- `shared/yang/netd-interface-vxlan.yang` (augments ietf-interfaces for VXLAN interfaces)
- `shared/yang/netd-interface-wireguard.yang` (augments ietf-interfaces for WireGuard interfaces)
- `shared/yang/netd-interface-80211.yang` (augments ietf-interfaces for 802.11 wireless interfaces)

### Standard YANG Schemas
- `yang/standard/ietf/RFC/iana-if-type@2014-05-08.yang`
- `yang/standard/ietf/RFC/ietf-interfaces@2018-02-20.yang`  
- `yang/standard/ietf/RFC/ietf-network-instance@2019-01-21.yang`
- `yang/standard/ietf/RFC/ietf-routing@2018-03-13.yang`

### Server
- `server/CMakeLists.txt`
- `server/include/CMakeLists.txt`
- `server/src/CMakeLists.txt`
- `server/src/netconf.cpp` (unix socket listener only; see libnetconf2 headers they have this, find libraries in PkgConfig/CMake, use lib/include dirs provided by PkgConfig)
- `server/src/store.cpp` (candidate, running, startup, startup will use the freebsd objects to enumerate config, and the base classes of the freebsd objects will have the toYang logic for creating the config data which can be committed to candidate when a config doesn't already exist. User can then run commit to save the configuration to running.)

### Client
- `client/CMakeLists.txt`
- `client/include/CMakeLists.txt`
- `client/src/CMakeLists.txt`
- `client/src/netconf.cpp` (unix socket client only see libnetconf2 headers they have this, find libraries in PkgConfig/CMake, use lib/include dirs provided by PkgConfig)) 
- `client/src/terminal.cpp` (should use /usr/include/curses.h)

### Client NETCONF Functionality
- `client/include/netconf.hpp` (NETCONF client operations: get-config, edit-config, commit, etc.)
- `client/src/netconf.cpp` (NETCONF client implementation with unix socket communication)
- `client/include/terminal.hpp` (Terminal interface for interactive CLI)
- `client/src/terminal.cpp` (Terminal implementation using curses)

#### Parser
- `client/parser/CMakeLists.txt`
- `client/parser/get.y` (yacc)
- `client/parser/set.y` (yacc)
- `client/parser/commit.y` (yacc)
- `client/parser/delete.y` (yacc)
- `client/parser/get.l` (flex)
- `client/parser/set.l` (flex)
- `client/parser/commit.l` (flex)
- `client/parser/delete.l` (flex)

### FreeBSD Implementation
**⚠️ IMPORTANT: FreeBSD-specific code is ONLY used by the server, never by client or shared libraries.**
- `freebsd/CMakeLists.txt`

#### Interface Headers
- `freebsd/include/interface/bridge.hpp` (extends shared base interface classes using multiple inheritance, adds functionality for FreeBSD acquisition and configuration) 
- `freebsd/include/interface/epair.hpp` (extends shared base interface classes using multiple inheritance, adds functionality for FreeBSD acquisition and configuration) 
- `freebsd/include/interface/ethernet.hpp` (extends shared base interface classes using multiple inheritance, adds functionality for FreeBSD acquisition and configuration) 
- `freebsd/include/interface/lagg.hpp` (extends shared base interface classes using multiple inheritance, adds functionality for FreeBSD acquisition and configuration) 
- `freebsd/include/interface/ppp.hpp` (extends shared base interface classes using multiple inheritance, adds functionality for FreeBSD acquisition and configuration) 
- `freebsd/include/interface/tap.hpp` (extends shared base interface classes using multiple inheritance, adds functionality for FreeBSD acquisition and configuration) 
- `freebsd/include/interface/tun.hpp` (extends shared base interface classes using multiple inheritance, adds functionality for FreeBSD acquisition and configuration) 
- `freebsd/include/interface/vlan.hpp` (extends shared base interface classes using multiple inheritance, adds functionality for FreeBSD acquisition and configuration) 
- `freebsd/include/interface/vxlan.hpp` (extends shared base interface classes using multiple inheritance, adds functionality for FreeBSD acquisition and configuration) 
- `freebsd/include/interface/wireguard.hpp` (extends shared base interface classes using multiple inheritance, adds functionality for FreeBSD acquisition and configuration) 
- `freebsd/include/interface/80211.hpp` (extends shared base interface classes using multiple inheritance, adds functionality for FreeBSD acquisition and configuration)

#### Interface Implementation
- `freebsd/src/interface/CMakeLists.txt`
- `freebsd/src/interface/bridge.cpp` (/usr/include/net/if_bridgevar.h)
- `freebsd/src/interface/epair.cpp` (/usr/include/net/if_clone.h maybe)
- `freebsd/src/interface/ethernet.cpp` (/usr/include/netinet/if_ether.h)
- `freebsd/src/interface/lagg.cpp` (/usr/include/net/if_lagg.h)
- `freebsd/src/interface/ppp.cpp` (/usr/include/net/ppp_defs.h)
- `freebsd/src/interface/tap.cpp` (/usr/include/net/if_tap.h)
- `freebsd/src/interface/tun.cpp` (/usr/include/net/if_tun.h)
- `freebsd/src/interface/vlan.cpp` (/usr/include/net/if_vlan_var.h)
- `freebsd/src/interface/vxlan.cpp` (/usr/include/net/if_vxlan.h)
- `freebsd/src/interface/wireguard.cpp` 
- `freebsd/src/interface/80211.cpp` (/usr/include/net80211/ /usr/include/lib80211)

### Build System
- `cmake/Yacc.cmake` (processing .y) 
- `cmake/Lex.cmake` (processing .l)
- `cmake/YangSchema.cmake` (processing in-tree .yang)

### Include Path Strategy
All source files use full path includes with `<>` syntax, for example:
- `#include <freebsd/include/interface/tun.hpp>`
- `#include <shared/include/interface.hpp>`
- `#include <server/include/netconf.hpp>`

Each component's CMakeLists.txt sets only one include directory: `${CMAKE_CURRENT_SOURCE_DIR}/../..` (project root)

## Design Principles

### Base Class Implementation
- The base classes should implement properties/getters/setters for holding this data as well as any data provided in the augment yang files, but using the standard/ietf first should always be preferred before creating augments.

### Object-Oriented Design
**⚠️ IMPORTANT: Use proper classes for complex types instead of strings for everything:**
- **Address classes**: Create proper `Address` classes for IPv4/IPv6 addresses instead of string representations
- **Interface references**: Bridge members, LAG ports, etc. should be pointers to `Interface` objects, not string names
- **Network objects**: Subnets, routes, and other network entities should have dedicated classes
- **Configuration objects**: Complex configurations should be represented as objects with proper validation

### Coding Standards
- No excessive logging messages
- FreeBSD license every file (Paige Thompson / Ravenhammer Research (paige@paige.bio))
- No compound word filenames 
- No underscores in filenames
- No cout
- No exec/popen calls; system libraries only

### Dependencies
- You can look in `/usr/local/include/libnetconf2` and `/usr/local/include/libyang` but you should in no way rely on these paths being consistent; just so you can look at the headers

#### libyang Headers
- `context.h`
- `dict.h`
- `hash_table.h`
- `in.h`
- `libyang.h`
- `log.h`
- `ly_config.h`
- `metadata.h`
- `out.h`
- `parser_data.h`
- `parser_schema.h`
- `plugins.h`
- `plugins_exts.h`
- `plugins_types.h`
- `printer_data.h`
- `printer_schema.h`
- `set.h`
- `tree.h`
- `tree_data.h`
- `tree_edit.h`
- `tree_schema.h`
- `version.h`

#### libnetconf2 Headers
- `log.h`
- `messages_client.h`
- `messages_server.h`
- `netconf.h`
- `server_config.h`
- `session.h`
- `session_client.h`
- `session_client_ch.h`
- `session_server.h`
- `session_server_ch.h`

#### nc_* Headers
- `/usr/local/include/nc_client.h`
- `/usr/local/include/nc_server.h`
- `/usr/local/include/nc_version.h`

## Architecture

### Data Flow
`client -> netconf request <-> server -> freebsd api calls`

### Component Isolation
- **Shared Library**: Platform-agnostic interfaces and utilities
- **Client**: Pure NETCONF client with terminal interface, no platform-specific code
- **Server**: NETCONF server that uses FreeBSD-specific implementations
- **FreeBSD Implementation**: Server-only code that extends shared interfaces with FreeBSD system calls

### Client Features
- Client should support interactive and oneshot modes

### Logger Usage
- **Client**: Logger callback should use `terminal.write` for output to terminal interface
- **Server**: Logger callback should use `fprintf(stderr, ...)` for output to stderr
- Logger supports user-defined callbacks to allow different output handling per component

## Command Reference

### Show Commands

#### VRF Information
- `net> show vrf`

#### Routes for Specific VRF
- `net> show vrf id <fib> protocol static`
- `net> show vrf id <fib> protocol static inet`
- `net> show vrf id <fib> protocol static inet6`
- `net> show route protocol static`  # same as "show vrf id 0 protocol static"

#### Interface Information
- `net> show interface`
- `net> show interface type <type>`
- `net> show interface group <group>`
- `net> show interface <type> <name>`

### Configuration Commands

#### Static Routes
- `net> set vrf id <fib> protocol static <inet|inet6> host <host> gateway <gateway> [iface <interface>]`
- `net> set vrf id <fib> protocol static <inet|inet6> host <host> iface <interface>`
- `net> set vrf id <fib> protocol static <inet|inet6> network <network> gateway <gateway> [iface <interface>]`
- `net> set vrf id <fib> protocol static <inet|inet6> network <network> iface <interface>`

- `net> delete vrf id <fib> protocol static <inet|inet6> network <network> gateway <gateway>`

#### Interface Management
- `net> set interface type <type> name <name> vrf id <id>`
- `net> set interface type <type> name <name> address <inet|inet6> <address>`
- `net> set interface type <type> name <name> mtu <size>`
- `net> set interface type <type> name <name> group <group_name>`

#### Bridge Operations
- `net> set interface type bridge name <name> member <interface_name>`
- `net> delete interface name <name> member`
- `net> delete interface name <name> member <interface_name>`

#### LAG Operations
- `net> set interface type lagg name <name> laggproto <protocol>`
- `net> set interface type lagg name <name> laggport <interface_name>`
- `net> delete interface name <name> laggport`
- `net> delete interface name <name> laggport <interface_name>`

#### Ethernet Pair Operations
- `net> set interface type epair name <name> peer <a|b>`
- `net> set interface type epair name <name> peer <a|b> vrf id <id>`
- `net> set interface type epair name <name> peer <a|b> address <inet|inet6> <address>`

#### VLAN Operations
- `net> set interface type vlan name <name> vlandev <interface>`
- `net> set interface type vlan name <name> vlanproto <protocol>`
- `net> set interface type vlan name <name> layer2 vlan id <id>`

#### VXLAN Operations
- `net> set interface type vxlan name <name> vxlanid <id>`
- `net> set interface type vxlan name <name> tunnel <inet|inet6> vxlanlocal <addr_local> vxlanremote <addr_remote>`
- `net> set interface type vxlan name <name> vxlandev <interface>`

#### GIF Tunnel Operations
- `net> set interface type gif name <name> tunnel <inet|inet6> local <addr_local> remote <addr_remote>`
- `net> set interface type gif name <name> tunnelvrf id <id>`

#### Interface Deletion
- `net> delete interface name <name>`
- `net> delete interface name <name> address <inet|inet6>`

#### Configuration Management
- `net> commit`

#### VRF Creation
- `net> set vrf id <fib> table <table_number>`
- `net> set vrf id <fib> name <name>`

## FreeBSD System Headers

### Core Networking Headers

#### Interface Management
- `/usr/include/net/if.h` - Basic interface definitions and structures
- `/usr/include/net/if_var.h` - Interface variables and statistics
- `/usr/include/net/if_private.h` - Private interface structures
- `/usr/include/net/if_clone.h` - Interface cloning (for epair, tun, tap)
- `/usr/include/net/if_mib.h` - Interface MIB information

#### Interface Type Specific Headers
- `/usr/include/net/if_bridgevar.h` - Bridge interface variables and structures
- `/usr/include/net/if_lagg.h` - Link aggregation interface
- `/usr/include/net/if_tap.h` - TAP interface (Ethernet tap)
- `/usr/include/net/if_tun.h` - TUN interface (IP tunnel)
- `/usr/include/net/if_vlan_var.h` - VLAN interface variables
- `/usr/include/net/if_vxlan.h` - VXLAN interface
- `/usr/include/netinet/if_ether.h` - Ethernet interface definitions
- `/usr/include/net/ppp_defs.h` - PPP interface definitions

#### Wireless (802.11) Headers
- `/usr/include/net80211/ieee80211_var.h` - 802.11 interface variables
- `/usr/include/net80211/ieee80211_node.h` - 802.11 node management
- `/usr/include/net80211/ieee80211_proto.h` - 802.11 protocol definitions
- `/usr/include/net80211/ieee80211_input.h` - 802.11 input processing
- `/usr/include/net80211/ieee80211_scan.h` - 802.11 scanning
- `/usr/include/lib80211/ieee80211_ioctl.h` - 802.11 ioctl definitions

### Routing and VRF Headers

#### Routing Infrastructure
- `/usr/include/net/route.h` - Core routing definitions
- `/usr/include/net/route/route_var.h` - Routing variables
- `/usr/include/net/route/route_ctl.h` - Routing control
- `/usr/include/net/route/nhop.h` - Next hop structures
- `/usr/include/net/route/nhop_var.h` - Next hop variables
- `/usr/include/net/route/fib_algo.h` - FIB algorithms

#### FIB (Forwarding Information Base)
- `/usr/include/netinet/in_fib.h` - IPv4 FIB definitions
- `/usr/include/netinet6/in6_fib.h` - IPv6 FIB definitions

#### Network Instances
- `/usr/include/net/radix.h` - Radix tree for routing tables

### System Control and Configuration

#### System Control
- `/usr/include/sys/sysctl.h` - System control interface
- `/usr/include/sys/socket.h` - Socket interface
- `/usr/include/sys/ioctl.h` - I/O control operations

#### Network Configuration
- `/usr/include/net/pfil.h` - Packet filter interface
- `/usr/include/net/pfvar.h` - Packet filter variables

### Protocol Headers

#### Internet Protocols
- `/usr/include/netinet/in.h` - IPv4 definitions
- `/usr/include/netinet/in_var.h` - IPv4 variables
- `/usr/include/netinet6/in6.h` - IPv6 definitions
- `/usr/include/netinet6/in6_var.h` - IPv6 variables
- `/usr/arpa/inet.h` - Internet address manipulation

#### Network Utilities
- `/usr/include/net/ethernet.h` - Ethernet utilities
- `/usr/include/net/if_strings.h` - Interface string utilities

### Additional Headers

#### Network Graph (Netgraph)
- `/usr/include/netgraph/netgraph.h` - Netgraph framework
- `/usr/include/netgraph/ng_bridge.h` - Netgraph bridge node
- `/usr/include/netgraph/ng_vlan.h` - Netgraph VLAN node
- `/usr/include/netgraph/ng_ether.h` - Netgraph Ethernet node

#### Packet Capture
- `/usr/include/pcap/pcap.h` - Packet capture interface
- `/usr/include/pcap/vlan.h` - VLAN packet capture

### External Library Headers

#### libnetconf2 Headers
**⚠️ IMPORTANT: Do NOT hardcode these paths in CMake. Use pkg-config to find libnetconf2 dynamically.**
- `/usr/local/include/libnetconf2/log.h` - Logging functionality
- `/usr/local/include/libnetconf2/messages_client.h` - Client message handling
- `/usr/local/include/libnetconf2/messages_server.h` - Server message handling
- `/usr/local/include/libnetconf2/netconf.h` - Core NETCONF definitions
- `/usr/local/include/libnetconf2/server_config.h` - Server configuration
- `/usr/include/libnetconf2/session.h` - Session management
- `/usr/local/include/libnetconf2/session_client.h` - Client session handling
- `/usr/local/include/libnetconf2/session_client_ch.h` - Client channel session
- `/usr/local/include/libnetconf2/session_server.h` - Server session handling
- `/usr/local/include/libnetconf2/session_server_ch.h` - Server channel session

#### libyang Headers
**⚠️ IMPORTANT: Do NOT hardcode these paths in CMake. Use pkg-config to find libyang dynamically.**
- `/usr/local/include/libyang/context.h` - YANG context management
- `/usr/local/include/libyang/dict.h` - Dictionary operations
- `/usr/local/include/libyang/hash_table.h` - Hash table utilities
- `/usr/local/include/libyang/in.h` - Input processing
- `/usr/local/include/libyang/libyang.h` - Main libyang header
- `/usr/local/include/libyang/log.h` - Logging functionality
- `/usr/local/include/libyang/ly_config.h` - Configuration management
- `/usr/local/include/libyang/metadata.h` - Metadata handling
- `/usr/local/include/libyang/out.h` - Output processing
- `/usr/local/include/libyang/parser_data.h` - Data parsing
- `/usr/local/include/libyang/parser_schema.h` - Schema parsing
- `/usr/local/include/libyang/plugins.h` - Plugin system
- `/usr/local/include/libyang/plugins_exts.h` - Plugin extensions
- `/usr/local/include/libyang/plugins_types.h` - Plugin types
- `/usr/local/include/libyang/printer_data.h` - Data printing
- `/usr/local/include/libyang/printer_schema.h` - Schema printing
- `/usr/local/include/libyang/set.h` - Set operations
- `/usr/local/include/libyang/tree.h` - Tree data structures
- `/usr/local/include/libyang/tree_data.h` - Tree data operations
- `/usr/local/include/libyang/tree_edit.h` - Tree editing
- `/usr/local/include/libyang/tree_schema.h` - Schema tree operations
- `/usr/local/include/libyang/version.h` - Version information

#### nc_* Headers
**⚠️ IMPORTANT: Do NOT hardcode these paths in CMake. Use pkg-config to find these libraries dynamically.**
- `/usr/local/include/nc_client.h` - NETCONF client utilities
- `/usr/local/include/nc_server.h` - NETCONF server utilities
- `/usr/local/include/nc_version.h` - Version information

## Source Code Examples

### FreeBSD System Utilities Source Code
The following FreeBSD system utilities provide excellent examples of network interface management and routing implementation:

#### ifconfig Source Code (`/usr/src/sbin/ifconfig/`)
- **Main Files**: `ifconfig.c`, `ifconfig.h` - Core interface configuration logic
- **Address Family Support**: 
  - `af_inet.c` - IPv4 interface configuration
  - `af_inet6.c` - IPv6 interface configuration
  - `af_link.c` - Link layer configuration
  - `af_nd6.c` - IPv6 Neighbor Discovery
- **Interface Types**:
  - `ifbridge.c` - Bridge interface management
  - `ifclone.c` - Interface cloning (tun, tap, epair)
  - `ifgif.c` - GIF tunnel interface
  - `ifgre.c` - GRE tunnel interface
  - `iflagg.c` - Link aggregation interface
  - `ifvlan.c` - VLAN interface management
  - `ifvxlan.c` - VXLAN interface management
  - `ifmedia.c` - Media selection and configuration
- **Special Features**:
  - `carp.c` - CARP (Common Address Redundancy Protocol)
  - `ifpfsync.c` - PF state synchronization
  - `sfp.c` - SFP module management

#### netstat Source Code (`/usr/src/usr.bin/netstat/`)
- **Main Files**: `main.c`, `netstat.h` - Core netstat functionality
- **Network Display**:
  - `if.c` - Interface statistics display
  - `inet.c` - IPv4 socket information
  - `inet6.c` - IPv6 socket information
  - `route.c` - Routing table display
  - `route_netlink.c` - Netlink routing information
- **Protocol Support**:
  - `sctp.c` - SCTP protocol statistics
  - `ipsec.c` - IPsec statistics
  - `pfkey.c` - PF key management
- **System Information**:
  - `mbuf.c` - Memory buffer statistics
  - `netisr.c` - Network interrupt service routines
  - `netgraph.c` - Netgraph information
  - `unix.c` - Unix domain socket information
- **Utilities**:
  - `common.c` - Common utility functions
  - `bpf.c` - BPF (Berkeley Packet Filter) information
  - `nlist_symbols` - Symbol table utilities

### Key Implementation Patterns
These source files demonstrate:
1. **Interface Management**: How to enumerate, configure, and monitor network interfaces
2. **Address Family Handling**: Proper abstraction for different network protocols
3. **System Call Usage**: Examples of using `ioctl`, `sysctl`, and other system interfaces
4. **Error Handling**: Robust error handling patterns for network operations
5. **Data Structures**: Usage of FreeBSD networking data structures and APIs

## Technical Approach

### Build System
- Use CMake with pkg-config for dependency resolution
- Implement custom CMake modules for Yacc/Lex/YANG processing
- Ensure cross-platform compatibility (though targeting FreeBSD)

### Architecture Patterns
- **Abstract Factory**: For interface type creation
- **Strategy**: For different interface implementations
- **Observer**: For configuration change notifications
- **Singleton**: For logger and configuration management

### Error Handling
- Implement comprehensive error codes
- Provide meaningful error messages to users
- Ensure graceful degradation when operations fail

## Dependencies & Requirements

### System Libraries
- libnetconf2 (via pkg-config)
- libyang (via pkg-config)
- FreeBSD system headers
- curses (for terminal interface)

### Build Tools
- CMake 3.16+
- Flex/Lex
- Bison/Yacc
- C++17 compatible compiler

## Risk Assessment

### High Risk
- **FreeBSD API integration**: Complex system calls and interface management
- **NETCONF protocol implementation**: Ensuring full compliance with standards

### Medium Risk
- **YANG schema management**: Complex schema loading and validation
- **Configuration persistence**: Reliable storage and recovery

### Low Risk
- **Build system setup**: Standard CMake configuration
- **CLI interface**: Well-established patterns for terminal applications

## Success Criteria

1. **Functional CLI**: All specified commands work correctly
2. **NETCONF compliance**: Server responds properly to NETCONF requests
3. **Configuration persistence**: Changes survive system restarts
4. **Performance**: Commands execute in reasonable time (< 100ms for simple operations)
5. **Stability**: No crashes or memory leaks during normal operation

## Usage Notes

1. **Interface Creation**: Use `if_clone.h` for creating new interfaces
2. **Configuration**: Use `ioctl` operations for interface configuration
3. **Statistics**: Use `if_mib.h` for interface statistics
4. **Routing**: Use `route_ctl.h` for routing table manipulation
5. **System Control**: Use `sysctl.h` for VRF and system configuration

## Dependencies

These headers depend on:
- Standard C library headers
- System call definitions
- Kernel data structures
- Network protocol definitions

## Compilation

When compiling NETD, ensure these headers are available and properly linked. The CMake build system will handle include path configuration automatically.
