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
    "EVENT_NUMBER_MAX": 720, # maximum number of events to process after CFR initialization; 2 h for 10 s event interarrival time
    "RADIO_OFF_PERIOD": 600 # period of radio off time after each beacon reception in seconds
}


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

# set up for contiker docker container
client = docker.from_env()
containers = client.containers.list(filters={'ancestor': 'contiker/contiki-ng'})
if not containers:
    print("[LOG: main] ERROR: No container found for contiker/contiki-ng")
    sys.exit(1)
container_id = containers[0].id
if container_id is not None:
    container = client.containers.get(container_id)
else:
    print("[LOG: main] ERROR: No container found for contiker/contiki-ng")
    sys.exit(1)
working_dir = "/home/user/contiki-ng/examples/iot-time-sync/csc"

# customize macro values
########################################################################
# DEBUG
########################################################################
# macros["BEACON_INTERVAL"] = 10 # beacon interval in seconds
# macros["NB_SKIP"] = 6 # 1 m
# macros["NB_CFR"] = 60 # 10 m
# macros["EVENT_NUMBER_MAX"] = 720 # ~2 h for 10 s event interarrival time
# macros["RADIO_OFF_PERIOD"] = 3600 # 1 h
########################################################################
# TEST
########################################################################
macros["BEACON_INTERVAL"] = 10 # beacon interval in seconds
macros["NB_SKIP"] = 6 # 1 m
macros["NB_CFR"] = 180 # 30 m
macros["EVENT_NUMBER_MAX"] = 120 # ~2 h for 60 s event interarrival time
macros["RADIO_OFF_PERIOD"] = 0 # no radio off time for testing
del macros["NDEBUG"] # turn on assert() and debug-related routines

# prepare commands for CSC on TelosB motes and event generation on a Raspberry Pi
macros_str = "".join([f"DEFINES+={k}={v} " for k, v in macros.items()])
csc_commands = [
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
eg_command = [
        "python", "../tools/event_generation.py",
        "--on_period", "0.1",
        "--log_folder", f"./log/{datetime_string}",
        "--num_events", f"{int(macros['EVENT_NUMBER_MAX']+5)}", # should be greater than 'EVENT_NUMBER_MAX'
        # "--interarrival_time", "10.0" # DEBUG
        "--interarrival_time", "60.0" # TEST
    ]

# save commands and macros to a JSON file for later use
settings = {
    "csc_commands": csc_commands,
    "eg_command": ' '.join(eg_command),
    "macros": macros
}
settings_file = f"./log/{datetime_string}/csc-client-server_settings.json"
with open(settings_file, "w", encoding="utf-8") as f:
    json.dump(settings, f, sort_keys=True, indent=4)

# processes to run in the container for TelosB motes
for command in csc_commands:
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

# monitoring process for the server
command = "serialdump /dev/ttyUSB0 > " + f"./log/{datetime_string}/csc-server.log"
print(f"[LOG: main] {command}")
process_server = subprocess.Popen(
    command,
    shell=True,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT
)
print(f"[LOG: main] Started with PID={process_server.pid} ...")

# monitoring process for the client, which also trigger the event
# generation on the remote Raspberry Pi
command = "serialdump /dev/ttyUSB1 | tee " + f"./log/{datetime_string}/csc-client.log"
print(f"[LOG: main] {command}")
process_client = subprocess.Popen(
    command,
    shell=True,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT
)
while True:
    byte_line = process_client.stdout.readline()
    if not byte_line:
        break
    line = byte_line.decode('utf-8', errors='replace').strip()
    print(line.rstrip()) # Process or print the line in real-time
    sys.stdout.flush()

    match = re.search(r"CFR initialized:", line)
    if match:
        process_event_generation = subprocess.Popen(
            eg_command,
            env=custom_env,
            stdin=subprocess.DEVNULL,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            close_fds=True
        )
        print(f"[LOG: main] Event generation on the remote Raspberry Pi started with PID={process_event_generation.pid} ...")

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

# # terminate the event generation process
# process_event_generation.terminate()
