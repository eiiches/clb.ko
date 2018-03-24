#!/bin/bash

sudo ipvsadm -C
sudo systemctl stop docker.service
for table in nat filter mangle raw; do
	sudo iptables -t $table -F
	sudo iptables -t $table -X
done

for table in nat filter mangle raw; do
	sudo modprobe -r iptable_$table
done

sudo rmmod sock_vs
sudo modprobe -r ip_vs_rr
sudo modprobe -r ip_vs
sudo modprobe -r ipt_MASQUERADE
# sudo modprobe -r ipt_nat_masquerade_ipv4
sudo modprobe -r nf_conntrack_netlink
sudo modprobe -r nf_conntrack_ipv4
sudo modprobe -r xt_nat xt_tcpudp xt_statistic
sudo modprobe -r nf_nat
sudo modprobe -r xt_conntrack
sudo modprobe -r nf_conntrack
sudo modprobe -r xt_addrtype

lsmod | grep nf
lsmod | grep xt
lsmod | grep ip
