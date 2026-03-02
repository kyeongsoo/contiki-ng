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
from datetime import datetime
from gpiozero import LED
from time import sleep


# set up GPIO output
led = LED(17, initial_value=False)


def generate_events(interarrival_time: float = 360.0, on_period: float = 0.1, end_time: float = 3600.0):
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
    now = datetime.now()
    timestamp = now.strftime("%Y%m%d%H%M%S")
    file_name = f"./log/event_generation_{timestamp}.log"
    targets = logging.StreamHandler(sys.stdout), logging.FileHandler(file_name)
    logging.basicConfig(format='%(message)s', level=logging.INFO, handlers=targets)

    print("Generating events ... ")
    sleep(2) # initial delay

    # post-processing indicator and header row for column names in CSV format
    logging.info("##### BEGIN");
    logging.info("event_number,current_time")

    current_time = 0.0
    event_number = 0

    # main loop
    while True:
        led.off()
        ia_time = random.expovariate(arrival_rate)
        if (current_time + ia_time > end_time):
            break
        sleep(ia_time)
        led.on()
        sleep(on_period) # rising edge duration
        current_time += ia_time + on_period
        logging.info(f"{event_number},{current_time:.4E}")
        event_number += 1

    logging.info("##### END");
    # input("Press enter to exit: ")
    # print("Event generation is terminated.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-I",
        "--interarrival_time",
        help="average interarrival time of events [s]; default is 360.0 (0.1 hours)",
        default=360.0,
        type=float)
    parser.add_argument(
        "-O",
        "--on_period",
        help="on period of output pulse (i.e., rising edge duration) for event triggering [s]; default is 0.1",
        default=0.1,
        type=float)
    parser.add_argument(
        "-E",
        "--end_time",
        help="end time of event generation [s]; default is 3600.0 (1 hour)",
        default=3600.0,
        type=float)
    args = parser.parse_args()

    # set variables using command-line arguments
    interarrival_time = args.interarrival_time
    on_period = args.on_period
    end_time = args.end_time

    # set random seed for reproducibility
    random.seed(12345)

    # generate events
    generate_events(interarrival_time, on_period, end_time)
