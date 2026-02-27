CONTIKI_PROJECT = rtimer-ext-test rtimer-ext-calibration
all: $(CONTIKI_PROJECT)

MAKE_MAC = MAKE_MAC_CSMA
MAKE_NET = MAKE_NET_NULLNET

# GPIO extensions for edge detection and timestamping
# DEFINES += P2_EXT=6 # edge detection and timestamping at P2.6 (GPIO3) (stable)
# DEFINES += P2_EXT=3 # edge detection and timestamping at P2.3 (GPIO2) (working)
# DEFINES += P2_EXT=1 # edge detection and timestamping at P2.1 (GPIO1) (experimental)
# DEFINES += P2_EXT=0 # edge detection and timestamping at P2.0 (GPIO0) (not working!!)

# Rtimer extensions for 32-bit extended rtimer and microsecond-level timing
DEFINES += RTIMER_EXT # 32-bit extended rtimer
DEFINES += US_EXT=8 # 0.4875 MHz (SMCLK divided by 8; stable)
# DEFINES += US_EXT=4 # 0.975 MHz (SMCLK divided by 4; experimental)
# DEFINES += US_EXT=2 # 1.95 MHz (SMCLK divided by 2; unstable!!!)

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
