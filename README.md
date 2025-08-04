# Examples
## route table (default fib)
```
net> show route protocol static

Route Table:
Destination               Gateway                 Interface
0.0.0.0                   192.168.32.129          em0      
127.0.0.1                 link#2                  lo0      
192.168.0.0               link#17                 lo0      
192.168.0.0               link#17                 tap5     
192.168.32.128            link#1                  em0      
192.168.32.254            link#1                  lo0      
::                        ::1                     lo0      
::1                       link#2                  lo0      
::ffff:0.0.0.0            ::1                     lo0      
fcff::                    fcff:16::192:168:32:129 em0      
fcff:16::                 link#1                  em0      
fcff:16::192:168:32:254   link#1                  lo0      
fe80::                    ::1                     lo0      
fe80::                    link#1                  em0      
fe80::6600:6aff:fe85:a26e link#1                  lo0      
fe80::                    link#2                  lo0      
fe80::1                   link#2                  lo0      
fe80::                    link#11                 vlan28   
fe80::6600:6aff:fe85:a26e link#11                 lo0      
fe80::                    link#12                 lo28     
fe80::1                   link#12                 lo0      
fe80::                    link#14                 em0.28   
fe80::6600:6aff:fe85:a26e link#14                 lo0      
fe80::                    link#16                 vlan28.1 
fe80::6600:6aff:fe85:a26e link#16                 lo0      
fe80::                    link#17                 tap5     
fe80::5a9c:fcff:fe10:ffe4 link#17                 lo0      
ff02::                    ::1                     lo0      
Total routes: 28
```

## Summary of all interfaces
```
net> show interface 

Interface Table:
Name     Status VRF     MTU   Flags IPv4            IPv6                      Groups     
em0      UP     default 1500  UBRPM 192.168.32.254  fe80::6600:6aff:fe85:a26e            
                                                    fcff:16::192:168:32:254              
lo0      UP     default 16384 URML  127.0.0.1       ::1                       lo         
                                                    fe80::1                              
wlan0    DOWN   default 1500  BM                                              wlan       
bridge0  UP     18      1500  UBRM  192.168.64.254  fcff:18::192:168:64:254   bridge     
                                    169.254.169.254                                      
bridge1  UP     30      1500  UBRM  192.168.72.254                            bridge     
                                    169.254.169.254                                      
tap0     UP     18      1500  UBRPM                 fe80::5a9c:fcff:fe10:ffb6 tap        
tap1     UP     30      1500  UBRPM                 fe80::5a9c:fcff:fe10:740a tap        
tap2     UP     30      1500  UBRPM                 fe80::5a9c:fcff:fe10:cb7a tap        
                                                                              vm-port    
tap3     UP     30      1500  UBRPM                 fe80::5a9c:fcff:fe10:ffb7 tap        
tap4     UP     18      1500  UBRPM                 fe80::5a9c:fcff:fe10:ffc5 tap        
vlan28   UP     default 1500  UBM                   fe80::6600:6aff:fe85:a26e vlan       
em0.18   UP     18      1500  UBRPM                 fe80::6600:6aff:fe85:a26e vlan       
em0.28   UP     28      1500  UBRM  203.0.113.111   fe80::6600:6aff:fe85:a26e vlan       
em0.30   UP     30      1500  UBRPM                 fe80::6600:6aff:fe85:a26e vlan       
vlan28.1 UP     28      1496  UBRM                  fe80::6600:6aff:fe85:a26e vlan       
tap5     UP     default 1500  UBRM  192.168.0.0     fe80::5a9c:fcff:fe10:ffe4 tap        
Total interfaces displayed above.
Flags: U=UP, B=BROADCAST, R=RUNNING, P=PROMISC, M=MULTICAST, L=LOOPBACK
```

## Summary of bridge interface group
```
net> show interface group bridge

Interface Table (Group: bridge):
Name    Status VRF Members               MTU  Flags IPv4            IPv6                    Groups
bridge0 UP     18  em0.18                1500 UBRM  192.168.64.254  fcff:18::192:168:64:254 bridge
                   tap4                             169.254.169.254                               
                   tap0                                                                           
bridge1 UP     30  em0.30                1500 UBRM  192.168.72.254                          bridge
                   tap3                             169.254.169.254                               
                   tap2                                                                           
                   tap1                                                                           
Total interfaces in group 'bridge': 2
Total interfaces displayed above.
Flags: U=UP, B=BROADCAST, R=RUNNING, P=PROMISC, M=MULTICAST, L=LOOPBACK

net> 
```

## Summary of WLAN group
```
net> show interface group wlan

Wireless Interface Table (Group: wlan):
Name  Status SSID Channel Frequency Mode Security Signal Rate Groups
wlan0 DOWN                                                    wlan  
Total interfaces in group 'wlan': 1
Total wireless interfaces displayed above.
Flags: U=UP, B=BROADCAST, R=RUNNING, P=PROMISC, M=MULTICAST, L=LOOPBACK

net> 
```

## Summary of VRF

```
net> show vrf

VRF Table:
Name    FIB Description                 
default 0   Default VRF (system managed)
-       18  FIB 18 (auto-detected)      
-       28  FIB 28 (auto-detected)      
-       30  FIB 30 (auto-detected)      
Total VRFs: 4
net> 
```

# TODO 

- `show vrf name <name> protocol static`
- `show vrf id <id> protocol static` (this is just to show that either wants to be unique and should qualify for selection)
- `show interface name <interface_name>`
- `set interface`
- `set route` 
- `set vrf`
- `set vrf id <id> name <name>`
- `delete`
- `commit`
- `save`