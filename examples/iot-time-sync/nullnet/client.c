/*
 * Copyright (c) 2017, RISE SICS.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         NullNet broadcast example
 * \author
*         Simon Duquennoy <simon.duquennoy@ri.se>
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "sys/log.h"
#include "sys/rtimer.h"
#include "nullnetdata.h"
#include "csc.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
#define NB_CFR 100 // number of beacons required for CFR initialization

/* for time sync */
static bool cfr_initialized = false;
// static float cfr = 0.0; // (float)A / (float)D
// static float cfr_tolerance = 1E-3; // CFR initialization condition
static int num_beacons = 0;
static int num_events = 0;
static int num_iter = 0;
static uint64_t A = 0ULL; // cumulative arrival time
static uint64_t D = 0ULL; // cumulative departure time
static uint64_t iat = 0ULL; // interarrival time
static uint64_t idt = 0ULL; // interdeparture time
static uint64_t elapsed_time = 0ULL; // elapsed time since CFR initialization
static uint64_t rst_ds;
static uint64_t rst_sp_div;
static rtimer_clock_t rx_timestamp = 0;
// static uint64_t rx_timestamp_init = 0ULL;
static rtimer_clock_t rx_timestamp_prev = 0;
static rtimer_clock_t tx_timestamp = 0;
// static uint64_t tx_timestamp_init = 0ULL;
static rtimer_clock_t tx_timestamp_prev = 0;

PROCESS(nullnet_example_process, "NullNet broadcast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  rx_timestamp = RTIMER_NOW();
  if(len == sizeof(nullnet_data_t)) {
    nullnet_data_t nn_data;
    memcpy(&nn_data, data, sizeof(nn_data));
    tx_timestamp = nn_data.timestamp;
   
    // handle timestamp wraparound
    if (rx_timestamp < rx_timestamp_prev) {
      iat = (uint64_t)rx_timestamp + (RTIMER_CLOCK_MAX - rx_timestamp_prev);
    } else {
      iat = (uint64_t)(rx_timestamp - rx_timestamp_prev);
    }
    if (tx_timestamp < tx_timestamp_prev) {
      idt = (uint64_t)tx_timestamp + (RTIMER_CLOCK_MAX - tx_timestamp_prev);
    } else {
      idt = (uint64_t)(tx_timestamp - tx_timestamp_prev);
    }

    if (cfr_initialized == false) {
      A += iat;
      D += idt;
      num_beacons++;
      LOG_INFO("Receive a beacon with seq_num=%u, tx_ts=%u, rx_ts=%u, A=%llu, D=%llu, num_beacons=%d\n",
        (unsigned)nn_data.seq_num, (unsigned)tx_timestamp, (unsigned)rx_timestamp, A, D, num_beacons);
      if (num_beacons == NB_CFR) {
        // cfr = (float)A / (float)D;
        cfr_initialized = true;
        LOG_INFO("CFR initialized: A=%llu, D=%llu\n", A, D);
      }
    }
    else {
      elapsed_time += iat;
      num_events++;
      rst_ds = csc_ds(elapsed_time, D, A, &num_iter);
      rst_sp_div = csc_sp_div(elapsed_time, D, A, &num_iter);
      // LOG_INFO("CSC: num_events=%d, i=%lld, csc_ds=%lld with num_iter=%d, csc_sp_div=%lld, csc_dp_div=%lld\n",
      //   num_events, elapsed_time, csc_ds(elapsed_time, D, A, &num_iter), num_iter, csc_sp_div(elapsed_time, D, A, &num_iter), csc_dp_div(elapsed_time, D, A, &num_iter));
      LOG_INFO("CSC: num_events=%d, i=%lld, csc_ds=%lld with num_iter=%d, csc_sp_div=%lld, diff=%lld\n",
        num_events, elapsed_time, rst_ds, num_iter, rst_sp_div, rst_ds  - rst_sp_div);
    }

    rx_timestamp_prev = rx_timestamp;
    tx_timestamp_prev = tx_timestamp;
  }
}

PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static nullnet_data_t nn_data = {0, 0};

  PROCESS_BEGIN();

  nullnet_buf = (nullnet_data_t *)&nn_data;
  nullnet_len = sizeof(nn_data);
  nullnet_set_input_callback(input_callback);

  PROCESS_END();
}