/**
 * \brief A client for CSC experiments.

 * \author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
 */

#include <assert.h>
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
// #define LOG_LEVEL LOG_LEVEL_DBG

// experimental setup
#ifndef EVENT_NUMBER_MAX
#define EVENT_NUMBER_MAX 1000 // maximum number of events to process after CFR initialization
#endif
#ifndef NB_CFR
#define NB_CFR 100 // number of beacons for CFR initialization
#endif
#ifndef NB_SKIP
#define NB_SKIP 10 // number of initial beacons to skip before CFR initialization
#endif
#ifndef RADIO_OFF_PERIOD
#define RADIO_OFF_PERIOD 60 // period of radio off time after each beacon reception in seconds
#endif

// shared data between the main process and the input callback function
static bool beacon_received = false;
static csc_data_t nn_data = {0, 0}; // for storing the received beacon data
static rtimer_ext_clock_t rx_timestamp = 0;

PROCESS(csc_client_process, "A client for CSC experiments");
#ifdef P2_EXT
struct process *p2_ext_process = &csc_client_process; // for P2 extension
#endif
AUTOSTART_PROCESSES(&csc_client_process);

void input_callback(const void *data, uint16_t len, const linkaddr_t *src,
  const linkaddr_t *dest)
{
  rx_timestamp = RTIMER_EXT_NOW();
  LOG_DBG("Receive a beacon\n");

  if(len == sizeof(csc_data_t)) {
    memcpy(&nn_data, data, sizeof(nn_data));
    beacon_received = true;
    process_poll(&csc_client_process);
  }
}

PROCESS_THREAD(csc_client_process, ev, data)
{
  static struct etimer periodic_timer;

  static bool cfr_initialized = false;
  static uint32_t seq_num = 0;
  static uint32_t num_beacons = 0;
  static csc_int_t A = 0; // cumulative arrival time
  static csc_int_t D = 0; // cumulative departure time
  static csc_int_t iat = 0; // interarrival time
  static csc_int_t idt = 0; // interdeparture time
  static uint16_t num_iter = 0; // ignored in this experiment
  static rtimer_ext_clock_t rx_timestamp_prev = 0;
  static rtimer_ext_clock_t tx_timestamp = 0;
  static rtimer_ext_clock_t tx_timestamp_prev = 0;

#ifdef P2_EXT
  // GPIO trigger external variables
  extern volatile rtimer_ext_clock_t gio_timestamp;
  extern volatile uint8_t gio_triggered;
  static bool event_initialized = false;
  static uint32_t event_number = 0;
  static csc_int_t elapsed_time = 0; // elapsed time since CFR initialization
  static uint64_t iet = 0ULL; // inter-event time
  static csc_int_t rst_ds; // result of CSC based on double-precision FP division
  static csc_int_t rst_sp; // result of CSC based on single-precision FP division
  static csc_int_t diff;
  static rtimer_ext_clock_t gio_timestamp_prev = 0;
#endif

  PROCESS_BEGIN();

  // enable trigger detection at GPIO
  SENSORS_ACTIVATE(button_sensor);

  // initialize NullNet
  nullnet_set_input_callback(input_callback);

  while (true) {
    PROCESS_WAIT_EVENT();

    switch (ev) {
      case PROCESS_EVENT_POLL:
#ifdef P2_EXT
        if (gio_triggered == true) {
          // process GPIO trigger at P2.x
          if (cfr_initialized == true) {
            if (event_initialized == false) {
              event_initialized = true;
              LOG_INFO("t=%"RTIMER_PRI_EXT": Detect 1st event after CFR initialization\n", gio_timestamp);

              // post-processing indicator and header row for column names in CSV format
              printf("##### BEGIN\n"); 
              printf("event_number,i,D,A,ds,sp,diff\n");
            }
            else {
              // handle timestamp wraparound
              if (gio_timestamp < gio_timestamp_prev) {
                iet = (csc_int_t)gio_timestamp + (RTIMER_EXT_CLOCK_MAX - gio_timestamp_prev);
              } else {
                iet = (csc_int_t)(gio_timestamp - gio_timestamp_prev);
              }
              elapsed_time += iet;
              rst_ds = csc_ds(elapsed_time, D, A, &num_iter);
              rst_sp = csc_sp(elapsed_time, D, A, &num_iter);
              diff = rst_ds - rst_sp;
              LOG_DBG("t=%"RTIMER_PRI_EXT": event_number=%"PRIu32", elapsed_time=%"CSC_INT_PRI", D=%"CSC_INT_PRI", A=%"CSC_INT_PRI"\n",
                gio_timestamp, event_number, elapsed_time, D, A);

              // data row in CSV format
              printf("%"PRIu32",%"CSC_INT_PRI",%"CSC_INT_PRI",%"CSC_INT_PRI",%"CSC_INT_PRI",%"CSC_INT_PRI",%"CSC_INT_PRI"\n",
                event_number, elapsed_time, D, A, rst_ds, rst_sp, diff);
              event_number++; // only after event initialization
              
              if (event_number == EVENT_NUMBER_MAX) {
                // indicator for post-processing
                printf("##### END\n");
                PROCESS_EXIT(); // exit the process
              }
              
            } // end of else for "event_initialized == true"
            gio_timestamp_prev = gio_timestamp;
            gio_triggered = 0; // clear the flag
          } // end of if() for "cfr_initialized == true"
        } else if (beacon_received == true) {
#else
        if (beacon_received == true) {
#endif
          // process a received beacon
          seq_num = nn_data.seq_num;
          tx_timestamp = nn_data.timestamp;
          num_beacons++;
          if (cfr_initialized == false) {
            if (num_beacons <= NB_SKIP) {
              LOG_INFO("Skip a beacon with seq_num=%"PRIu32", tx_ts=%"RTIMER_PRI_EXT", rx_ts=%"RTIMER_PRI_EXT", num_beacons=%"PRIu32"\n",
                seq_num, tx_timestamp, rx_timestamp, num_beacons);
            }
            else {
              // handle timestamp wraparound and update 'A' and 'D'
              if (rx_timestamp < rx_timestamp_prev) {
                iat = (csc_int_t)rx_timestamp + (RTIMER_EXT_CLOCK_MAX - rx_timestamp_prev);
              } else {
                iat = (csc_int_t)(rx_timestamp - rx_timestamp_prev);
              }
              if (tx_timestamp < tx_timestamp_prev) {
                idt = (csc_int_t)tx_timestamp + (RTIMER_EXT_CLOCK_MAX - tx_timestamp_prev);
              } else {
                idt = (csc_int_t)(tx_timestamp - tx_timestamp_prev);
              }
              assert((A + iat <= CSC_INT_MAX) && (D + idt <= CSC_INT_MAX)); // to prevent overflow
              A += iat;
              D += idt;
              LOG_INFO("Receive a beacon with seq_num=%"PRIu32", tx_ts=%"RTIMER_PRI_EXT", rx_ts=%"RTIMER_PRI_EXT", num_beacons=%"PRIu32", A=%"CSC_INT_PRI", D=%"CSC_INT_PRI"\n",
                nn_data.seq_num, tx_timestamp, rx_timestamp, num_beacons, A, D);
              if (num_beacons == (NB_SKIP + NB_CFR)) {
                cfr_initialized = true;
                LOG_INFO("CFR initialized: A=%"CSC_INT_PRI", D=%"CSC_INT_PRI"\n", A, D);
                NETSTACK_RADIO.off(); // to minimize interference with GPIO trigger
                etimer_set(&periodic_timer, RADIO_OFF_PERIOD*CLOCK_SECOND);
              }
            }
          } else {
            // handle timestamp wraparound and update 'A' and 'D'
            if (rx_timestamp < rx_timestamp_prev) {
              iat = (csc_int_t)rx_timestamp + (RTIMER_EXT_CLOCK_MAX - rx_timestamp_prev);
            } else {
              iat = (csc_int_t)(rx_timestamp - rx_timestamp_prev);
            }
            if (tx_timestamp < tx_timestamp_prev) {
              idt = (csc_int_t)tx_timestamp + (RTIMER_EXT_CLOCK_MAX - tx_timestamp_prev);
            } else {
              idt = (csc_int_t)(tx_timestamp - tx_timestamp_prev);
            }
            assert((A + iat <= CSC_INT_MAX) && (D + idt <= CSC_INT_MAX)); // to prevent overflow
            A += iat;
            D += idt;
            LOG_DBG("Receive a beacon with seq_num=%"PRIu32", tx_ts=%"RTIMER_PRI_EXT", rx_ts=%"RTIMER_PRI_EXT", num_beacons=%"PRIu32", A=%"CSC_INT_PRI", D=%"CSC_INT_PRI"\n",
                    nn_data.seq_num, tx_timestamp, rx_timestamp, num_beacons, A, D);
            NETSTACK_RADIO.off(); // to minimize interference with GPIO trigger
            etimer_set(&periodic_timer, RADIO_OFF_PERIOD*CLOCK_SECOND);
          }

          rx_timestamp_prev = rx_timestamp;
          tx_timestamp_prev = tx_timestamp;
          beacon_received = false; // clear the flag
        }
        break;
      case PROCESS_EVENT_TIMER:
        if (data == &periodic_timer) {
          // turn on the radio to update CFR based on a new beacon
          NETSTACK_RADIO.on();
        }
      default:
        break;
    } // end of switch () for event handling
  } // end of while()

  PROCESS_END();
}
