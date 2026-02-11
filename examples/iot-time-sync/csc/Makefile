CONTIKI_PROJECT = csc-analysis csc-client csc-server gpio-trigger
all: $(CONTIKI_PROJECT)

PROJECT_SOURCEFILES += csc.c
TARGET_LIBFILES += -lm

CONTIKI = ../../..

MAKE_MAC = MAKE_MAC_CSMA
MAKE_NET = MAKE_NET_NULLNET

# disable TCP, UDP, and 6lowpan fragmentation
DEFINES += UIP_CONF_TCP=0
DEFINES += UIP_CONF_UDP=0
DEFINES += SICSLOWPAN_CONF_FRAG=0

# disable module logs
DEFINES += LOG_CONF_LEVEL_MAIN=0
DEFINES += LOG_CONF_LEVEL_SKY=0

# enable my extensions
DEFINES += NULLNET_EXT=1
# DEFINES += P2_EXT=1 # edge detection and timestamping at P2.1 (GIO1) # now working!
DEFINES += P2_EXT=3 # edge detection and timestamping at P2.3 (GIO2) # working!
# DEFINES += P2_EXT=6 # edge detection and timestamping at P2.6 (GIO3) # working!
DEFINES += RTIMER_EXT=1

include $(CONTIKI)/Makefile.include
