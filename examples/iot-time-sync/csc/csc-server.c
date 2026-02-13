/**
 * \file
 *         A server for CSC experiments.
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

#define LOG_MODULE "CSC-Server"
#define LOG_LEVEL LOG_LEVEL_INFO

#define BEACON_INTERVAL (1 * CLOCK_SECOND) // 1 <= RTIMER_CLOCK_MAX/RTIMER_SECOND (=2 for TelosB)

// GPIO trigger variables
#ifdef RTIMER_EXT
extern rtimer32_clock_t gio_timestamp;
#else
extern rtimer_clock_t gio_timestamp;
#endif
extern volatile uint8_t gio_triggered;

static bool event_initialized = false;
static unsigned int event_number = 0;
#ifdef RTIMER_EXT
static rtimer32_clock_t elapsed_time = 0;
static rtimer32_clock_t event_init_time = 0;
#else
static rtimer_clock_t elapsed_time = 0;
static rtimer_clock_t event_init_time = 0;
#endif

PROCESS(csc_server_process, "A server for CSC experiments");
struct process *p2_ext_process = &csc_server_process; // for the P2 extension
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
  
  etimer_set(&periodic_timer, BEACON_INTERVAL);
  while(1) {
    // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    PROCESS_WAIT_EVENT();

    // send a beacon with the current timestamp
    if (ev == PROCESS_EVENT_TIMER && data == &periodic_timer) {
      memcpy(nullnet_buf, &nn_data, sizeof(nn_data));
      nullnet_len = sizeof(nn_data);
#ifdef RTIMER_EXT
      nn_data.timestamp = RTIMER32_NOW();
#else
      nn_data.timestamp = RTIMER_NOW();
#endif
      NETSTACK_NETWORK.output(NULL);
#ifdef RTIMER_EXT
      LOG_INFO("Send a beacon with seq_num=%lu, timestamp=%lu\n",
        (unsigned long)nn_data.seq_num, (unsigned long)nn_data.timestamp);
#else
      LOG_INFO("Send a beacon with seq_num=%lu, timestamp=%u\n",
        (unsigned long)nn_data.seq_num, (unsigned)nn_data.timestamp);
#endif
      /* LOG_INFO_LLADDR(NULL); */
      /* LOG_INFO_("\n"); */
      nn_data.seq_num++;
      etimer_reset(&periodic_timer);
    }

    // process GPIO trigger at P2.x
    if(ev == PROCESS_EVENT_POLL && gio_triggered == 1) {
      if (event_initialized == false) {
#ifdef RTIMER_EXT
        LOG_INFO("Detect 1st event with timestamp=%lu\n", (unsigned long)gio_timestamp);
#else
        LOG_INFO("Detect 1st event with timestamp=%u\n", (unsigned)gio_timestamp);
#endif
          event_init_time = gio_timestamp;
          event_initialized = true;
      }
      else {
          elapsed_time = gio_timestamp - event_init_time;
#ifdef RTIMER_EXT
          LOG_INFO("Event with timestamp=%lu, number=%u, elapsed_time=%lu",
              (unsigned long)gio_timestamp, event_number, (unsigned long)elapsed_time);
#else
          LOG_INFO("Event with timestamp=%u, number=%u, elapsed_time=%u",
              (unsigned)gio_timestamp, event_number, (unsigned)elapsed_time);
#endif
          event_number++; // only after event initialization
      }
      gio_triggered = 0; // reset the trigger for the next event
    }
  }

  PROCESS_END();
}
