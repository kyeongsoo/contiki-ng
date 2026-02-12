/**
 * \file
 *         A client for CSC experiments.
 * \author
 *         Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h> // for memcpy()
#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "sys/log.h"
#include "sys/rtimer.h"
#include "csc-data.h"
#include "csc.h"

// control CSC optimization
#define _OPT1 // turn off iteration couting in DS */
#define _OPT2 // enable branchless programming in DS
#define _OPT3 // turn off checking the value of A in division algos

#define LOG_MODULE "CSC-Client"
#define LOG_LEVEL LOG_LEVEL_NONE

#define NB_CFR 100 // number of beacons for CFR initialization

static bool cfr_initialized = false;
// static float cfr = 0.0; // (float)A / (float)D
// static float cfr_tolerance = 1E-3; // CFR initialization condition
static int num_beacons = 0;
static int num_events = 0;
static int num_iter = 0; // ignored in this experiment
static uint64_t A = 0ULL; // cumulative arrival time
static uint64_t D = 0ULL; // cumulative departure time
static uint64_t iat = 0ULL; // interarrival time
static uint64_t idt = 0ULL; // interdeparture time
static uint64_t elapsed_time = 0ULL; // elapsed time since CFR initialization
static uint64_t rst_ds;
static uint64_t rst_sp_div;
static int64_t diff;
#ifdef RTIMER_EXT
static rtimer32_clock_t rx_timestamp = 0;
// static uint64_t rx_timestamp_init = 0ULL;
static rtimer32_clock_t rx_timestamp_prev = 0;
static rtimer32_clock_t tx_timestamp = 0;
// static uint64_t tx_timestamp_init = 0ULL;
static rtimer32_clock_t tx_timestamp_prev = 0;
#else
static rtimer_clock_t rx_timestamp = 0;
// static uint64_t rx_timestamp_init = 0ULL;
static rtimer_clock_t rx_timestamp_prev = 0;
static rtimer_clock_t tx_timestamp = 0;
// static uint64_t tx_timestamp_init = 0ULL;
static rtimer_clock_t tx_timestamp_prev = 0;
#endif

PROCESS(csc_client_process, "A client for CSC experiments");
struct process *p2_ext_process = &csc_client_process; // for the P2 extension
AUTOSTART_PROCESSES(&csc_client_process);

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  rx_timestamp = RTIMER_NOW();
  if(len == sizeof(csc_data_t)) {
    csc_data_t nn_data;
    memcpy(&nn_data, data, sizeof(nn_data));
    tx_timestamp = nn_data.timestamp;
   
    // handle timestamp wraparound
    if (rx_timestamp < rx_timestamp_prev) {
#ifdef RTIMER_EXT
      iat = (uint64_t)rx_timestamp + (RTIMER32_CLOCK_MAX - rx_timestamp_prev);
#else
      iat = (uint64_t)rx_timestamp + (RTIMER_CLOCK_MAX - rx_timestamp_prev);
#endif
    } else {
      iat = (uint64_t)(rx_timestamp - rx_timestamp_prev);
    }
    if (tx_timestamp < tx_timestamp_prev) {
#ifdef RTIMER_EXT
      idt = (uint64_t)tx_timestamp + (RTIMER32_CLOCK_MAX - tx_timestamp_prev);
#else
      idt = (uint64_t)tx_timestamp + (RTIMER_CLOCK_MAX - tx_timestamp_prev);
#endif
    } else {
      idt = (uint64_t)(tx_timestamp - tx_timestamp_prev);
    }

    if (cfr_initialized == false) {
      A += iat;
      D += idt;
      num_beacons++;
#ifdef RTIMER_EXT
      LOG_INFO("Receive a beacon with seq_num=%lu, tx_ts=%lu, rx_ts=%lu, A=%llu, D=%llu, num_beacons=%d\n",
        (unsigned long)nn_data.seq_num, (unsigned long)tx_timestamp, (unsigned long)rx_timestamp, A, D, num_beacons);
#else
      LOG_INFO("Receive a beacon with seq_num=%lu, tx_ts=%u, rx_ts=%u, A=%llu, D=%llu, num_beacons=%d\n",
        (unsigned long)nn_data.seq_num, (unsigned)tx_timestamp, (unsigned)rx_timestamp, A, D, num_beacons);
#endif
      if (num_beacons == NB_CFR) {
        // cfr = (float)A / (float)D;
        cfr_initialized = true;
        LOG_INFO("CFR initialized: A=%llu, D=%llu\n", A, D);

        // post-processing indicator and header row for column names in CSV format
        printf("##### BEGIN\n"); 
        printf("num_events,i,ds,sp_div,diff\n");
      }
    }
    else {
      elapsed_time += iat;
      num_events++;
      rst_ds = csc_ds(elapsed_time, D, A, &num_iter);
      rst_sp_div = csc_sp_div(elapsed_time, D, A, &num_iter);
      diff = rst_ds - rst_sp_div;
      LOG_INFO("CSC: num_events=%d, i=%lld, csc_ds=%lld, csc_sp_div=%lld, diff=%lld\n",
        num_events, elapsed_time, rst_ds, rst_sp_div, diff);

      // data row in CSV format
      printf("%d,%lld,%lld,%lld,%lld\n",
        num_events, elapsed_time, rst_ds, rst_sp_div, diff);
    }

    rx_timestamp_prev = rx_timestamp;
    tx_timestamp_prev = tx_timestamp;
  }
}

PROCESS_THREAD(csc_client_process, ev, data)
{
  static csc_data_t nn_data = {0, 0};

  PROCESS_BEGIN();

  nullnet_buf = (csc_data_t *)&nn_data;
  nullnet_len = sizeof(nn_data);
  nullnet_set_input_callback(input_callback);

  PROCESS_END();
}
