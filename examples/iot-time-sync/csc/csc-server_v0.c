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

#define BEACON_INTERVAL (1 * CLOCK_SECOND) // 1 <= RTIMER_CLOCK_MAX/RTIMER_SECOND (=2 for TelosB)

// GPIO trigger external variables
extern rtimer_ext_clock_t gpio_timestamp;
extern volatile uint8_t gpio_triggered;

static bool event_initialized = false;
static uint32_t event_number = 0;
static uint64_t elapsed_time = 0ULL;
static uint64_t iet = 0ULL; // inter-event time
static rtimer_ext_clock_t gpio_timestamp_prev = 0;

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
  
  etimer_set(&periodic_timer, BEACON_INTERVAL);
  while (1) {
    PROCESS_WAIT_EVENT();

    // send a beacon with the current timestamp
    if (ev == PROCESS_EVENT_TIMER && data == &periodic_timer) {
      memcpy(nullnet_buf, &nn_data, sizeof(nn_data));
      nullnet_len = sizeof(nn_data);
      nn_data.timestamp = RTIMER_EXT_NOW();
      NETSTACK_NETWORK.output(NULL);
      if (event_initialized == false) {
        LOG_INFO("Send a beacon with seq_num=%"PRIu32", timestamp=%"RTIMER_PRI_EXT"\n",
          nn_data.seq_num, nn_data.timestamp);
      }
      nn_data.seq_num++;
      etimer_reset(&periodic_timer);
    }

    // process GPIO trigger at P2.x
    if(ev == PROCESS_EVENT_POLL && gpio_triggered == 1) {
      if (event_initialized == false) {
        event_initialized = true;
        LOG_INFO("Detect 1st event with timestamp=%"RTIMER_PRI_EXT"\n", gpio_timestamp);

        // post-processing indicator and header row for column names in CSV format
        printf("##### BEGIN\n"); 
        printf("event_number,elapsed_time\n");
      }
      else {
        // handle timestamp wraparound
        if (gpio_timestamp < gpio_timestamp_prev) {
          iet = (uint64_t)gpio_timestamp + (RTIMER_EXT_CLOCK_MAX - gpio_timestamp_prev);
        } else {
          iet = (uint64_t)(gpio_timestamp - gpio_timestamp_prev);
        }
        elapsed_time += iet;
        LOG_DBG("Event with timestamp=%"RTIMER_PRI_EXT", event_number=%"PRIu32", elapsed_time=%"PRIu64"\n",
          gpio_timestamp, event_number, elapsed_time);
        printf("%"PRIu32",%"PRIu64"\n", event_number, elapsed_time);
        event_number++; // only after event initialization
      }
      gpio_timestamp_prev = gpio_timestamp;
      gpio_triggered = 0; // clear the flag
    }
  }

  PROCESS_END();
}
