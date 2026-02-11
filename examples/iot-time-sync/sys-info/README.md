# Clock skew compensation (CSC)

A minimal Contiki-NG example, simple printing out "Hello, world".
This example runs a full IPv6 stack with 6LoWPAN and RPL.
It is possible, for example to ping such a node:

```
make TARGET=native && sudo ./csc.native
```

Look for the node's global IPv6, e.g.:
```
[INFO: Native    ] Added global IPv6 address fd00::302:304:506:708
```

And ping it (over the tun interface):
```
$ ping6 fd00::302:304:506:708
PING fd00::302:304:506:708(fd00::302:304:506:708) 56 data bytes
64 bytes from fd00::302:304:506:708: icmp_seq=1 ttl=64 time=0.289 ms
```

## How to run a project on a TelosB mote
1. First check the port (e.g., "/dev/ttyUSB0").
```
make motelist-all
```
2. Build and run the project on the mote (e.g., "csc" project).
```
make TARGET=sky PORT=/dev/ttyUSB0 csc.upload
make TARGET=sky PORT=/dev/ttyUSB0 login
```
