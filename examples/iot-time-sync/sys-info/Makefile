# MODULES += dev/sensor/sht11

CONTIKI_PROJECT = sys-info
all: $(CONTIKI_PROJECT)

# Rtimer extensions for 32-bit extended rtimer and microsecond-level timing
DEFINES += RTIMER_EXT # 32-bit extended rtimer
DEFINES += US_EXT=8 # 0.4875 MHz (SMCLK divided by 8)
# DEFINES += US_EXT=4 # 0.975 MHz (SMCLK divided by 4; experimental)
# DEFINES += US_EXT=2 # 1.95 MHz (SMCLK divided by 2; unstable!!!)

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
