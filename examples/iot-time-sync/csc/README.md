# Clock skew compensation (CSC)

## Introduction
TBD

## Experimental Setup
TBD: To decide the size of timestamps (i.e., 16 or 32 bits?) based on the time
periods for synchronzation and event monitoring on the on Telosb motes with
16-bit MSP430 microcontroller.

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
```

## References
<a id="1">[1]</a> 
Surya Siddharth, [Interrupts in MSP430 – Writing GPIO Interrupt Program using Code Composer Studio](https://circuitdigest.com/microcontroller-projects/interrupts-in-msp430-writing-gpio-interrupt-program-using-code-composer-studio), Aug. 4, 2020.
