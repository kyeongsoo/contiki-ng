CONTIKI_PROJECT = gpio-test
all: $(CONTIKI_PROJECT)

MAKE_MAC = MAKE_MAC_CSMA
MAKE_NET = MAKE_NET_NULLNET

# DEFINES += P2_EXT=0 # edge detection and timestamping at P2.0 (GIO0 # now working!
# DEFINES += P2_EXT=1 # edge detection and timestamping at P2.1 (GIO1) # now working!
# DEFINES += P2_EXT=3 # edge detection and timestamping at P2.3 (GIO2) # working!
DEFINES += P2_EXT=6 # edge detection and timestamping at P2.6 (GIO3) # working!
DEFINES += RTIMER_EXT=1 # 32-bit extended rtimer

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
