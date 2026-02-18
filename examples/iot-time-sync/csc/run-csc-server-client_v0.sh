#!/bin/sh
make -f Makefile.client-server clean && make -f Makefile.client-server MOTES=/dev/ttyUSB0 csc-server.upload && make -f Makefile.client-server clean && make -f Makefile.client-server MOTES=/dev/ttyUSB1 csc-client.upload
