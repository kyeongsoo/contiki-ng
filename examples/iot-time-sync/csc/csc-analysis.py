#!/usr/bin/env python
# -*- coding: utf-8 -*-
##
# @file     csc-analysis.py
# @author   Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
# @date     2026-02-07
#
# @brief    Provide a statistical summary of the results of the performance
#           evaluation of clock skew estimation algorithms.
#
# @remarks  Copyright (C) 2026 Kyeong Soo (Joseph) Kim. All rights reserved.
#
# @remarks  This software is written and distributed under the GNU General
#           Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
#           You must not remove this notice, or any other, from this software.
#

import argparse
import pandas as pd
from pathlib import Path

RTIMER_SECOND = 32768 # number of rtimer ticks per second for sky/TelosB platform in contiki-ng

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-l",
        "--log_file",
        help="log file name",
        default="csc-analysis.log",
        type=str)
    arg = parser.parse_args()

    alg_names = {"ds": "Direct Search", "sp": "Single-Precision Division"}
    log_file = arg.log_file
    df = pd.read_csv(log_file, header=0)
    df['elapsed_us'] = df['elapsed_ticks'] * 1.0E6 / RTIMER_SECOND

    md_file = (Path(log_file).with_suffix("").with_suffix(".md"))
    pd.set_option('display.float_format', '{:.4e}'.format)
    with open(md_file, "w") as f:
        for p in range(6, 10):
            i = 10**p
            f.write(f"# i={i:.0e}\n")
            for alg in ['ds', 'sp']:
                f.write("## " + alg_names[alg] + "\n")
                f.write(df[(df['alg'] == alg) & (df['i'] == i)].iloc[:,5:].describe().to_string() + "\n")
