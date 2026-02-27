/**
 * \brief A server for CSC experiments.
 * 
 * \author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
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

#define LOG_MODULE "CSC-Server"
#define LOG_LEVEL LOG_LEVEL_INFO

// experimental setup
#ifndef EVENT_NUMBER_MAX
#define EVENT_NUMBER_MAX 1000 // maximum number of events to process after CFR initialization
#endif
#ifndef BEACON_INTERVAL // beacon interval in seconds
#ifdef RTIMER_EXT
#define BEACON_INTERVAL 10 // <= RTIMER_CLOCK_MAX/RTIMER_SECOND (~8,810 s (~2.45 hours) for TelosB)
#else
#define BEACON_INTERVAL 1 // <= RTIMER_CLOCK_MAX/RTIMER_SECOND (~2 s for TelosB)
#endif
#endif

// GPIO trigger external variables
extern volatile rtimer_clock_t gio_timestamp;
extern volatile uint8_t gio_triggered;

static bool event_initialized = false;
static uint32_t event_number = 0;
static uint64_t elapsed_ticks = 0ULL;
static uint64_t iet = 0ULL; // inter-event ticks
static rtimer_clock_t gio_timestamp_prev = 0;

PROCESS(csc_server_process, "A server for CSC experiments");
struct process *p2_ext_process = &csc_server_process; // for P2 extension
AUTOSTART_PROCESSES(&csc_server_process);

PROCESS_THREAD(csc_server_process, ev, data)
{
  static struct etimer periodic_timer;
  static csc_data_t nn_data = {0, 0};

  PROCESS_BEGIN();

  // enable trigger detection at GPIO
  SENSORS_ACTIVATE(button_sensor);

  // initialize NullNet
  nullnet_buf = (csc_data_t *)&nn_data;
  nullnet_len = sizeof(nn_data);
  
  etimer_set(&periodic_timer, BEACON_INTERVAL*CLOCK_SECOND);
  while (1) {
    PROCESS_WAIT_EVENT();

    // send a beacon with the current timestamp
    if (ev == PROCESS_EVENT_TIMER && data == &periodic_timer) {
      memcpy(nullnet_buf, &nn_data, sizeof(nn_data));
      nullnet_len = sizeof(nn_data);
      nn_data.timestamp = RTIMER_NOW();
      NETSTACK_NETWORK.output(NULL);
      if (event_initialized == false) {
        LOG_INFO("Send a beacon with seq_num=%"PRIu32", timestamp=%"RTIMER_PRI"\n",
          nn_data.seq_num, nn_data.timestamp);
      }
      nn_data.seq_num++;
      etimer_reset(&periodic_timer);
    }

    // process GPIO trigger at P2.x
    if (ev == PROCESS_EVENT_POLL && gio_triggered == 1) {
      if (event_initialized == false) {
        event_initialized = true;
        LOG_INFO("Detect 1st event with timestamp=%"RTIMER_PRI"\n", gio_timestamp);

        // post-processing indicator and header row for column names in CSV format
        printf("##### BEGIN\n"); 
        printf("event_number,elapsed_ticks\n");
      }
      else {
        // handle timestamp wraparound
        if (gio_timestamp < gio_timestamp_prev) {
          iet = (uint64_t)gio_timestamp + (RTIMER_CLOCK_MAX - gio_timestamp_prev);
        } else {
          iet = (uint64_t)(gio_timestamp - gio_timestamp_prev);
        }
        elapsed_ticks += iet;
        LOG_DBG("Event with timestamp=%"RTIMER_PRI", event_number=%"PRIu32", elapsed_ticks=%"PRIu64"\n",
          gio_timestamp, event_number, elapsed_ticks);
        printf("%"PRIu32",%"PRIu64"\n", event_number, elapsed_ticks);
        event_number++; // only after event initialization

        if (event_number == EVENT_NUMBER_MAX) {
          // indicator for post-processing
          printf("##### END\n");
          break; // end the process
        } 
      }
      gio_timestamp_prev = gio_timestamp;
      gio_triggered = 0; // clear the flag
    } // end of if () for GPIO trigger
  } // end of while () for event loop

  PROCESS_END();
}
