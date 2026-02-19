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
import matplotlib.pyplot as plt
import pandas as pd
from io import StringIO
from pathlib import Path


RTIMER_SECOND = 32768 # number of rtimer ticks per second for sky/TelosB platform in contiki-ng


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
                if "##### END" in line:
                    cvs_start = False
                else:
                    filtered_lines.append(line.strip())
    return "\n".join(filtered_lines)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c",
        "--client_log_file",
        help="client log file name",
        default="csc-client.log",
        type=str)
    parser.add_argument(
        "-s",
        "--server_log_file",
        help="server log file name",
        default="csc-server.log",
        type=str)
    arg = parser.parse_args()
    client_log_file = arg.client_log_file
    server_log_file = arg.server_log_file

    df_c = pd.read_csv(StringIO(log_to_string(client_log_file)), header=0)
    df_s = pd.read_csv(StringIO(log_to_string(server_log_file)), header=0)
    df = pd.merge(df_s, df_c, on=['event_number'], how='inner')
    df['elapsed_second'] = df['elapsed_ticks'] / RTIMER_SECOND
    df['i_err'] = (df['i'] - df['elapsed_ticks']) / RTIMER_SECOND
    df['ds_err'] = (df['ds'] - df['elapsed_ticks']) / RTIMER_SECOND
    df['sp_err'] = (df['sp'] - df['elapsed_ticks']) / RTIMER_SECOND

    # save the dataframe to a pickle file for later use
    pkl_file = Path(client_log_file).with_suffix("").with_suffix(".pkl")
    parts = list(pkl_file.parts)
    parts[-1] = parts[-1].replace('client', 'client-server')
    pkl_file = Path(*parts)
    df.to_pickle(pkl_file)

    md_file = pkl_file.with_suffix("").with_suffix(".md")
    alg_names = {"i": "Uncompensated", "ds": "Compensated (Direct Search)", "sp": "Compensated (Single-Precision Division)"}
    pd.set_option('display.float_format', '{:.4e}'.format)
    with open(md_file, "w") as f:
        for alg in ['ds', 'sp']:
            f.write("# " + alg_names[alg] + "\n")
            f.write(df[alg + '_err'].describe().to_string() + "\n")

    pdf_file = md_file.with_suffix("").with_suffix(".pdf")
    xmin = df['elapsed_second'].min()
    xmax = df['elapsed_second'].max()
    ymin = min(df['i_err'].min(), df['ds_err'].min(), df['sp_err'].min())
    ymax = max(df['i_err'].max(), df['ds_err'].max(), df['sp_err'].max())
    ax = df.plot(kind='scatter', x='elapsed_second', y='i_err', color='red', marker='x',
        xlabel='Time [s]', ylabel='Event Time Estimation Error [s]',
        label=alg_names['i'], grid=True, legend=True, figsize=(10, 6),
        xlim=(xmin, xmax), ylim=(ymin, ymax))
    df.plot(kind='scatter', x='elapsed_second', y='ds_err', color='green', marker='o',
        xlabel='Time [s]', ylabel='Event Time Estimation Error [s]',
        label=alg_names['ds'], grid=True, legend=True, figsize=(10, 6),
        ax=ax)
    df.plot(kind='scatter', x='elapsed_second', y='sp_err', color='blue', marker='^',
        xlabel='Time [s]', ylabel='Event Time Estimation Error [s]',
        label=alg_names['sp'], grid=True, legend=True, figsize=(10, 6),
        ax=ax)
    plt.show()
    plt.savefig(pdf_file)