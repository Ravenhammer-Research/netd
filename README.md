# NetD - Network Configuration Daemon

NetD is a modern network configuration daemon for FreeBSD that provides NETCONF-based network management with a CLI interface. It offers a clean separation between configuration management and the underlying operating system, making it suitable for both traditional network equipment and modern software-defined networking environments.

## What is NETCONF?

**NETCONF (Network Configuration Protocol)** is an IETF standard (RFC 6241) that provides a standardized way to install, manipulate, and delete the configuration of network devices. It was developed by the Internet Engineering Task Force (IETF) to address the limitations of traditional network management protocols like SNMP.

### Why NETCONF?

NETCONF offers several advantages over traditional network management approaches:

- **Standardized Protocol**: IETF-standardized protocol ensures interoperability across different vendors and devices
- **YANG Data Modeling**: Uses YANG (Yet Another Next Generation) for data modeling, providing strong typing and validation
- **Transaction-based**: Supports atomic configuration changes with rollback capabilities
- **Separation of Concerns**: Clear separation between configuration data, operational data, and state data
- **Modern Architecture**: Designed for modern network automation and software-defined networking
- **Industry Adoption**: Widely adopted by major network equipment vendors (Cisco, Juniper, Nokia, Huawei, etc.)

### IETF's Role

The IETF has been instrumental in developing and maintaining NETCONF:

- **RFC 6241**: Defines the core NETCONF protocol
- **RFC 6020**: Defines the YANG data modeling language
- **RFC 6242**: Defines NETCONF over SSH
- **RFC 8071**: Defines NETCONF over TLS
- **Ongoing Work**: Continuous development of new YANG models and protocol extensions

NETCONF represents the IETF's vision for modern network management, providing a robust foundation for network automation, configuration management, and operational monitoring.

## Features

- **NETCONF Server**: Full NETCONF 1.1 implementation with YANG data modeling
- **Interactive CLI**: Curses-based command-line interface with command completion
- **FreeBSD Integration**: Native FreeBSD interface and routing management
- **Modular Architecture**: Clean separation between shared types, OS-specific implementations, and server logic
- **YANG Support**: Comprehensive YANG schema support with libyang integration
- **Type Safety**: Strongly-typed C++ implementation with comprehensive error handling

## Architecture

NetD follows a layered architecture:

```mermaid
graph TB
    subgraph "Client Layer"
        CLI[Interactive CLI<br/>netc]
        Parser[Command Parser<br/>Flex/Bison]
    end
    
    subgraph "Server Layer"
        Server[NETCONF Server<br/>netd]
        Handlers[RPC Handlers]
    end
    
    subgraph "Shared Layer"
        Types[Common Types]
        YANG[YANG Support]
        Marshalling[Data Marshalling]
    end
    
    subgraph "OS Layer"
        FreeBSD[FreeBSD Integration]
        Interfaces[Interface Management]
        Routing[Routing Management]
    end
    
    CLI --> Parser
    CLI -.->|NETCONF over Unix Socket| Server
    Server --> Handlers
    Handlers --> Types
    Types --> YANG
    Types --> Marshalling
    Marshalling --> FreeBSD
    FreeBSD --> Interfaces
    FreeBSD --> Routing
```

### Components

- **`netd`**: The main server daemon that provides NETCONF services
- **`netc`**: The client CLI application for interactive configuration
- **Shared Layer**: Common types, YANG support, and marshalling logic
- **FreeBSD Layer**: OS-specific interface and routing management
- **Parser**: Flex/Bison-based command parser for the CLI

## Supported Interface Types

NetD supports a comprehensive range of network interface types:

```mermaid
graph TB
    subgraph "Physical Interfaces"
        Ethernet[Ethernet<br/>em*, igb*, ix*, bge*, re*, fxp*]
        Wireless[Wireless<br/>wlan*, ath*]
        PPP[PPP<br/>ppp*]
    end
    
    subgraph "Virtual Interfaces"
        Bridge[Bridge<br/>bridge*, br*]
        VLAN[VLAN<br/>vlan*]
        TUN[TUN<br/>tun*]
        VXLAN[VXLAN<br/>Tunnel interfaces]
        WireGuard[WireGuard<br/>VPN interfaces]
        TAP[TAP<br/>TAP interfaces]
    end
    
    subgraph "Interface Hierarchy"
        Ether[Ether<br/>Base Type]
        Master[Master<br/>Management Type]
        Tunnel[Tunnel<br/>Tunneling Type]
    end
    
    Ethernet --> Ether
    Wireless --> Ether
    PPP --> Ether
    VLAN --> Ether
    
    Bridge --> Master
    
    TUN --> Tunnel
    VXLAN --> Tunnel
    WireGuard --> Tunnel
    TAP --> Tunnel
```

### Interface Type Details

- **Physical Interfaces**: Direct hardware interfaces
- **Virtual Interfaces**: Software-created interfaces
- **Interface Hierarchy**: Three base types for different interface categories

## Installation

### Prerequisites

- FreeBSD 13.0 or later
- CMake 3.16 or later
- libyang and libnetconf2 development packages
- Flex and Bison for parser generation

### Building

```bash
# Clone the repository
git clone <repository-url>
cd netd

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make

# Install (optional)
sudo make install
```

### Dependencies

The following packages are required:

```bash
# On FreeBSD
pkg install cmake libyang libnetconf2 flex bison

# Development packages
pkg install libyang-dev libnetconf2-dev
```

## Usage

### Starting the Server

```bash
# Start the NetD server
sudo ./netd

# The server will listen on /var/run/netd.sock by default
```

### Using the Client

```bash
# Start the interactive CLI
./netc

# The client will connect to the server automatically
```

### Available Commands

#### Interface Management
```bash
# Show all interfaces
show interfaces

# Show specific interface
show interfaces em0

# Show interface with unit
show interfaces em0 unit 0

# Configure interface
set interfaces em0 unit 0 family inet address 192.168.1.1/24
set interfaces em0 description "Management interface"
```

#### Routing Configuration
```bash
# Show routing instances
show routing-instances

# Show routes
show route

# Configure static routes
set routing-options static route 0.0.0.0/0 next-hop 192.168.1.1
```

#### System Information
```bash
# Show system information
show version
show system uptime
show chassis

# Show ARP table
show arp
show arp no-resolve

# Show protocols
show protocols

# Show IPv6 neighbors
show ipv6 neighbors
```

#### Configuration Management
```bash
# Commit configuration changes
commit

# Edit configuration
edit interfaces em0 unit 0

# Delete configuration
delete interfaces em0 unit 0 family inet address 192.168.1.1/24
```

## Configuration

### Server Configuration

The server can be configured through command-line options:

```bash
# Custom socket path
./netd --socket /tmp/custom-netd.sock

# Custom YANG directory
./netd --yang-dir /usr/local/share/yang

# Enable debug logging
./netd --debug
```

### Client Configuration

The client automatically connects to the server. Configuration options:

```bash
# Connect to custom server socket
./netc --socket /tmp/custom-netd.sock

# Enable debug mode
./netc --debug
```

## YANG Models

NetD includes comprehensive YANG models for network configuration:

- **Standard IETF Models**: RFC-compliant models for interfaces, routing, and NETCONF
- **Custom Models**: NetD-specific models for advanced features
- **Vendor Models**: Support for vendor-specific extensions

YANG models are located in the `yang/` directory:

- `yang/standard/`: Standard IETF and IEEE models
- `yang/vendor/`: Vendor-specific models
- `shared/yang/`: NetD-specific models

## Development

### Project Structure

```mermaid
classDiagram
    class NetconfClient {
        +connect(socketPath)
        +disconnect()
        +sendRequest(request)
        +getConfig(source)
        +get(filter)
        +editConfig(target, config)
    }
    
    class Terminal {
        +initialize()
        +readLine()
        +writeLine(text)
        +redrawPrompt()
        +runInteractive()
    }
    
    class CommandProcessor {
        +processCommand(command)
        -handleShowCommand()
        -handleSetCommand()
        -handleDeleteCommand()
    }
    
    class NetconfServer {
        +start()
        +stop()
        +handleRpc()
    }
    
    class RpcHandler {
        +handleGetConfig()
        +handleGet()
        +handleEditConfig()
        +handleCommit()
    }
    
    class Yang {
        +getInstance()
        +getContext()
        +loadSchema()
        +yangToXml()
        +xmlToYang()
    }
    
    class Logger {
        +getInstance()
        +info(message)
        +error(message)
        +debug(message)
        +setCallback()
    }
    
    class EthernetInterface {
        +getAllEthernetInterfaces()
        +configure()
        +getStatus()
    }
    
    class BridgeInterface {
        +getAllBridgeInterfaces()
        +addMember()
        +removeMember()
    }
    
    class VlanInterface {
        +getAllVlanInterfaces()
        +setVlanId()
        +setParent()
    }
    
    class VRF {
        +create()
        +delete()
        +addInterface()
        +removeInterface()
    }
    
    class Route {
        +addStaticRoute()
        +deleteRoute()
        +getRoutes()
    }
    
    NetconfClient --> Terminal
    NetconfClient --> Yang
    CommandProcessor --> Terminal
    CommandProcessor --> NetconfClient
    
    NetconfServer --> RpcHandler
    NetconfServer --> Yang
    RpcHandler --> Logger
    
    Yang --> Logger
    EthernetInterface --> Logger
    BridgeInterface --> Logger
    VlanInterface --> Logger
    
    VRF --> EthernetInterface
    VRF --> BridgeInterface
    VRF --> VlanInterface
    Route --> VRF
```

### Adding New Interface Types

1. **Define the model class** in `shared/include/interface/` - These classes provide platform-independent functionality including YANG serialization/deserialization (`toYang`/`fromYang`) and serve as data models for creating native interfaces
2. **Implement the FreeBSD-specific version** in `freebsd/src/interface/` - These classes handle native functionality including configuration acquisition and application via system calls
3. Add discovery functions to the FreeBSD layer
4. Update the server handlers to support the new type
5. Add YANG models if needed

### Adding New Commands

1. Add tokens to `client/parser/parser.l`
2. Add grammar rules to `client/parser/parser.y`
3. Implement command handlers in `client/src/main.cpp`
4. Add server-side support if needed

### Building from Source

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# Build specific components
make netd          # Server only
make netc          # Client only
make netd_shared   # Shared library only
```

## API Reference

### NETCONF Operations

NetD supports standard NETCONF operations:

```mermaid
sequenceDiagram
    participant Client as netc (Client)
    participant Server as netd (Server)
    participant Store as Configuration Store
    participant FreeBSD as FreeBSD Layer
    
    Note over Client,FreeBSD: Configuration Retrieval
    Client->>Server: <get-config>
    Server->>Store: Query configuration
    Store-->>Server: Return config data
    Server-->>Client: <rpc-reply>
    
    Note over Client,FreeBSD: Operational Data
    Client->>Server: <get>
    Server->>FreeBSD: Query operational state
    FreeBSD-->>Server: Return state data
    Server-->>Client: <rpc-reply>
    
    Note over Client,FreeBSD: Configuration Changes
    Client->>Server: <edit-config>
    Server->>Store: Store pending changes
    Store-->>Server: Success/Error
    Server-->>Client: <rpc-reply>
    
    Note over Client,FreeBSD: Commit Changes
    Client->>Server: <commit>
    Server->>Store: Apply configuration
    Store->>FreeBSD: Apply native configuration
    FreeBSD-->>Store: Apply result
    Store->>Store: Write to running config
    Store-->>Server: Commit result
    Server-->>Client: <rpc-reply>
```

**Supported Operations:**
All standard NETCONF operations are supported:
- **`<get-config>`**: Retrieve configuration data from the store
- **`<get>`**: Retrieve operational data from the system
- **`<edit-config>`**: Modify configuration (stored in candidate)
- **`<commit>`**: Commit candidate configuration to running
- **`<close-session>`**: Close NETCONF session
- **`<kill-session>`**: Terminate another session
- **`<lock>`**: Lock configuration datastore
- **`<unlock>`**: Unlock configuration datastore
- **`<validate>`**: Validate configuration
- **`<copy-config>`**: Copy configuration between datastores
- **`<delete-config>`**: Delete configuration datastore
- **`<discard-changes>`**: Discard uncommitted changes

### YANG Data Models

All configuration and operational data is modeled using YANG:

```mermaid
graph TB
    subgraph "Standard IETF Models"
        IETFInterfaces[ietf-interfaces<br/>Interface configuration]
        IETFRouting[ietf-routing<br/>Routing configuration]
        IETFNetconf[ietf-netconf<br/>NETCONF protocol]
        IETFInet[ietf-inet-types<br/>IP address types]
        IETFYang[ietf-yang-types<br/>YANG types]
    end
    
    subgraph "NetD Custom Models"
        NetdInterface[netd-interface-80211<br/>WiFi interfaces]
        NetdEpair[netd-interface-epair<br/>Epair interfaces]
        NetdPpp[netd-interface-ppp<br/>PPP interfaces]
        NetdTap[netd-interface-tap<br/>TAP interfaces]
        NetdTun[netd-interface-tun<br/>TUN interfaces]
        NetdVxlan[netd-interface-vxlan<br/>VXLAN interfaces]
        NetdWireguard[netd-interface-wireguard<br/>WireGuard interfaces]
    end
    
    subgraph "Vendor Models"
        Cisco[Cisco Models<br/>IOS XE/XR]
        Juniper[Juniper Models<br/>JunOS]
        Nokia[Nokia Models<br/>SR OS]
        Huawei[Huawei Models<br/>VRP]
    end
    
    IETFInterfaces --> NetdInterface
    IETFInterfaces --> NetdEpair
    IETFInterfaces --> NetdPpp
    IETFInterfaces --> NetdTap
    IETFInterfaces --> NetdTun
    IETFInterfaces --> NetdVxlan
    IETFInterfaces --> NetdWireguard
    
    IETFRouting --> IETFInterfaces
    IETFNetconf --> IETFYang
    IETFInet --> IETFYang
```

**Model Categories:**
- **Standard IETF Models**: RFC-compliant models for interfaces, routing, and NETCONF
- **NetD Custom Models**: NetD-specific interface extensions
- **Vendor Models**: Support for vendor-specific extensions

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request


## License

NetD is licensed under the BSD 2-Clause License. See the [LICENSE](LICENSE) file for details.

## Support

- **Documentation**: See the `doc/` directory for detailed documentation
- **Issues**: Report bugs and feature requests via GitHub issues
- **Discussions**: Use GitHub discussions for questions and general discussion

## Roadmap

- [ ] Enhanced VXLAN and WireGuard support
- [ ] SCTP protocol support
- [ ] RESTCONF API support
- [ ] Configuration validation and rollback
- [ ] SNMP integration
- [ ] Web-based management interface
- [ ] Docker containerization
- [ ] Integration with network orchestration platforms

## Acknowledgments

- **libyang**: YANG data modeling library
- **libnetconf2**: NETCONF protocol implementation
- **FreeBSD**: The underlying operating system
- **IETF**: For NETCONF and YANG standards

---

**NetD** - Modern network configuration for FreeBSD
