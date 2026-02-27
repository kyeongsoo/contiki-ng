<img src="https://github.com/contiki-ng/contiki-ng.github.io/blob/master/images/logo/Contiki_logo_2RGB.png" alt="Logo" width="256">

# Contiki-NG TelosB Extension

TelosB mote is a legacy open-source platform widely used in the research and development of wireless sensor network (WSN) algorithms and protocols. It is based on Texas Instruments [MSP430F1611](https://www.ti.com/product/MSP430F1611)---an 8 MHz MCU with 48KB Flash, 10KB SRAM, 12-bit ADC, Dual 12-bit DAC, comparator, DMA, I2C/SPI/UART---and [CC2420](https://www.ti.com/product/CC2420)--a single-chip 2.4 GHz IEEE 802.15.4 compliant RF transceiver---and also integrates humidity, temperature, and light sensors.

The support of TelosB in Contiki-NG is provided through sky platform, which, however, is outdated and limited (e.g., a 16-bit rtimer running at 32,768 Hz and no general support of GPIO).

This fork, therefore, extends Contiki-NG to provide enhanced and new capabilities with the following macro definitions:

* Custom payload in NullNet.
* GPIO support for detecting external trigger:
  - P2_EXT=6: Rising edge detection and timestamping at P2.6 (GIO3) (stable)
  - P2_EXT=3: Rising edge detection and timestamping at P2.3 (GIO2) (working)
  - P2_EXT=1: Rising edge detection and timestamping at P2.3 (GIO2) (experimental)
* 32-bit extension of rtimer:
  - RTIMER_EXT
* Microsecnd resolution extension of rtimer based on SMCLK running at 3.9 MHz:
  - US_EXT=8: 0.4875 MHz (SMCLK divided by 8)
  - US_EXT=4: 0.975 MHz (SMCLK divided by 4; experimental)
  - US_EXT=2: 1.95 MHz (SMCLK divided by 2; unstable!!!)
