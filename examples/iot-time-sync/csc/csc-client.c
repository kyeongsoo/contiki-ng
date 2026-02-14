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
#include "dev/button-sensor.h" // for GPIO trigger through button-sensor extension
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "sys/log.h"
#include "sys/rtimer.h"
#include "csc-data.h"
#include "csc.h"

#define LOG_MODULE "CSC-Client"
#define LOG_LEVEL LOG_LEVEL_INFO
#define NB_SKIP 10 // number of initial beacons to skip before CFR initialization
#define NB_CFR 100 // number of beacons for CFR initialization

static bool cfr_initialized = false;
// static float cfr = 0.0; // (float)A / (float)D
// static float cfr_tolerance = 1E-3; // CFR initialization condition
static uint32_t seq_num = 0;
static uint32_t num_beacons = 0;
static uint64_t A = 0ULL; // cumulative arrival time
static uint64_t D = 0ULL; // cumulative departure time
static uint64_t iat = 0ULL; // interarrival time
static uint64_t idt = 0ULL; // interdeparture time
static rtimer_ext_clock_t rx_timestamp = 0;
// static rtimer_ext_clock_t rx_timestamp_init = 0;
static rtimer_ext_clock_t rx_timestamp_prev = 0;
static rtimer_ext_clock_t tx_timestamp = 0;
// static rtimer_ext_clock_t tx_timestamp_init = 0;
static rtimer_ext_clock_t tx_timestamp_prev = 0;

#ifdef P2_EXT
// GPIO trigger external variables
extern rtimer_ext_clock_t gio_timestamp;
extern volatile uint8_t gio_triggered;

static bool event_initialized = false;
static uint32_t event_number = 0;
static int num_iter = 0; // ignored in this experiment
static uint64_t elapsed_time = 0ULL; // elapsed time since CFR initialization
static uint64_t iet = 0ULL; // inter-event time
static uint64_t rst_ds; // result of CSC based on double-precision FP division
static uint64_t rst_sp_div; // result of CSC based on single-precision FP division
static int64_t diff;
static rtimer_ext_clock_t gio_timestamp_prev = 0;
#endif

PROCESS(csc_client_process, "A client for CSC experiments");
#ifdef P2_EXT
struct process *p2_ext_process = &csc_client_process; // for the P2 extension
#endif
AUTOSTART_PROCESSES(&csc_client_process);

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  rx_timestamp = RTIMER_EXT_NOW();
  LOG_DBG("Receive a beacon\n");
  if(len == sizeof(csc_data_t)) {
    csc_data_t nn_data;
    memcpy(&nn_data, data, sizeof(nn_data));
    seq_num = nn_data.seq_num;
    tx_timestamp = nn_data.timestamp;
    num_beacons++;
    if (cfr_initialized == false) {
      if (num_beacons <= NB_SKIP) {
        LOG_INFO("t=%"RTIMER_PRI_EXT": Skip a beacon with seq_num=%"PRIu32", tx_ts=%"RTIMER_PRI_EXT"\n",
          rx_timestamp, seq_num, tx_timestamp);
      }
      else {
        // handle timestamp wraparound
        if (rx_timestamp < rx_timestamp_prev) {
          iat = (uint64_t)rx_timestamp + (RTIMER_EXT_CLOCK_MAX - rx_timestamp_prev);
        } else {
          iat = (uint64_t)(rx_timestamp - rx_timestamp_prev);
        }
        if (tx_timestamp < tx_timestamp_prev) {
          idt = (uint64_t)tx_timestamp + (RTIMER_EXT_CLOCK_MAX - tx_timestamp_prev);
        } else {
          idt = (uint64_t)(tx_timestamp - tx_timestamp_prev);
        }
        A += iat;
        D += idt;
        LOG_INFO("Receive a beacon with seq_num=%"PRIu32", tx_ts=%"RTIMER_PRI_EXT", rx_ts=%"RTIMER_PRI_EXT", A=%"PRIu64", D=%"PRIu64", num_beacons=%"PRIu32"\n",
          nn_data.seq_num, tx_timestamp, rx_timestamp, A, D, num_beacons);
        if (num_beacons == (NB_SKIP + NB_CFR)) {
          cfr_initialized = true;
          LOG_INFO("CFR initialized: A=%"PRIu64", D=%"PRIu64"\n", A, D);
          NETSTACK_RADIO.off(); // to minimize interference with GPIO trigger
        }
      }
    }
    rx_timestamp_prev = rx_timestamp;
    tx_timestamp_prev = tx_timestamp;
  }
}

PROCESS_THREAD(csc_client_process, ev, data)
{
  static csc_data_t nn_data = {0, 0};

  PROCESS_BEGIN();

  // enable trigger detection at GPIO
  SENSORS_ACTIVATE(button_sensor);

  // initialize NullNet
  nullnet_buf = (csc_data_t *)&nn_data;
  nullnet_len = sizeof(nn_data);
  nullnet_set_input_callback(input_callback);

  while (1) {
    PROCESS_WAIT_EVENT();

#ifdef P2_EXT
    // process GPIO trigger at P2.x
    if(ev == PROCESS_EVENT_POLL && gio_triggered == 1) {
      if (cfr_initialized == true) {
        if (event_initialized == false) {
          event_initialized = true;
          LOG_INFO("t=%"RTIMER_PRI_EXT": Detect 1st event after CFR initialization\n", gio_timestamp);

          // post-processing indicator and header row for column names in CSV format
          printf("##### BEGIN\n"); 
          printf("event_number,i,ds,sp_div,diff\n");
        }
        else {
          // handle timestamp wraparound
          if (gio_timestamp < gio_timestamp_prev) {
            iet = (uint64_t)gio_timestamp + (RTIMER_EXT_CLOCK_MAX - gio_timestamp_prev);
          } else {
            iet = (uint64_t)(gio_timestamp - gio_timestamp_prev);
          }
          elapsed_time += iet;
          rst_ds = csc_ds(elapsed_time, D, A, &num_iter);
          rst_sp_div = csc_sp_div(elapsed_time, D, A, &num_iter);
          diff = rst_ds - rst_sp_div;
          LOG_DBG("t=%"RTIMER_PRI_EXT": elapsed_time=%"PRIu64", D=%lld, A=%lld\n",
            gio_timestamp, elapsed_time, D, A);

          // data row in CSV format
          printf("%"PRIu32",%"PRIu64",%"PRIu64",%"PRIu64",%"PRId64"\n",
            event_number, elapsed_time, rst_ds, rst_sp_div, diff);
          event_number++; // only after event initialization
        }
        gio_timestamp_prev = gio_timestamp;
        gio_triggered = 0; // clear the flag
      }
    }
#endif
  }

  PROCESS_END();
}
