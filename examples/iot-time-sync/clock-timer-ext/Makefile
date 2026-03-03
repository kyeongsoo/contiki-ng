CONTIKI_PROJECT = rtimer-ext-test rtimer-ext-calibration
all: $(CONTIKI_PROJECT)

MAKE_MAC = MAKE_MAC_CSMA
MAKE_NET = MAKE_NET_NULLNET

# GPIO extensions for edge detection and timestamping
DEFINES += P2_EXT=6 # edge detection and timestamping at P2.6 (GPIO3) (stable)
# DEFINES += P2_EXT=3 # edge detection and timestamping at P2.3 (GPIO2) (working)
# DEFINES += P2_EXT=1 # edge detection and timestamping at P2.1 (GPIO1) (experimental)
# DEFINES += P2_EXT=0 # edge detection and timestamping at P2.0 (GPIO0) (not working!!)

# Rtimer extensions
# DEFINES += RTIMER_EXT=1 # 32-bit rtimer running at 32.768 kHz (32.768 kHz ACLK driven by crystal; default)
# DEFINES += RTIMER_EXT=2 # 32-bit rtimer running at 0.4875 MHz (3.9 MHz SMCLK divided by 8; stable)
DEFINES += RTIMER_EXT=3 # 32-bit rtimer running at 0.975 MHz (3.9 MHz SMCLK divided by 4; experimental)
# DEFINES += RTIMER_EXT=4 # 32-bit rtimer running at 1.95 MHz (3.9 MHz SMCLK divided by 2; unstable!!!)

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
