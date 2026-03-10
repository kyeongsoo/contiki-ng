#!/usr/bin/env python
# -*- coding: utf-8 -*-
##
# @file     csc-client-server.py
# @author   Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
# @date     2026-02-18
#
# @brief    Provide a statistical summary and a plot of the results of the performance
#           evaluation of clock skew estimation algorithms on TelosB client and server.
#
# @remarks  Copyright (C) 2026 Kyeong Soo (Joseph) Kim. All rights reserved.
#
# @remarks  This software is written and distributed under the GNU General
#           Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
#           You must not remove this notice, or any other, from this software.
#

import argparse
import json
import matplotlib.pyplot as plt
import pandas as pd
import re
from io import StringIO
from pathlib import Path


def log_to_string(log_file: str):
    """Prefilter and convert a log file to a single string."""
    filtered_lines = []
    cvs_start = False
    with open(log_file, 'r', encoding='ISO-8859-1') as file:
        for line in file:
            if cvs_start == False:
                if "##### BEGIN" in line:
                    cvs_start = True
            else:
                match = re.search(r"\[DBG|\[INFO", line)
                if match:
                    # skip
                    continue
                elif "##### END" in line:
                    cvs_start = False
                else:
                    filtered_lines.append(line.strip())
    return "\n".join(filtered_lines)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-L",
        "--log_folder",
        help="folder for a log file; default is './log'",
        default="",
        type=str)
    parser.add_argument(
        "-r",
        "--rtimer_second",
        help="the number of rtimer ticks per second for sky/TelosB platform in contiki-ng; default is 975000 (i.e., 0.975 MHz)",
        default=975000,
        type=int)
    args = parser.parse_args()
    log_folder = args.log_folder.strip('/') # remove any trailing slash
    rtimer_second = args.rtimer_second

    client_log_file = f"{log_folder}/csc-client.log"
    server_log_file = f"{log_folder}/csc-server.log"
    df_c = pd.read_csv(StringIO(log_to_string(client_log_file)), header=0)
    df_s = pd.read_csv(StringIO(log_to_string(server_log_file)), header=0)
    df = pd.merge(df_s, df_c, on=['event_number'], how='inner')
    df['elapsed_second'] = df['elapsed_ticks'] / rtimer_second  
    df['i_err'] = (df['i'] - df['elapsed_ticks']) / rtimer_second
    df['eds_err'] = (df['eds'] - df['elapsed_ticks']) / rtimer_second
    df['sp_err'] = (df['sp'] - df['elapsed_ticks']) / rtimer_second
    df['i_abs_err'] = abs(df['i'] - df['elapsed_ticks']) / rtimer_second
    df['eds_abs_err'] = abs(df['eds'] - df['elapsed_ticks']) / rtimer_second
    df['sp_abs_err'] = abs(df['sp'] - df['elapsed_ticks']) / rtimer_second

    # save the dataframe to a pickle file for later use
    pkl_file = Path(client_log_file).with_suffix("").with_suffix(".pkl")
    parts = list(pkl_file.parts)
    parts[-1] = parts[-1].replace('client', 'client-server')
    pkl_file = Path(*parts)
    df.to_pickle(pkl_file)

    json_file = f"{log_folder}/csc-client-server_settings.json"
    with open(json_file, "r") as f:
        settings = json.load(f)
    md_file = pkl_file.with_suffix("").with_suffix(".md")
    alg_names = {"i": "Uncompensated", "eds": "Compensated (Direct Search)", "sp": "Compensated (Single-Precision Division)"}
    pd.set_option('display.float_format', '{:.4e}'.format)
    with open(md_file, "w") as f:
        f.write("# CSC Client and Server Settings\n")
        for k, v in settings.items():
            f.write(f"- {k}: {v}\n")
        for alg in ['eds', 'sp']:
            f.write(f"# {alg_names[alg]} Error\n")
            for k, v in df[alg + '_err'].describe().items():
                f.write(f"- {k}: {v:.4e}\n")
            # f.write(df[alg + '_err'].describe().to_string() + "\n")

    pdf_file = md_file.with_suffix("").with_suffix(".pdf")
    xmin = df['elapsed_second'].min()
    xmax = df['elapsed_second'].max()
    ymin = df[['i_err', 'eds_err', 'sp_err']].min().min()
    ymax = df[['i_err', 'eds_err', 'sp_err']].max().max()
    df.plot(x='elapsed_second', y=['i_err', 'eds_err', 'sp_err'], kind='line',
        xlabel='Time [s]', ylabel='Event Time Estimation Error [s]',
        label=[alg_names['i'], alg_names['eds'], alg_names['sp']],
        grid=True, legend=True, figsize=(10, 6), xlim=(xmin, xmax),
        ylim=(ymin, ymax))
    # ax = df.plot(kind='scatter', x='elapsed_second', y='i_err', color='red', marker='x',
    #     xlabel='Time [s]', ylabel='Event Time Estimation Error [s]',
    #     label=alg_names['i'], grid=True, legend=True, figsize=(10, 6),
    #     xlim=(xmin, xmax), ylim=(ymin, ymax))
    # df.plot(kind='scatter', x='elapsed_second', y='eds_err', color='green', marker='o',
    #     xlabel='Time [s]', ylabel='Event Time Estimation Error [s]',
    #     label=alg_names['eds'], grid=True, legend=True, figsize=(10, 6),
    #     ax=ax)
    # df.plot(kind='scatter', x='elapsed_second', y='sp_err', color='blue', marker='^',
    #     xlabel='Time [s]', ylabel='Event Time Estimation Error [s]',
    #     label=alg_names['sp'], grid=True, legend=True, figsize=(10, 6),
    #     ax=ax)
    plt.show()
    plt.savefig(pdf_file)