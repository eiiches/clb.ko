# redirector.ko

Redirect connect(2)ions transparently to the applications, with zero overhead after establishment.

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
