export ipaddr=`ip addr show dev eth0 scope global primary | perl -ane 'if ($F[0] eq "inet") {$F[1] =~ "(.*)/"; print $1."\n"}'`
iptables -t nat -A PREROUTING -j DNAT -p tcp --dport 3389 --to-destination $ipaddr
iptables -t mangle -A PREROUTING -p tcp --dport 3389 -j TPROXY --on-port 3389 --tproxy-mark 0x1/0x1
ip rule add fwmark 1 lookup 100
ip route add local 0.0.0.0/0 dev lo table 100
#iptables -t mangle -A DIVERT -j MARK --set-mark 1
#iptables -t mangle -A DIVERT -j ACCEPT
#iptables -t mangle -A PREROUTING -p tcp --dport 3389 -j TPROXY --tproxy-mark 0x1/0x1 --on-port 3389
