#!/bin/bash

rsync $1 -auv --exclude='build' --exclude='*~' --exclude='*ihex' ~/tools/contiki-ng/examples/iot-time-sync/ risgc-ws:tools/contiki-ng/examples/iot-time-sync
