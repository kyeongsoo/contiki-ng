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
import json
import os
import re
import subprocess
import sys
from gpiozero import LED
from time import sleep


# customize the environment for the remote Raspberry Pi
custom_env = os.environ.copy()
custom_env["PIGPIO_ADDR"] = "192.168.5.100"

# set up the default values of macros for contiki-ng and its extensions
macros = {
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
    # Rtimer extensions
    "RTIMER_EXT": 3, # 32-bit rtimer running at 0.975 MHz (3.9 MHz SMCLK divided by 4; experimental)
    # CSC optimization
    "NDEBUG": 1, # turn off assert()
    # CSC optimization
    "CSC_INT_SIZE": 8, # number of bytes for 'i', 'D', and 'A' (4 or 8)
    "CSC_NO_ITER_COUNT": 1, # turn off iteration counting in iterative algorithms
    "CSC_NO_DIV_CHECK": 1, # turn off checking the value of A in division algorithms
    # experimental setup
    "BEACON_INTERVAL": 10, # beacon interval in seconds
    "NB_SKIP": 10, # number of initial beacons to skip before CFR initialization
    "NB_CFR": 100, # number of beacons for CFR initialization
    "ELAPSED_TIME_MAX": 3600, # maximum elapsed time in seconds after CFR initialization
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

# customize macro values
########################################################################
# DEBUG
########################################################################
# macros["BEACON_INTERVAL"] = 10 # beacon interval in seconds
# macros["NB_SKIP"] = 6 # 1 m
# macros["NB_CFR"] = 60 # 10 m
# macros["ELAPSED_TIME_MAX"] = 36000 # 10 h
# macros["RADIO_OFF_PERIOD"] = 3600 # 1 h
########################################################################
# TEST
########################################################################
macros["BEACON_INTERVAL"] = 1 # beacon interval in seconds
macros["NB_SKIP"] = 5
macros["NB_CFR"] = 10
macros["ELAPSED_TIME_MAX"] = 360 # 6 m
macros["RADIO_OFF_PERIOD"] = 60

# processes to run in the container for TelosB motes
macros_str = "".join([f"DEFINES+={k}={v} " for k, v in macros.items()])
commands = [
    # clean
    "make -f Makefile.client-server clean",
    # build and upload csc-server
    "make -f Makefile.client-server "
    + macros_str
    + "MOTES=/dev/ttyUSB0 csc-server.upload",
    # build and upload csc-client
    "make -f Makefile.client-server "
    + macros_str
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

# datetime string for a directory for log files
now = datetime.datetime.now()
datetime_string = now.strftime("%Y%m%d%H%M%S")
try:
    os.mkdir("./log/" + datetime_string)
except FileExistsError:
    print(f"[LOG: main] Directory './log/{datetime_string}' already exists. Log files will be overwritten.")
except Exception as e:
    print(f"[LOG: main] ERROR: Failed to create directory './log/{datetime_string}': {e}")
    sys.exit(1)

# save commands and macros to a JSON file for later use
settings = {
    "commands": commands,
    "macros": macros
}
settings_file = f"./log/{datetime_string}/csc-client-server_settings.json"
with open(settings_file, "w", encoding="utf-8") as f:
    json.dump(settings, f, sort_keys=True, indent=4)

# monitoring process for the server
command = "serialdump /dev/ttyUSB0 | tee " + f"./log/{datetime_string}/csc-server.log"
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
command = "serialdump /dev/ttyUSB1 | tee " + f"./log/{datetime_string}/csc-client.log"
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
        process_event_generation = subprocess.Popen(
            [
                "python", "../tools/event_generation.py",
                "--interarrival_time", "10.0",
                "--on_period", "0.1",
                "--end_time", f"{int(macros['ELAPSED_TIME_MAX']*1.1)}", # with a guard time
                "--log_folder", f"./log/{datetime_string}"
            ], # DEBUG
            # [
            #     "python", "../tools/event_generation.py",
            #     "--interarrival_time", "1.0",
            #     "--on_period", "0.1",
            #     "--end_time", f"{int(macros['ELAPSED_TIME_MAX']*1.1)}", # with a guard time
            #     "--log_folder", f"./log/{datetime_string}"
            # ], # TEST
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
stdout, stderr = process_event_generation.communicate()
for line in stdout.splitlines():
    print(f"[LOG: event_generation.py] {line}")
process_event_generation.terminate()
