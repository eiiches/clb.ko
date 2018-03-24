# sock_vs.ko

Balance connect(2)ions across servers transparently to the application, with zero overhead after establishment.

### Motivation

Client Side Loadbalancing with iptables or IPVS has some overhead.


#### Baseline Performance

Client: 192.168.111.11
```
$ taskset -c 1 redis-benchmark -h 192.168.111.1 -p 6379 -c 100 -P 1 -t ping -n 10000000
^CNG_INLINE: 115972.19
```

Server: 192.168.111.1
```bash
$ taskset -c 0 redis-server --port 6379 --save '' --bind 0.0.0.0
```

#### IPVS Performance

Client: 192.168.111.11 with -g (Direct Routing):
```
$ sudo ipvsadm -A -t 10.0.0.1:6379 -s rr
$ sudo ipvsadm -a -t 10.0.0.1:6379 -r 192.168.111.1:6379 -g
$ taskset -c 1 redis-benchmark -h 10.0.0.1 -p 6379 -c 100 -P 1 -t ping -n 10000000
^CNG_INLINE: 111467.13
```

Client: 192.168.111.11 with -m (NAT):
```
$ sudo ipvsadm -A -t 10.0.0.1:6379 -s rr
$ sudo ipvsadm -a -t 10.0.0.1:6379 -r 192.168.111.1:6379 -m
$ taskset -c 1 redis-benchmark -h 10.0.0.1 -p 6379 -c 100 -P 1 -t ping -n 10000000
^CNG_INLINE: 103044.31
```

#### iptables Performance

Client: 192.168.111.11 (without SNAT)
```
$ sudo iptables -t nat -A OUTPUT -m statistic --mode random --probability 1.0 -m tcp -p tcp -d 10.0.0.1 --dport 6379 -j DNAT --to-destination 192.168.111.1:6379
$ taskset -c 1 redis-benchmark -h 10.0.0.1 -p 6379 -c 100 -P 1 -t ping -n 10000000
^CNG_INLINE: 95377.39
```


### Features

#### Pros

 * Performance. There's no overhead after the connection is established.
 * Can connect to different port on the real servers. This is what ipvs cannot when using direct routing.

#### Cons

 * There is some overhead on connect(2) and close(2). This shouldn't be noticeable if you don't establish > 100 thousands of connections per second.
 * Cannot mix IPv4 and IPv6 sevices. IPv4 virtual server can only be served by IPv4 real servers. Same for IPv6.
 * This is a client-side load balancing solution and works only for locally-originated connect(2)ions. Use iptables or IPVS if you want to accept remote connections.
 * This is a layer-4 load balancer. Cannot balance load per-request basis. i.e.) all HTTP requests on the same connection goes to the same backend server.

### Requirements

 * Arch: x86\_64

### Usage

TBD

### Caveats

 * While `connect(2)` are load-balanced transparently to the application, `getpeername(2)` on the socket will return the real server address, not the virtual one, breaking some transparency.


### License

The GNU General Public License v2.0
