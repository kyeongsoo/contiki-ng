#!/usr/bin/env python
# -*- coding: utf-8 -*-
##
# @file     rtimer-ext-calibration.py
# @author   Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
# @date     2026-03-02
#
# @brief    Automate running CSC experiments based on TelosB client/server
#           and a remote Raspberry Pi.
#
# @remarks  Make sure that pigpiod daemon is running on a remote Raspberry Pi.
#
# @remarks  Copyright (C) 2026 Kyeong Soo (Joseph) Kim. All rights reserved.
#
# @remarks  This software is written and distributed under the GNU General
#           Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
#           You must not remove this notice, or any other, from this software.
#

import datetime
import docker
import os
import re
import subprocess
import sys
from gpiozero import LED
from time import sleep


# customize the environment for the remote Raspberry Pi
custom_env = os.environ.copy()
custom_env["PIGPIO_ADDR"] = "192.168.5.100"

# set up default values for contiki-ng DEFINES macro
defines = {
    # disable TCP, UDP, and 6lowpan fragmentation
    "UIP_CONF_TCP": 0,
    "UIP_CONF_UDP": 0,
    "SICSLOWPAN_CONF_FRAG": 0,
    # disable module logs
    "LOG_CONF_LEVEL_MAIN": 0,
    "LOG_CONF_LEVEL_SKY": 0,
    # NullNet extension for custom payload and timestamping
    "NULLNET_EXT": 1,
    # GPIO extensions for edge detection and timestamping
    "P2_EXT": 6, # edge detection and timestamping at P2.6 (GPIO3) (stable)
    # Rtimer extensions for 32-bit extended rtimer and microsecond-level timing
    "RTIMER_EXT": 1, # 32-bit extended rtimer
    "US_EXT": 8, # 0.4875 MHz (SMCLK divided by 8)
    # CSC optimization
    "NDEBUG": 1, # turn off assert()
    # CSC optimization
    "CSC_INT_SIZE": 4, # number of bytes for 'i', 'D', and 'A' (4 or 8)
    "CSC_DS_OPT1": 1, # turn off iteration couting in DS
    "CSC_DS_OPT2": 1, # enable branchless programming in DS
    "CSC_DIV_OPT": 1, # turn off checking the value of A in division algos
    # experimental setup
    "BEACON_INTERVAL": 10, # beacon interval in seconds
    "ELAPSED_TIME_MAX": 3600, # maximum elapsed time in seconds after CFR initialization
    "NB_CFR": 100, # number of beacons for CFR initialization
    "NB_SKIP": 10, # number of initial beacons to skip before CFR initialization
    "RADIO_OFF_PERIOD": 600 # period of radio off time after each beacon reception in seconds
}


# set up for contiker docker container
client = docker.from_env()
containers = client.containers.list(filters={'ancestor': 'contiker/contiki-ng'})
if not containers:
    print("[LOG: main] ERROR: No container found for contiker/contiki-ng")
    sys.exit(1)
container_id = containers[0].id
container = client.containers.get(container_id)
working_dir = "/home/user/contiki-ng/examples/iot-time-sync/csc"

# customize contiki-ng DEFINES macro
# DEBUG
defines["US_EXT"] = 4 # 0.975 MHz (SMCLK divided by 4; experimental)
defines["BEACON_INTERVAL"] = 1 # beacon interval in seconds
defines["ELAPSED_TIME_MAX"] = 600 # maximum elapsed time in seconds after CFR initialization
defines["NB_CFR"] = 20
defines["NB_SKIP"] = 10
defines["RADIO_OFF_PERIOD"] = 60
# DEBUG

# processes to run in the container for TelosB motes
defines_str = "".join([f"DEFINES+={k}={v} " for k, v in defines.items()])
commands = [
    # clean
    "make -f Makefile.client-server clean",
    # build and upload csc-server
    "make -f Makefile.client-server "
    + defines_str
    + "MOTES=/dev/ttyUSB0 csc-server.upload",
    # build and upload csc-client
    "make -f Makefile.client-server "
    + defines_str
    + "MOTES=/dev/ttyUSB1 csc-client.upload"
    ]
for command in commands:
    print(f"[LOG: main] {command}")
    exit_code, output_stream = container.exec_run(
        cmd=command,
        user="user",
        workdir=working_dir,
        tty=True,
        stream=True,
        demux=False
    )
    for line in output_stream:
        line = line.decode('utf-8').strip()
        print(line)
        sys.stdout.flush()

# datetime string for monitoring logs
now = datetime.datetime.now()
datetime_string = now.strftime("%Y%m%d%H%M%S")

# monitoring process for the server
command = "serialdump /dev/ttyUSB0 | tee ./log/csc-server_" + datatime_string + ".log"
print(f"[LOG: main] {command}")
process_server = subprocess.Popen(
    command,
    shell=True,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT
    # text=True
)
# while True:
#     byte_line = process_server.stdout.readline()
#     if not byte_line:
#         break
#     line = byte_line.decode('utf-8', errors='replace').strip()
#     print(line.rstrip()) # Process or print the line in real-time
#     sys.stdout.flush()

# monitoring process for the client, which also trigger the event
# generation on the remote Raspberry Pi
command = "serialdump /dev/ttyUSB1 | tee ./log/csc-client_" + datetime_string + ".log"
print(f"[LOG: main] {command}")
process_client = subprocess.Popen(
    command,
    shell=True,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT
    # text=True
)
# for line in process_client.stdout:
#     print(line.rstrip()) # Process or print the line in real-time
#     sys.stdout.flush()
while True:
    byte_line = process_client.stdout.readline()
    if not byte_line:
        break
    line = byte_line.decode('utf-8', errors='replace').strip()
    print(line.rstrip()) # Process or print the line in real-time
    sys.stdout.flush()

    match = re.search(r"CFR initialized:", line)
    if match:
        print("[LOG: main] Start event generation on the remote Raspberry Pi ...")
        # run as a background process to avoid blocking the main process
        process = subprocess.Popen(
            # ["python", "../tools/event_generation.py"],
            [
                "python", "../tools/event_generation.py",
                "--interarrival_time", "10.0",
                "--on_period", "0.1",
                "--end_time", "610.0" # with a guard time of 10 s
            ], # DEBUG
            env=custom_env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True
        )

    match = re.search(r"##### END", line)
    if match:
        # for process_to_kill in ['serialdump', 'tee']:
        for process_to_kill in ['serialdump']:
            try:
                results = subprocess.run(
                    ['pkill', '-x', process_to_kill],
                    stdout=subprocess.PIPE, # merge stdout and stderr
                    stderr=subprocess.STDOUT, 
                    text=True,
                    check=True
                )
                # workaround for the failure in capturing pkill output
                print(f"[LOG: {process_to_kill}] ", end="")
            except subprocess.CalledProcessError:
                print(f"[LOG: {process_to_kill}] {results.stdout}.")

# check and print the output of the background process
stdout, stderr = process.communicate()
for line in stdout.splitlines():
    print(f"[LOG: event_generation.py] {line}")
process.terminate()
