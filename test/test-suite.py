#!/usr/bin/env python3
# Set env variable "ADDRESS" with BMC IP

import os
import socket
import time
from socket import AF_INET, SOCK_DGRAM

sock = socket.socket(AF_INET, SOCK_DGRAM)

bmcAddr = os.environ.get("ADDRESS", "127.0.0.1")
addr = (bmcAddr, 427)

print("Invalid Lang Tag Length, large payload")
payload = (
    b"\x02"  # Version
    + b"\x09"  # Function ID: SRVTYPERQST
    + b"\x00" * 2  # Ignored Length bytes?
    + b"\xff"  # Length
    + b"\x00" * 2  # Flags
    + b"\x00" * 3  # Ext
    + b"\x00" * 2  # XID
    + b"\xff" * 2  # Language Tag Length
    + b"A" * 65000  # Language Tag
)
ret = sock.sendto(payload, addr)

time.sleep(3)

print("Large Lang Tag Length, large payload")
payload = (
    b"\x02"  # Version
    + b"\x09"  # Function ID: SRVTYPERQST
    + b"\x00" * 2  # Ignored Length bytes?
    + b"\xff"  # Length
    + b"\x00" * 2  # Flags
    + b"\x00" * 3  # Ext
    + b"\x00" * 2  # XID
    + b"\xfd\xe8"  # Language Tag Length
    + b"A" * 65000  # Language Tag
)
ret = sock.sendto(payload, addr)

time.sleep(3)

print("Invalid Lang Tag Length, small payload")
payload = (
    b"\x02"  # Version
    + b"\x09"  # Function ID: SRVTYPERQST
    + b"\x00" * 2  # Ignored Length bytes?
    + b"\xff"  # Length
    + b"\x00" * 2  # Flags
    + b"\x00" * 3  # Ext
    + b"\x00" * 2  # XID
    + b"\xff" * 2  # Language Tag Length
    + b"A" * 200  # Language Tag
)
ret = sock.sendto(payload, addr)

time.sleep(3)

print("Large Lang Tag Length, small payload")
payload = (
    b"\x02"  # Version
    + b"\x09"  # Function ID: SRVTYPERQST
    + b"\x00" * 2  # Ignored Length bytes?
    + b"\xff"  # Length
    + b"\x00" * 2  # Flags
    + b"\x00" * 3  # Ext
    + b"\x00" * 2  # XID
    + b"\xfd\xe8"  # Language Tag Length
    + b"A" * 200  # Language Tag
)
ret = sock.sendto(payload, addr)

time.sleep(3)

print("Invalid Lang Tag Length (overflow)")
payload = (
    b"\x02"  # Version
    + b"\x09"  # Function ID: SRVTYPERQST
    + b"\x00" * 2  # Ignored Length bytes?
    + b"\xff"  # Length
    + b"\x00" * 2  # Flags
    + b"\x00" * 3  # Ext
    + b"\x00" * 2  # XID
    + b"\x00\x20"  # Language Tag Length
    + b"A" * 10  # Language Tag
)
ret = sock.sendto(payload, addr)

time.sleep(3)

print("slptool findsrvtypes")
payload = (
    b"\x02"  # Version
    + b"\x09"  # Function ID: SRVTYPERQST
    + b"\x00" * 2  # Ignored Length bytes?
    + b"\x1d"  # Length
    + b"\x00" * 2  # Flags
    + b"\x00" * 3  # Ext
    + b"\x74\xe2"  # XID
    + b"\x00\x02"  # Language Tag Length
    + b"\x65\x6e"  # Language Tag (en)
    + b"\x00\x00\xff\xff\x00\x07\x44\x45\x46\x41\x55\x4c\x54"
)
ret = sock.sendto(payload, addr)

time.sleep(5)

print("slptool findsrvs service:obmc_console")
payload = (
    b"\x02"  # Version
    + b"\x01"  # Function ID: SRVTYPERQST
    + b"\x00" * 2  # Ignored Length bytes?
    + b"\x35"  # Length
    + b"\x00" * 2  # Flags
    + b"\x00" * 3  # Ext
    + b"\xe5\xc2"  # XID
    + b"\x00\x02"  # Language Tag Length
    + b"\x65\x6e"  # Language Tag (en)
    + b"\x00\x00"  # PR List Length
    + b"\x00\x14service:obmc_console"  # Service
    + b"\x00\x07\x44\x45\x46\x41\x55\x4c\x54"  # Scope
    + b"\x00\x00\x00\x00"  # Predicate and SLP SPI Length
)
ret = sock.sendto(payload, addr)
