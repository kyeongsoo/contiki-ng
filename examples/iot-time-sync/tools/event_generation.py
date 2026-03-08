#!/usr/bin/env python
# -*- coding: utf-8 -*-
##
# @file     event_generation.py
# @author   Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
# @date     2026-02-17
#
# @brief    Generate triggering events for WSN/IoT applications.
#
# @remarks  Copyright (C) 2026 Kyeong Soo (Joseph) Kim. All rights reserved.
#

import argparse
import logging
import random
import sys
import warnings
warnings.filterwarnings("ignore", module="gpiozero")
# ignore warnings from gpiozero about the lack of lgpio and RPi
# modules, which are not available on non-Raspberry Pi platforms.
from gpiozero import LED
from time import sleep


# set up GPIO output
led = LED(17, initial_value=False)


def generate_events(interarrival_time: float = 1.0, on_period: float = 0.1, num_events: int = 360, log_folder: str = "./log"):
    if interarrival_time <= on_period:
            print("Warning: interarrival time should be greater than on period.")
            sys.exit(1)
    else:
        arrival_rate = 1.0 / (interarrival_time - on_period)  # adjust arrival rate to account for on period
        print(f"Using arrival rate of {arrival_rate:.4f} events/s for event generation.")

    # send messagges to both stdout and a log file
    ####################################################################
    # Source - https://stackoverflow.com/a/24205566
    # Posted by Adam, modified by community. See post 'Timeline' for change history
    # Retrieved 2026-02-17, License - CC BY-SA 3.0
    ####################################################################
    file_name = f"{log_folder}/event_generation.log"
    targets = logging.StreamHandler(sys.stdout), logging.FileHandler(file_name)
    logging.basicConfig(format='%(message)s', level=logging.INFO, handlers=targets)

    print("Generating events ... ")
    # sleep(2) # initial delay

    # post-processing indicator and header row for column names in CSV format
    logging.info("##### BEGIN");
    logging.info("event_number,current_time")

    current_time = 0.0
    # event_number = 0

    # main loop
    for event_number in range(num_events):
        led.off()
        ia_time = random.expovariate(arrival_rate)
        # if (current_time + ia_time > end_time):
        #     break
        sleep(ia_time)
        led.on()
        sleep(on_period) # rising edge duration
        current_time += ia_time + on_period
        logging.info(f"{event_number},{current_time:.4E}")
        # event_number += 1

    logging.info("##### END");


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-I",
        "--interarrival_time",
        help="average interarrival time of events [s]; default is 1.0",
        default=1.0,
        type=float)
    parser.add_argument(
        "-O",
        "--on_period",
        help="on period of output pulse (i.e., rising edge duration) for event triggering [s]; default is 0.1",
        default=0.1,
        type=float)
    parser.add_argument(
        "-N",
        "--num_events",
        help="number of events to generate; default is 360",
        default=360,
        type=int)
    parser.add_argument(
        "-L",
        "--log_folder",
        help="folder for a log file; default is './log'",
        default="",
        type=str)
    args = parser.parse_args()

    # set variables using command-line arguments
    interarrival_time = args.interarrival_time
    on_period = args.on_period
    num_events = args.num_events
    log_folder = args.log_folder.strip('/') # remove any trailing slash

    # set random seed for reproducibility
    random.seed(20260217)

    # generate events
    generate_events(interarrival_time, on_period, num_events, log_folder)
