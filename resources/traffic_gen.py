from scapy.all import *
import sys

# test
for i in range(0, 100):
    send(IP(src="10.0.99.100",dst="10.1.99.100")/ICMP()/"Hello World .flv .flv ")

# generate torrent traffic on ports


# generate HTTP traffic on ports


# generate video traffic on ports
