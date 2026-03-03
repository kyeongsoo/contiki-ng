# MODULES += dev/sensor/sht11

CONTIKI_PROJECT = sys-info
all: $(CONTIKI_PROJECT)

# Rtimer extensions
# DEFINES += RTIMER_EXT=1 # 32-bit rtimer running at 32.768 kHz (32.768 kHz ACLK driven by crystal; default)
# DEFINES += RTIMER_EXT=2 # 32-bit rtimer running at 0.4875 MHz (3.9 MHz SMCLK divided by 8; stable)
# DEFINES += RTIMER_EXT=3 # 32-bit rtimer running at 0.975 MHz (3.9 MHz SMCLK divided by 4; experimental)
# DEFINES += RTIMER_EXT=4 # 32-bit rtimer running at 1.95 MHz (3.9 MHz SMCLK divided by 2; unstable!!!)

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
