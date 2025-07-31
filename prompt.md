## Project Overview

Create a comprehensive network configuration and management tool for FreeBSD. The tool should manage network interfaces, routing tables, 
and support FreeBSD-specific features like multiple FIBs (Forwarding Information Bases) and tunnel FIBs.

## Core Architecture

### Client-Server Model
- **Server (`netd`)**: Unix domain socket server that handles network configuration operations

- **Client (`net`)**: Interactive CLI client with readline support and batch mode capabilities

- **Communication**: Unix domain socket (`/var/run/netd.sock`); the client / server will use 
the netconf protocol exclusively along with yang models provided in yang/. The standard yang
models in std/yang will be used before anything else. This will also support a server / client
that uses SCTP protocol, but it should not be implemented more than "not implemented" at this
time.

- VRF will be substituted to represent FreeBSD's FIB assignment for interfaces and route tables. 
VRF is already defined in standard Yang.

- The client will support interactive/prompt based configuration as well as one-shot. The client
will be implemented using Yacc on FreeBSD (a parser generator.) The client in prompt/interactive
mode will support tab completion of syntax/names. The client will use nothing except for netconf/yang
to retrieve potential names for completion from the server.

- At no point in either the server nor the client (or any aspect of this project) should system,
popen or exec ever be used to call to a userland binary. This is not acceptable, and any attempt
to implement this project with these calls will be subsequently deleted and the whole project
will have to start over.

- libnetconf2 and libyang are currently installed to /usr/local 

- functionality should be logically spread across multiple source files; I thought this would be
a given but on numerous occasions claude has attempted to put too much into source files making 
them too complex for it to work on.

- I have provided the source code for both ifconfig and route commands on FreeBSD to be used as
examples for for implementing the necesarry functionality in netd.

- Do not leave TODOs or unimplemented code; implement or don't, again no stub files, no todos just 
real code.

- bsdxml.h will be used wherever xml is needed 
do not attempt to use libxml or any other weird shit, the only linkable libraries there are 
acceptable are:

- libc
- libnetconf2
- libyang
- readline

- each function will be commented

- For the server if debug option is used log to stderr, multiple levels of debug logging please, otherwise all 
other messages log to syslog

- for the client make the best use of both stdout and stderr

- no (void) unused parameter casts that shit pisses me off.

[client] <-> (netconf/yang) <-> [server] <-> (ifconfig / route ioctl, sysctl and possibly sysctl)

# example commands
these are just examples, none of the interfaces names or vrf names (except for default vrf) should
ever be hardcoded as claude has attempted to do at times:

set vrf my-vrf table 16           (creates a VRF named my-vrf that is associated with FIB 16, the server
                                  (will need to keep track of state in it's configuration file.)
show vrf                          (shows all VRFs)
delete vrf default                (invalid, can't delete default it should be system managed)
set vrf default fib 16            (invalid, default is always 0)

show interface                   (show all interfaces)
show interface epair             (show all epair interfaces)
show interface gif               (show all gif interfaces)
show interface gre               (show all gre interfaces)
show interface lagg              (show all lagg interfaces)
show interface lo                (show all loopback interfaces)
show interface ovpn              (show all ovpn interfaces)
show interface tun               (show all tun interfaces)
show interface tap               (show all tap interfaces)
show interface vlan              (show all vlan interfaces)
show interface vxlan             (show all vxlan interfaces)
show interface vxlan vxlan0      (show vxlan0 interface)
show interface group my-if-group (shows all interfaces in the my-if-group)
show interface ethernet          (will show phyisical ethernet interfaces)     
show interface wireless80211     (will show wireless 802.11 interfaces)

set interface epair epair0a vrf my-vrf                   (the server will set the interface fib parameter to 16)
set interface epair epair0a address inet 10.242.242.1/24                                                         
set interface epair epair0a address inet6 fc00::a/64 
set interface epair epair0a mtu 9000
set interface epair epair0a group my-if-group (will add the epair0a interface to the my-if-group group)

when any set interface <type> <name> is specified, if the interface doesn't exist it will be created, except
for phyisical interfaces like ethernet and wifi because they can't be materialized out of thin air in the physical 
world.

delete interface epair epair0a address inet (will delete all ipv4 addresses from the interface)
delete interface epair epair0a              (will delete the epair0a interface if it exists)
delete interface epair                      (this should be considered invalid)

show vrf default static inet                (show inet4 routes for fib 0)
show vrf default static inet6               (show inet4 routes for fib 0)
show vrf my-vrf static inet                 (show inet4 routes for fib 16)

set vrf default inet static 192.168.0.0/16 10.0.0.1   (net route via gw fib 0)
set vrf default inet static 10.0.0.0/8 reject         (reject dest fib 0)
set vrf default inet static 10.0.0.0/8 blackhole      (blackhole dest fib 0)
set vrf default inet static 192.168.0.1 10.0.0.1      (host route via gw fib 0)
set vrf my-vrf inet static  169.254.0.0/16 reject     (reject dest fib 16)
delete vrf my-vrf                                     (deletes all routes in fib 16, and all interfaces assigned to fib 16 
                                                      (then deletes the vrf.)
delete vrf my-vrf inet                                (deletes all ipv4 routes in fib 16)

commit (commands are transactional, no work is actually done until this command is run)
save (writes the running configuration to configuration file, configuration file should be in the same command format
as commands are given.)

- tab completion of names may depend on interface names that don't exist yet (before commit)

# src tree layout 
- example/ (examples of how to implement freebsd specific functionality using system apis / ioctls)
- yang/ (yang models)
- yang/std/standard/ietf/RFC (standard yang models)
- yang/netd.yang (augments, but standard should be used as much as possible)
- src/client (.c/.h files)
- src/server (.c/.h files)
- ./LICENSE (use in all source files)

the client and server should not share any code in common for feat that the client will not rely on the server for information
that it needs. Consider that the client and server support SCTP, the client may not be on the same machine as the server. With
consideration to that, code can be shared but the client and server must always use netconf / yang to communicate and the 
client should never rely on the host it's running on for information, and the server should never be preformatting data
for the client for displaying purposes..

- you cant build locally, this program is for freebsd and claude only runs on my linux box so no sense in ever trying to run make or any other commands.