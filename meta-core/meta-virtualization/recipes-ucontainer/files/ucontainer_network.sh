#!/bin/sh
echo "1" > /proc/sys/net/ipv4/ip_forward 
ip link add bridge0 type bridge
ifconfig bridge0 10.0.0.1 up
iptables -P FORWARD ACCEPT
iptables -t nat -A POSTROUTING -s 10.0.0.0/24 -o wlan0 -j MASQUERADE