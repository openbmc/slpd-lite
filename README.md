# slpd-lite

## Building

The repository is built using meson.

`meson setup builddir && ninja -C builddir`

## Details

SLPD:-This is a unicast SLP UDP server which serves the following two messages:

1. finsrvs
2. findsrvtypes

NOTE:- Multicast support is not there and this server neither listen to any
advertisement messages nor it advertises it's services with DA.
