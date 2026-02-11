CONTIKI_PROJECT = client server
PROJECT_SOURCEFILES += csc.c
TARGET_LIBFILES += -lm

CONTIKI = ../../..

MAKE_MAC ?= MAKE_MAC_CSMA
MAKE_NET = MAKE_NET_NULLNET

# disable TCP, UDP, and 6lowpan fragmentation
DEFINES += UIP_CONF_TCP=0
DEFINES += UIP_CONF_UDP=0
DEFINES += SICSLOWPAN_CONF_FRAG=0

# enable custom payload
DEFINES += NULLNET_EXT=1

include $(CONTIKI)/Makefile.include
all: $(CONTIKI_PROJECT)
