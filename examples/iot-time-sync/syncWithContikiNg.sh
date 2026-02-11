#!/bin/bash

rsync $1 -auv --exclude='*~' --exclude='*new' ~/tools/contiki-ng/examples/iot-time-sync/csc/contiki-ng-ext/ ~/tools/contiki-ng
