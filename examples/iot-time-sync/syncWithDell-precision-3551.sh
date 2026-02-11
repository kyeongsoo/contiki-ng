#!/bin/bash

rsync $1 -auv --exclude='build' --exclude='*~' --exclude='*ihex' ~/tools/contiki-ng/examples/iot-time-sync/ dell-precision-3551:tools/contiki-ng/examples/iot-time-sync
