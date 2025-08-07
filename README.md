# netd - Network Management Tool

A NETCONF-based network configuration tool for FreeBSD systems.

## Building

```bash
make clean && make
```

## Usage

### View Commands

**Show all VRFs:**
```bash
net> show vrf
```

**Show routes for specific VRF:**
```bash
net> show vrf id <fib> protocol static
net> show vrf id <fib> protocol static inet
net> show vrf id <fib> protocol static inet6
net> show route protocol static  # same as "show vrf id 0 protocol static"
```

**Show interfaces:**
```bash
net> show interface
net> show interface type <type>
net> show interface group <group>
net> show interface <type> <name>
```

### Configuration Commands

**Add static route:**
```bash
net> set vrf id <fib> protocol static <inet|inet6> host <host> gateway <gateway> [iface <interface>]
net> set vrf id <fib> protocol static <inet|inet6> host <host> iface <interface>
net> set vrf id <fib> protocol static <inet|inet6> network <network> gateway <gateway> [iface <interface>]
net> set vrf id <fib> protocol static <inet|inet6> network <network> iface <interface>
```

**Delete static route:**
```bash
net> delete vrf id <fib> protocol static <inet|inet6> network <network> gateway <gateway>
```

**Create interface:**
```bash
net> set interface type <type> name <name> vrf id <id>
net> set interface type <type> name <name> address <inet|inet6> <address>
net> set interface type <type> name <name> mtu <size>
net> set interface type <type> name <name> group <group_name>
```

**Bridge member operations:**
```bash
net> set interface type bridge name <name> member <interface_name>
net> delete interface name <name> member
net> delete interface name <name> member <interface_name>
```

**LAG operations:**
```bash
net> set interface type lagg name <name> laggproto <protocol>
net> set interface type lagg name <name> laggport <interface_name>
net> delete interface name <name> laggport
net> delete interface name <name> laggport <interface_name>
```

**Ethernet pair operations:**
```bash
net> set interface type epair name <name> peer <a|b>
net> set interface type epair name <name> peer <a|b> vrf id <id>
net> set interface type epair name <name> peer <a|b> address <inet|inet6> <address>
```

**VLAN operations:**
```bash
net> set interface type vlan name <name> vlandev <interface>
net> set interface type vlan name <name> vlanproto <protocol>
net> set interface type vlan name <name> layer2 vlan id <id>
```

**VXLAN operations:**
```bash
net> set interface type vxlan name <name> vxlanid <id>
net> set interface type vxlan name <name> tunnel <inet|inet6> vxlanlocal <addr_local> vxlanremote <addr_remote>
net> set interface type vxlan name <name> vxlandev <interface>
```

**GIF tunnel operations:**
```bash
net> set interface type gif name <name> tunnel <inet|inet6> local <addr_local> remote <addr_remote>
net> set interface type gif name <name> tunnelvrf id <id>
```

**Delete interface:**
```bash
net> delete interface name <name>
net> delete interface name <name> address <inet|inet6>
```

**Commit pending changes:**
```bash
net> commit
```

**Create VRF:**
```bash
net> set vrf id <fib> table <table_number>
net> set vrf id <fib> name <name>
```

## Interface Types

Supported interface types:
- `ethernet` - Ethernet interfaces
- `wireless80211` - Wireless 802.11 interfaces  
- `epair` - Ethernet pair interfaces
- `gif` - Generic tunnel interfaces
- `gre` - GRE tunnel interfaces
- `lagg` - Link aggregation interfaces
- `lo` - Loopback interfaces
- `ovpn` - OpenVPN interfaces
- `tun` - TUN tunnel interfaces
- `tap` - TAP interfaces
- `vlan` - VLAN interfaces
- `vxlan` - VXLAN interfaces

## Examples

```bash
# Add static route to VRF 0
net> set vrf id 0 protocol static inet network 10.0.0.0/32 gateway 192.168.32.129

# Add host route to VRF 0
net> set vrf id 0 protocol static inet host 192.168.0.1 gateway 10.0.0.1

# Add host route with gateway and interface constraint
net> set vrf id 0 protocol static inet host 192.168.0.1 gateway 10.0.0.1 iface em0

# Add route with interface (on-link)
net> set vrf id 0 protocol static inet network 192.168.1.0/24 iface em0

# Delete static route from VRF 0  
net> delete vrf id 0 protocol static inet network 10.0.0.0/32 gateway 192.168.32.129

# Create VLAN interface
net> set interface type vlan name vlan100 vrf id 0
net> set interface type vlan name vlan100 address inet 192.168.100.1/24
net> set interface type vlan name vlan100 mtu 1500

# Create TAP interface
net> set interface type tap name tap0 vrf id 18
net> set interface type tap name tap0 address inet6 fe80::1/64

# Create bridge interface with members
net> set interface type bridge name bridge0 vrf id 18
net> set interface type bridge name bridge0 member em0
net> set interface type bridge name bridge0 member em1

# Create LAG interface with ports
net> set interface type lagg name lagg0 vrf id 0
net> set interface type lagg name lagg0 laggproto lacp
net> set interface type lagg name lagg0 laggport re0
net> set interface type lagg name lagg0 laggport re1

# Create epair interface
net> set interface type epair name epair0 peer a vrf id 25
net> set interface type epair name epair0 peer a address inet 192.168.200.1/24
net> set interface type epair name epair0 peer b vrf id 0
net> set interface type epair name epair0 peer b address inet 192.168.200.2/24

# Create VLAN interface
net> set interface type vlan name vlan53 vlandev lagg0
net> set interface type vlan name vlan53 vlanproto 802.1ad
net> set interface type vlan name vlan53 layer2 vlan id 53

# Create GIF tunnel interface
net> set interface type gif name gif0 tunnel inet local 192.168.1.1 remote 192.168.1.2
net> set interface type gif name gif0 vrf id 25
net> set interface type gif name gif0 tunnelvrf id 0

# Delete interface address
net> delete interface name vlan100 address inet

# Delete specific bridge member
net> delete interface name bridge0 member em0

# Delete all bridge members
net> delete interface name bridge0 member

# Commit changes
net> commit

# View routes in VRF 0
net> show vrf id 0 protocol static
```