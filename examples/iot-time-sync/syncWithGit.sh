#!/bin/bash

#rsync $1 -auv --exclude='build' --exclude='*~' --exclude='*ihex' ~/research/synchronization/iot_time_sync/telosb/contiki-ng/ ~/tools/contiki-ng/examples/iot-time-sync
rsync $1 -auv --exclude='build' --exclude='*~' --exclude='*ihex' ~/tools/contiki-ng/examples/iot-time-sync/ ~/research/synchronization/iot_time_sync/telosb/contiki-ng
