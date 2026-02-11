#/bin/bash

# from
rsync $1 -auvz --exclude='build' --exclude='*~' --exclude='*ihex' risgc-ws-rpi2:tools/contiki-ng/examples/iot-time-sync/ ~/tools/contiki-ng/examples/iot-time-sync
# to
rsync $1 -auvz --exclude='build' --exclude='*~' --exclude='*ihex' ~/tools/contiki-ng/examples/iot-time-sync/ risgc-ws-rpi2:tools/contiki-ng/examples/iot-time-sync
