# Clock skew compensation (CSC)

## Introduction
### TBD

## Experimental setup
### TBD
1. To decide the size of timestamps (i.e., 16 or 32 bits?) based on the time periods for synchronzation and event monitoring on the on Telosb motes with 16-bit MSP430 microcontroller.
2. To turn off the radio after the completion of cumulative ratio (CR) estimation of clock frequency ratio (CFR) in order to avoid any interference between Zigbee frame reception and GPIO event detection.

## Configuration options
Before building a full system image, the following macros should be defined in a project Makefile (e.g., `DEFINES += CSC_AD_SIZE=4`):
### CSC algorithms
- `CSC_INT_SIZE`: To specify the number of bytes (i.e., 4 or 8) for integers representing `i`, `D`, and `A` of CSC algorithms.
- `CSC_DS_OPT1`: To turn off iteration couting in DS.
- `CSC_DS_OPT2`: To enable branchless programming in DS.
- `CSC_DIV_OPT`: To turn off checking the value of `A` in the division algorithms.
### Component extensions
- `NULLNET_EXT`: To enable user-defined data (e.g., transmission timestamps).
- `P2_EXT`: To enable edge detection and timestamping at P2.x; x could be 3 or 6.
- `RTIMER_EXT`: To extend the 16-bit rtimer to 32-bit one on the sky plastform (e.g., TelosB).

## How to run a project on a TelosB mote
1. First check the port (e.g., "/dev/ttyUSB0").
```
make motelist-all
```
2. Build and run the project on the mote with logging (e.g., "csc-analysis" project).
```
make TARGET=sky MOTES=/dev/ttyUSB0 csc-analysis.upload
make TARGET=sky MOTES=/dev/ttyUSB0 login | tee ./log/csc-analysis_$(date +'%Y%m%d%H%M%S').log
```
3. Process the log file (e.g., ".log/csc-analysis.log")
```
log-process.sh ./log/csc-analysis.log
python csc-analysis.py -l ./log/csc-analysis.log
```

## References
<a id="1">[1]</a> 
Surya Siddharth, [Interrupts in MSP430 – Writing GPIO Interrupt Program using Code Composer Studio](https://circuitdigest.com/microcontroller-projects/interrupts-in-msp430-writing-gpio-interrupt-program-using-code-composer-studio), Aug. 4, 2020.
