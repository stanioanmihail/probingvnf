from scapy.all import *
import sys

# test
for i in range(0, 100):
   send(IP(src="192.168.1.138",dst="10.1.99.100")/ICMP()/"Hello World .flv .flv ")

# generate torrent traffic on ports
'''
for i in range(0, 50):
    ip=IP(src="192.168.1.138")
    ip.dst="192.168.0.2"
    ip/TCP()
    tcp=TCP(sport=9999, dport=9091)
    (tcp/ip).show()
    send(ip/tcp)
'''
for i in range(0, 50):
    ip=IP(src="192.168.1.138")
    ip.dst="192.168.0.2"
    ip/TCP()
    tcp=TCP(sport=9999, dport=6882)
    (tcp/ip).show()
    send(ip/tcp)

# generate HTTP traffic on ports


# generate video traffic on ports
