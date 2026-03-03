CONTIKI_PROJECT = csc-analysis csc-client csc-server
all: $(CONTIKI_PROJECT)

PROJECT_SOURCEFILES += csc.c
TARGET_LIBFILES += -lm

MAKE_MAC = MAKE_MAC_CSMA
MAKE_NET = MAKE_NET_NULLNET

# disable TCP, UDP, and 6lowpan fragmentation
DEFINES += UIP_CONF_TCP=0
DEFINES += UIP_CONF_UDP=0
DEFINES += SICSLOWPAN_CONF_FRAG=0

# disable module logs
DEFINES += LOG_CONF_LEVEL_MAIN=0
DEFINES += LOG_CONF_LEVEL_SKY=0

# NullNet extension for custom payload and timestamping
DEFINES += NULLNET_EXT=1

# GPIO extensions for edge detection and timestamping
DEFINES += P2_EXT=6 # edge detection and timestamping at P2.6 (GPIO3) (stable)
# DEFINES += P2_EXT=3 # edge detection and timestamping at P2.3 (GPIO2) (working)
# DEFINES += P2_EXT=1 # edge detection and timestamping at P2.1 (GPIO1) (experimental)
# DEFINES += P2_EXT=0 # edge detection and timestamping at P2.0 (GPIO0) (not working!!!)

# Rtimer extensions
# DEFINES += RTIMER_EXT=1 # 32-bit rtimer running at 32.768 kHz (32.768 kHz ACLK driven by crystal; default)
# DEFINES += RTIMER_EXT=2 # 32-bit rtimer running at 0.4875 MHz (3.9 MHz SMCLK divided by 8; stable)
DEFINES += RTIMER_EXT=3 # 32-bit rtimer running at 0.975 MHz (3.9 MHz SMCLK divided by 4; experimental)
# DEFINES += RTIMER_EXT=4 # 32-bit rtimer running at 1.95 MHz (3.9 MHz SMCLK divided by 2; unstable!!!)

# CSC optimization
DEFINES += NDEBUG # turn off assert()
DEFINES += CSC_INT_SIZE=4 # number of bytes for 'i', 'D', and 'A' (4 or 8)
DEFINES += CSC_DS_OPT1=1 # turn off iteration couting in DS
DEFINES += CSC_DS_OPT2 # enable branchless programming in DS
DEFINES += CSC_DIV_OPT # turn off checking the value of A in division algos

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
